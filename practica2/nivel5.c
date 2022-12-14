#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64
#define DEBUGN1 0
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 -1
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

static char mi_shell[COMMAND_LINE_SIZE];
static int matado; // booleano que indica si el proceso ha sido matado o ha finalizado normalmente
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

// Lista de tareas
struct info_job
{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

static struct info_job jobs_list[N_JOBS];

// Funcion para inicializar jobs_List
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

int main(int argc, char *argv[])
{
    signal(SIGCHLD, reaper); // llamada al enterrador de zombies cuando un hijo acaba (señal SIGCHLD)
    signal(SIGINT, ctrlc);   // SIGINT es la señal de interrupción que produce Ctrl+C
    signal(SIGTSTP, ctrlz);  // SIGTSTP es la señal de interrupción que produce Ctrl+Z
    // Se inicializan la línea de comandos, el jobs_list y la variable mi_shell
    char line[COMMAND_LINE_SIZE];
    jobs_list_reset(0);
    strcpy(mi_shell, argv[0]);
    matado = 0; // se inicializa el valor a 0 indicando que no ha habido ningún proceso matado
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
        signal(SIGCHLD, SIG_DFL); // Asocia la acción por defecto a SIGCHLD
        signal(SIGINT, SIG_IGN);  // ignorará la señal SIGINT
        signal(SIGTSTP, SIG_IGN); // ignorará la señal SIGTSTP
        if (DEBUGN3 || DEBUGN4)   // hago el OR porque en la página 7 de la documentación del nivel 4 también aparece (aparte de en el del nivel 3)
        {
            fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)]\n" RESET, getpid(), line_inalterada);
        }
        execvp(args[0], args); // si sigue la ejecución es por un error
        fprintf(stderr, ROJO_T "%s: no se encontró la orden\n" RESET, line_inalterada);
        exit(FAILURE);
    }
    else if (pid > 0) // Proceso padre
    {
        if (DEBUGN3 || DEBUGN4) // hago el OR porque en la página 7 de la documentación del nivel 4 también aparece
        {
            fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n" RESET, getpid(), mi_shell);
        }

        if (!background)
        {
            jobs_list_update(0, pid, 'E', line_inalterada);
            while (jobs_list[0].pid > 0)
            { // cuando finaliza el proceso en primer plano sale del while
                pause();
            }
        }
        else
        {
            jobs_list_add(getpid(), 'E', line_inalterada); // si se ejecuta en background lo incorpora a la lista de trabajos jobs_list[ ]
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
            strcat(argsToCwd, args[i]);
            strcat(argsToCwd, " ");
            i++;
        }
        if (args[i][0] == 1 || args[i][0] == 6) // si hay comillas simples o dobles
        {
            int tipoComa = args[i][0];
            args[i]++; // paso por encima de la comilla
            printf("\n%s\n%c", args[i], tipoComa);
            fflush(stdout);
            while (args[i] != NULL && args[i][0] != '\0' && (args[i][strlen(args[i]) - 1] == 1 || args[i][strlen(args[i]) - 1] == 6))
            {
                strcat(argsToCwd, args[i]);
                strcat(argsToCwd, " ");
                i++;
            }
            if (args[i][strlen(args[i]) - 1] != tipoComa) // si no es " " o ' '
            {
                perror("Error en internal_cd() por comillas diferentes");
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
        fprintf(stdout, GRIS_T "[%d]\t%d\t%s\t%c\n" RESET, i, jobs_list[i].pid, jobs_list[i].cmd, jobs_list[i].status);
    }
    return SUCCESS;
}

int internal_fg(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_fg()→Esta función enviará un trabajo del background al foreground, o reactivará la ejecución en foreground de un trabajo que había sido detenido.]\n" RESET);
    }
    return 1;
}

int internal_bg(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_bg()→Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano.]\n" RESET);
    }
    return 1;
}

