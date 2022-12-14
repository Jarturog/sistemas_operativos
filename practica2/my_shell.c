// Autores: Juan Arturo Abaurrea Calafell, Pere Antoni Prats Villalonga y Marta González Juan
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64
#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 0
#define DEBUGN6 0
#define PROMPT '$'
#define SUCCESS 0
#define FAILURE -1
#include "colores.h"
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

struct info_job
{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

static char mi_shell[COMMAND_LINE_SIZE]; // el nombre del ejecutable con el que se ha ejecutado el mini shell
static struct info_job jobs_list[N_JOBS];
static int n_pids; // número de jobs en la lista de jobs

// declaraciones de funciones
char *read_line(char *line);
void imprimir_prompt();
int parse_args(char **args, char *line);
int execute_line(char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
void jobs_list_reset(int idx);
void jobs_list_update(int idx, pid_t pid, char status, char cmd[]);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);
int is_background(char **args);
int jobs_list_add(pid_t pid, char status, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
int is_output_redirection(char **args);

int main(int argc, char *argv[])
{
    signal(SIGCHLD, reaper); // llamada al enterrador de zombies cuando un hijo acaba (señal SIGCHLD)
    signal(SIGINT, ctrlc);   // SIGINT es la señal de interrupción que produce Ctrl+C
    signal(SIGTSTP, ctrlz);  // SIGTSTP es la señal de interrupción que produce Ctrl+Z
    // Se inicializan la línea de comandos, el jobs_list y la variable mi_shell
    char line[COMMAND_LINE_SIZE];
    jobs_list_reset(0);
    strcpy(mi_shell, argv[0]);
    n_pids = 0; // se inicializa el número de jobs a 0
    // Se inicia el bucle para leer los comandos
    while (1)
    {
        if (read_line(line) != NULL)
        {
            execute_line(line);
        }
    }
    return 0;
}

char *read_line(char *line)
{
    imprimir_prompt();
    if (fgets(line, COMMAND_LINE_SIZE, stdin))
    {
        line[COMMAND_LINE_SIZE - 1] = '\0'; // substituyo el carácter final por \0
        return line;
    } // si es NULL
    if (feof(stdin))
    { // CTRL+D
        printf("\nadiós\n");
        exit(0);
    }
    return NULL;
}

void imprimir_prompt()
{
    char cwd[COMMAND_LINE_SIZE];
    getcwd(cwd, COMMAND_LINE_SIZE);
    printf(MAGENTA_T "%s" RESET ":" CYAN_T "%s " VERDE_T "%c " RESET, getenv("USER"), cwd, PROMPT);
    fflush(stdout);
}

int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    int background = 0;                     // se inicializa a 0 indicando que no hay jobs
    char line_inalterada[strlen(line) + 1]; // paso extra para imprimir el cmd ya que parse_args altera line
    strcpy(line_inalterada, line);
    line_inalterada[strlen(line) - 1] = '\0'; // me deshago del salto de línea
    // fragmenta line en args
    if (!parse_args(args, line)) // si no hay tokens
    {
        return FAILURE;
    }
    // comprueba si es un comando interno
    if (check_internal(args))
    {
        return SUCCESS;
    }
    background = is_background(args); // Analizamos si en la línea de comandos hay un & y recogemos el resultado en background
    // Si no es un comando interno se crea un hilo para ejecutar el comando
    pid_t pid = fork();
    if (pid == 0) // Proceso hijo
    {
        signal(SIGCHLD, SIG_DFL);    // Asocia la acción por defecto a SIGCHLD
        signal(SIGINT, SIG_IGN);     // ignorará la señal SIGINT
        signal(SIGTSTP, SIG_IGN);    // ignorará la señal SIGTSTP
        is_output_redirection(args); // Se comprueba si hay redireccionamiento
        execvp(args[0], args);       // si sigue la ejecución es por un error
        fprintf(stderr, ROJO_T "%s: no se encontró la orden\n" RESET, line_inalterada);
        exit(FAILURE);
    }
    else if (pid > 0) // Proceso padre
    {
        if (DEBUGN3 || DEBUGN4 || DEBUGN5) // hago el OR porque en la página 7 de la documentación del nivel 4 y en la página 8 del nivel 5 también aparecen
        {                                  // se ha decidido imprimir los dos juntos ya que al tratarse de procesos diferentes no se aseguraba que estuvieran en orden las impresiones
            fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n[execute_line()→ PID hijo: %d (%s)]\n" RESET, getpid(), mi_shell, pid, line_inalterada);
        }

        if (!background) // si se ejecuta en primer plano
        {
            jobs_list_update(0, pid, 'E', line_inalterada);
            while (jobs_list[0].pid > 0)
            { // cuando finaliza el proceso en primer plano sale del while
                pause();
            }
        }
        else
        {
            jobs_list_add(pid, 'E', line_inalterada); // si se ejecuta en background lo incorpora a la lista de trabajos jobs_list[ ]
        }
    }
    else // Error
    {
        perror("Error tratando comando externo con fork()");
    }
    return SUCCESS;
}

int parse_args(char **args, char *line)
{
    const char delim[4] = "\t\n\r "; // delimitadores
    int i = 0;                       // índice
    // primer token
    args[0] = strtok(line, delim); // no puede ser NULL porque eso lo comprueba read_line
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[parse_args()→token %d: %s]\n" RESET, i, args[i]);
    }
    if (args[i] != NULL && args[i][0] == '#')
    {
        args[i] = NULL;
        if (DEBUGN1)
        {
            fprintf(stderr, GRIS_T "[parse_args()->token %d corregido: %s]\n" RESET, i, args[i]);
        }
    }
    // resto de tokens
    while (args[i] != NULL && i < ARGS_SIZE - 1)
    {
        i++;
        args[i] = strtok(NULL, delim);
        if (DEBUGN1)
        {
            fprintf(stderr, GRIS_T "[parse_args()→token %d: %s]\n" RESET, i, args[i]);
        }
        if (args[i] != NULL && args[i][0] == '#')
        {
            args[i] = NULL;
            if (DEBUGN1)
            {
                fprintf(stderr, GRIS_T "[parse_args()→token %d corregido: %s]\n" RESET, i, args[i]);
            }
        }
    }
    args[ARGS_SIZE - 1] = NULL; // el último token siempre ha de ser NULL
    return i;
}

int check_internal(char **args)
{
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
        return 1;
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        internal_cd(args);
        return 1;
    }
    else if (strcmp(args[0], "export") == 0)
    {
        internal_export(args);
        return 1;
    }
    else if (strcmp(args[0], "source") == 0)
    {
        internal_source(args);
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
        return 1;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        internal_fg(args);
        return 1;
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        internal_bg(args);
        return 1;
    }

    return 0;
}

int internal_cd(char **args)
{
    char cwd[COMMAND_LINE_SIZE]; // actual directory
    char *home;
    if (!(home = getenv("HOME"))) // si error
    {
        perror("getenv() error");
        return FAILURE;
    }
    // consigo el actual directory en cwd
    if (!getcwd(cwd, COMMAND_LINE_SIZE))
    {
        perror("getcwd() error");
        return FAILURE;
    }
    // si se quiere ir al HOME
    if (args[1] == NULL)
    {
        if (chdir(home)) // si error
        {
            perror("chdir() error");
            return FAILURE;
        }
        if (DEBUGN2)
        {
            fprintf(stderr, GRIS_T "[internal_cd()→ PWD: %s]\n" RESET, cwd);
        }
        return SUCCESS;
    }
    // comprobación de puntos para ir a una carpeta superior en los argumentos
    while (args[1][0] == args[1][1] && args[1][0] == 46) // si hay ..
    {
        do // vuelve atrás una carpeta
        {
            cwd[strlen(cwd) - 1] = '\0';
        } while (cwd[strlen(cwd) - 1] != 47); // 47 es /
        cwd[strlen(cwd) - 1] = '\0';          // elimino la barra sobrante
        args[1]++;                            // quito los dos puntos del string
        args[1]++;
        if (args[1][0] == 47) // si es una barra es que hay dos puntos más
        {
            args[1]++; // quito la barra
        }
    }
    // comprobación de espacios en los argumentos y creación del string que se pondrá en el chdir
    char argsToCwd[COMMAND_LINE_SIZE * ARGS_SIZE];       // como máximo el conjunto de los argumentos tendrá este tamaño
    memset(argsToCwd, 0, COMMAND_LINE_SIZE * ARGS_SIZE); // tengo que rellenar la memoria que ocupa de '\0' porque sino
                                                         // en cada llamada se mantiene el valor anterior de argsToCwd
    int i = 1;                                           // índice del argumento que se está comprobando
    while (args[i] != NULL && args[i][0] != '\0')
    {
        strcat(argsToCwd, "/");
        while (args[i] != NULL && args[i][0] != '\0' && args[i][strlen(args[i]) - 1] == 92) // si hay la barra inclinada del revés
        {
            args[i][strlen(args[i]) - 1] = '\0';
            strcat(argsToCwd, args[i]);
            strcat(argsToCwd, " ");
            i++;
        }
        if (args[i][0] == 34 || args[i][0] == 39) // si hay comillas simples o dobles
        {
            int tipoComa = args[i][0];
            args[i]++; // paso por encima de la comilla
            while (args[i] != NULL && args[i][0] != '\0' && !(args[i][strlen(args[i]) - 1] == 34 || args[i][strlen(args[i]) - 1] == 39))
            { // mientras no sea NULL, ni final y no haya llegado a otra " o '
                strcat(argsToCwd, args[i]);
                strcat(argsToCwd, " ");
                i++;
            }
            if (args[i][strlen(args[i]) - 1] != tipoComa) // si no es " " o ' '
            {
                fprintf(stderr, ROJO_T "Error en internal_cd() por comillas diferentes" RESET);
                return FAILURE;
            }
            args[i][strlen(args[i]) - 1] = '\0'; // al substituir la comilla por el \0 reduzco el tamaño del string
        }
        strcat(argsToCwd, args[i]);
        i++; // en el caso de que no hubiera espacios va al siguiente argumento, en caso contrario habrá ido de dos en dos
    }
    // concateno argsToCwd (lo que ha escrito el usuario que no sean ..) y cambio la dirección actual a esa
    strcat(cwd, argsToCwd);
    if (chdir(cwd)) // si error
    {
        perror("chdir() error");
        return FAILURE;
    }
    if (DEBUGN2)
    {
        fprintf(stderr, GRIS_T "[internal_cd()→ PWD: %s]\n" RESET, cwd);
    }
    return SUCCESS;
}