void reaper(int signum)      // Manejador propio para la señal SIGCHLD (señal enviada a un proceso cuando uno de sus procesos hijos termina)
{                            // asignamos de nuevo a reaper como manejador de la señal porque en algunos entornos asignará la acción predeterminada
    signal(SIGCHLD, reaper); // después de la asignación que hemos hecho
    pid_t ended;
    int status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (ended == jobs_list[0].pid) // si el que ha acabado estaba en foreground
        {
            if (DEBUGN4) // Enviamos la señal SIGINT al proceso
            {
                char mensaje[1200];
                if (!matado) // si no ha sido abortado
                {
                    sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n" RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
                }
                else // si ha sido abortado
                {
                    sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n" RESET, ended, jobs_list[0].cmd, SIGTERM);
                }
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            jobs_list_reset(0);
        }
        else
        {
            int pos = jobs_list_find(ended);
            char mensaje[1200];
            sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n" RESET, ended, jobs_list[0].cmd, SIGTERM);
            write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
            jobs_list_remove(pos);              // elimina el job al haber finalizado
        }
        matado = 0; // reinicio matado a 0
    }
}

void ctrlc(int signum) // Manejador propio para la señal SIGINT (Ctrl+C)
{
    signal(SIGINT, ctrlc); // asignamos de nuevo a ctrlc como manejador de la señal
    char mensaje[2];
    sprintf(mensaje,"\n");
    write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
    if (DEBUGN4 || DEBUGN5)
    {
        char mensaje[1200];
        sprintf(mensaje, GRIS_T "[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n[ctrlc()→ recibida señal %d (S)]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd, signum, signum);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }

    if (jobs_list[0].pid > 0) // si hay un proceso en primer plano
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) // Si el proceso en foreground NO es el mini shell entonces
        {
            if (DEBUGN4 || DEBUGN5)
            {
                char mensaje[1200];
                sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 enviada a %d (%s) por %d (%s)]\n" RESET, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            if (kill(jobs_list[0].pid, SIGTERM) != 0) // Enviamos la señal SIGTERM al proceso, y si ha habido error entra en el if
            {
                perror("kill");
                exit(FAILURE);
            }
            matado = -1; // al asignar -1 indica que el proceso ha sido matado
        }
        else if (DEBUGN4 || DEBUGN5)
        {
            char mensaje[1200];
            sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 no enviada por %d (%s) debido a que el proceso en foreground es el minishell]\n" RESET, getppid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
        }
    }
    else if (DEBUGN4 || DEBUGN5)
    {
        char mensaje[1200];
        sprintf(mensaje, GRIS_T "[ctrlc()→ Señal 15 no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }
}

void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    char mensaje[2];
    sprintf(mensaje,"\n");
    write(1, mensaje, strlen(mensaje)); // 1 es el flujo stdout
    if (jobs_list[0].pid > 0) // si hay un proceso en foreground
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0) // Si el proceso en foreground NO es el mini shell entonces
        {
            if (kill(jobs_list[0].pid, SIGSTOP) != 0) // Enviamos la señal SIGSTOP al proceso, y si ha habido error entra en el if
            {
                perror("kill");
                exit(FAILURE);
            }
                             /// Cambiar el status del proceso a ‘D’ (detenido).
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd); // Utilizar jobs_list_add() para incorporar el proceso a la tabla jobs_list[ ] por el final.
            jobs_list_reset(0);                              // Resetear los datos de jobs_list[0] ya que el proceso ha dejado de ejecutarse en foreground.
        }
        else if (DEBUGN5)
        {
            char mensaje[1200];
            sprintf(mensaje, GRIS_T "[texto1]\n" RESET);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr  
        }
    }
    else if (DEBUGN5)
    {
        char mensaje[1200];
        sprintf(mensaje, GRIS_T "[texto2]\n" RESET);
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

int jobs_list_add(pid_t pid, char status, char *cmd) // añade un job a la lista
{
    if (n_pids >= N_JOBS)
    {
        return FAILURE; // no se pueden añadir más jobs pasando el límite N_JOBS
    }
    jobs_list_update(n_pids, pid, status, cmd);
    n_pids++; // incrementamos el valor de n_pids indicando que añadimos un job
    return SUCCESS;
}

int jobs_list_find(pid_t pid) // Busca en el array de trabajos el PID que recibe como argumento
{
    for (int i = 0; i < n_pids; i++)
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
    jobs_list_update(pos, jobs_list[n_pids - 1].pid, jobs_list[n_pids - 1].status, jobs_list[n_pids - 1].cmd);
    jobs_list_reset(pos); // se elimina el job desplazado
    n_pids--;             // se decrementa la variable global n_pids.
    return SUCCESS;
}