int internal_export(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return FAILURE;
    }
    char *valor;
    if (!(valor = strchr(args[1], 61))) // 61 es =
    {
        if (DEBUGN2)
        {
            fprintf(stderr, GRIS_T "[internal_export()→ nombre: %s]\n[internal_export()→ valor: %s]\n" RESET, args[1], getenv(args[1]));
        }
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return FAILURE;
    }
    valor++; // ignoro el = que separa nombre y valor
    char *nombre = strtok(args[1], "=");
    if (DEBUGN2)
    {
        fprintf(stderr, GRIS_T "[internal_export()→ nombre: %s]\n[internal_export()→ valor: %s]\n" RESET, nombre, getenv(nombre));
    }
    if ((nombre == NULL) || (valor == NULL) || (nombre[0] == '\0') || (valor[0] == '\0'))
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return FAILURE;
    }
    char *vInicial;
    if (!(vInicial = getenv(nombre)))
    {
        perror("getenv() error");
        return FAILURE;
    }
    if (setenv(nombre, valor, 1))
    {
        perror("setenv() error");
        return FAILURE;
    }
    if (DEBUGN2)
    {
        fprintf(stderr, GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, vInicial, nombre, getenv(nombre));
    }
    return SUCCESS;
}

int internal_source(char **args)
{
    char line[COMMAND_LINE_SIZE];
    FILE *file;

    if (!args[1])
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return FAILURE;
    }
    if (!(file = fopen(args[1], "r")))
    {
        perror(ROJO_T "fopen");
        return FAILURE;
    }
    fflush(file);
    while (fgets(line, COMMAND_LINE_SIZE, file))
    {
        line[strlen(line) - 1] = '\0';
        fflush(file);
        if (DEBUGN3)
        {
            fprintf(stderr, GRIS_T "[Internal_source()→ LINE: %s]\n" RESET, line);
        }
        execute_line(line);
    }
    fclose(file);
    return SUCCESS;
}

int internal_jobs(char **args)
{
    for (int i = 1; i <= n_pids; i++)
    {
        fprintf(stdout, "[%d]\t%d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
    }
    return SUCCESS;
}

int internal_fg(char **args)
{
    // Se chequea la sintaxis
    int pos = atoi(args[1]);

    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: fg <Indice del proceso en JobsList>\n" RESET);
        return FAILURE;
    }
    if ((pos > n_pids) || (pos == 0))
    {
        fprintf(stderr, ROJO_T "fg %d: no existe ese trabajo\n" RESET, pos);
        return FAILURE;
    }

    // Si se llega aqui es que existe el trabajo
    // Si el status es 'D' se reactiva el proceso y se pasa a la posición 0
    if (jobs_list[pos].status == 'D')
    {
        if (kill(jobs_list[pos].pid, SIGCONT) != 0) // si ha habido error entra en el if
        {
            perror("kill");
            exit(FAILURE);
        }
    }

    if (DEBUGN6)
    {
        fprintf(stderr, GRIS_T "[internal_fg()→ Señal 18 (SIGCONT) enviada a %d (%s)]\n" RESET, jobs_list[pos].pid, jobs_list[pos].cmd);
    }
    // Si en cmd se encuentra el simbolo '&' se elimina
    if (jobs_list[pos].cmd[strlen(jobs_list[pos].cmd) - 1] == '&')
    {
        jobs_list[pos].cmd[strlen(jobs_list[pos].cmd) - 1] = '\0';
    }
    // Copiar los datos a jobs_list[0]
    jobs_list_update(0, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
    // Eliminar jobs_list[pos]
    jobs_list_remove(pos);
    // Mostrarlo por pantalla
    fprintf(stdout, "%s\n", jobs_list[0].cmd);
    // Pausa
    while (jobs_list[0].pid > 0)
    {
        pause();
    }
    return SUCCESS;
}

int internal_bg(char **args)
{
    // Se chequea la sintaxis
    int pos = atoi(args[1]);
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: bg <Indice del proceso en JobsList>\n" RESET);
        return FAILURE;
    }
    if ((pos > n_pids) || (pos == 0))
    {
        fprintf(stderr, ROJO_T "bg %d: no existe ese trabajo\n" RESET, pos);
        return FAILURE;
    }
    // Se chequea que el proceso este en pausa
    if (jobs_list[pos].status == 'E')
    {
        fprintf(stderr, ROJO_T "bg %d: el trabajo ya está en segundo plano\n" RESET, pos);
        return FAILURE;
    }
    // Si se llega hasta aqui es que el trabajo esta en pausa
    if (DEBUGN6)
    {
        fprintf(stderr, GRIS_T "[internal_bg()→ Señal 18 (SIGCONT) enviada a %d (%s)]\n" RESET, jobs_list[pos].pid, jobs_list[pos].cmd);
    }
    // Se cambia el status a 'E' y se añade '&' al cmd
    jobs_list[pos].status = 'E';
    jobs_list[pos].cmd[strlen(jobs_list[pos].cmd - 1)] = '&';
    // Se envia la señal de continuar
    if (kill(jobs_list[pos].pid, SIGCONT) != 0) // si ha habido error entra en el if
    {
        perror("kill");
        exit(FAILURE);
    }
    // Se imprime por pantalla
    fprintf(stdout, "[%d]%d\t%c\t%s\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
    return SUCCESS;
}

void reaper(int signum) // Manejador propio para la señal SIGCHLD (señal enviada a un proceso cuando uno de sus procesos hijos termina)
{                       // asignamos de nuevo a reaper como manejador de la señal porque en algunos entornos asignará la acción predeterminada después de ser llamado el reaper
    signal(SIGCHLD, reaper);
    pid_t ended;
    int status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (DEBUGN5)
        {
            char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
            sprintf(mensaje, GRIS_T "[reaper()→ recibida señal %d (SIGCHLD)]\n" RESET, SIGCHLD);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
        }
        int pos;
        char planoEjecucion[11];
        char cmd[COMMAND_LINE_SIZE];
        int foreground = ended == jobs_list[0].pid;
        if (foreground) // si el que ha acabado estaba en foreground
        {
            pos = 0; // es el job 0
            strcpy(planoEjecucion, "foreground");
            strcpy(cmd, jobs_list[pos].cmd);
            jobs_list_reset(pos);        // relleno de 0's el cmd y el pid
            jobs_list[pos].status = 'F'; // sustituyo el status por F
        }
        else
        {
            pos = jobs_list_find(ended);
            strcpy(planoEjecucion, "background");
            strcpy(cmd, jobs_list[pos].cmd);
            jobs_list_remove(pos); // elimina el job al haber finalizado
        }
        if (DEBUGN5)
        {
            char mensaje[12000];      // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
            if (!WIFSIGNALED(status)) // si no ha sido abortado
            {
                sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en %s (%s) finalizado con exit code %d]\n" RESET, ended, planoEjecucion, cmd, WEXITSTATUS(status));
            }
            else // si ha sido abortado
            {
                sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en %s (%s) finalizado por señal %d]\n" RESET, ended, planoEjecucion, cmd, WTERMSIG(status));
            }
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
        }
        if (!foreground) // si es en background imprime su terminación
        {
            char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
            sprintf(mensaje, "\nTerminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, cmd, pos, status);
            write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
        }
    }
}

void ctrlc(int signum) // Manejador propio para la señal SIGINT (Ctrl+C)
{
    signal(SIGINT, ctrlc); // asignamos de nuevo a ctrlc como manejador de la señal
    char mensaje[2];
    sprintf(mensaje, "\n");
    write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
    if (DEBUGN4 || DEBUGN5)
    {
        char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
        sprintf(mensaje, GRIS_T "[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n[ctrlc()→ recibida señal %d (SIGINT)]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd, signum);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }

    if (jobs_list[0].pid > 0) // si hay un proceso en primer plano
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) // Si el proceso en foreground NO es el mini shell entonces
        {
            if (DEBUGN4 || DEBUGN5)
            {
                char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
                sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 (SIGTERM) enviada a %d (%s) por %d (%s)]\n" RESET, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            if (kill(jobs_list[0].pid, SIGTERM) != 0) // Enviamos la señal SIGTERM al proceso, y si ha habido error entra en el if
            {
                perror("kill");
                exit(FAILURE);
            }
        }
        else if (DEBUGN4 || DEBUGN5)
        {
            char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
            sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 (SIGTERM) no enviada por %d (%s) debido a que el proceso en foreground es el minishell]\n" RESET, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
        }
    }
    else if (DEBUGN4 || DEBUGN5)
    {
        char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
        sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }
}

void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    char mensaje[2];
    sprintf(mensaje, "\n");             // salto de línea para que sea más bonito
    write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
    if (DEBUGN5)
    {
        char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
        sprintf(mensaje, GRIS_T "\n[ctrlz()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n[ctrlc()→ recibida señal %d (SIGTSTP)]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd, signum);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }
    if (jobs_list[0].pid > 0) // si hay un proceso en primer plano
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) // Si el proceso en foreground NO es el mini shell entonces
        {
            if (DEBUGN5)
            {
                char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
                sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d (SIGSTOP) enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            if (kill(jobs_list[0].pid, SIGSTOP) != 0) // Enviamos la señal SIGSTOP al proceso, y si ha habido error entra en el if
            {
                perror("kill");
                exit(FAILURE);
            }
            /// Cambiar el status del proceso a ‘D’ (detenido).
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd); // Utilizo jobs_list_add() para incorporar el proceso a la tabla jobs_list[ ] por el final.
            jobs_list_reset(0);                                     // Reseteo los datos de jobs_list[0] ya que el proceso ha dejado de ejecutarse en foreground.
        }
        else if (DEBUGN5)
        {
            char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
            sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d (SIGSTOP) no enviada por %d (%s) debido a que el proceso en foreground es el minishell]\n" RESET, SIGSTOP, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
        }
    }
    else if (DEBUGN5)
    {
        char mensaje[12000]; // creo un String arbitrariamente grande que será contenedor de lo que se va a imprimir
        sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d (SIGSTOP) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGSTOP, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }
}

int is_background(char **args) // Devuelve 1 si localiza el token & (background) en args[] y 0 en caso contrario (foreground).
{
    for (int i = 1; i < ARGS_SIZE && args[i] != NULL; i++) // comprueba desde el segundo argumento si está '&'
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL; // Sustituye el token & por NULL
            return 1;
        }
    }
    return 0;
}

void jobs_list_reset(int idx)
{
    jobs_list[idx].pid = 0;
    jobs_list[idx].status = 'N';
    memset(jobs_list[idx].cmd, '\0', COMMAND_LINE_SIZE);
}

void jobs_list_update(int idx, pid_t pid, char status, char cmd[])
{
    jobs_list[idx].pid = pid;
    jobs_list[idx].status = status;
    strcpy(jobs_list[idx].cmd, cmd);
}

int jobs_list_add(pid_t pid, char status, char *cmd) // añade un job a la lista
{
    if (n_pids >= N_JOBS)
    {
        return FAILURE; // no se pueden añadir más jobs pasando el límite N_JOBS
    }
    n_pids++;                                   // incrementamos el valor de n_pids indicando que añadimos un job
    jobs_list_update(n_pids, pid, status, cmd); // y se añade una vez incrementado
    fprintf(stdout, "[%d]\t%d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
    return SUCCESS;
}

int jobs_list_find(pid_t pid) // Busca en el array de trabajos el PID que recibe como argumento
{
    for (int i = 1; i <= n_pids; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            return i; // retorna la posición de él en la lista de jobs
        }
    }
    return -1; // no lo ha encontrado
}

int jobs_list_remove(int pos) // elimina un job y pone en su posición el último en la lista
{
    if (n_pids < 1)
    {
        return FAILURE; // si no hay jobs
    }
    // mueve el registro del último proceso de la lista a la posición del que eliminamos.
    jobs_list_update(pos, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
    jobs_list_reset(n_pids); // se elimina el job desplazado
    n_pids--;                // se decrementa la variable global n_pids.
    return SUCCESS;
}

int is_output_redirection(char **args)
{
    int i = 0;
    int fd;
    // Se busca el simbolo '>'
    while ((args[i] != NULL) && (strcmp(args[i], ">") != 0))
    {
        i++;
    }
    // Si ha salido porque no lo ha encontrado retornamos false(0)
    if (args[i] == NULL)
    {
        return 0;
    }
    // Si ha salido porque ha encontrado el redireccionamiento cambiamos el flujo de salida al fichero
    args[i] = NULL;
    if (args[i + 1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso:<COMANDO> > <NOMBRE DEL FICHERO>\n" RESET);
        return 0;
    }
    // Se abre el fichero
    if ((fd = open(args[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Error al abrir o crear fichero");
    }
    // Se redirecciona el descriptor 1 a fd
    dup2(fd, 1);
    if ((close(fd)) < 0)
    {
        perror("Error al cerrar fichero");
    }
    return SUCCESS;
}
