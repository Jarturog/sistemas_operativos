#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "colores.h"
#include <sys/wait.h>
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64
#define DEBUGN1 -1
#define DEBUGN2 -1
#define DEBUGN3 1
#define PROMPT '$'
#define SUCCESS 0
#define FAILURE -1
static char mi_shell[COMMAND_LINE_SIZE];

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
    signal(SIGCHLD, reaper); // escucha las señales  SIGCHLD y SIGINT
    signal(SIGINT, ctrlc);
    // Se inicializa la linia de comandos, el job_List y la variable mi_shell
    char line[COMMAND_LINE_SIZE];
    jobs_list_reset(0);
    strcpy(mi_shell, argv[0]);

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
        printf("\radiós");
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
    // fragmenta line en args
    parse_args(args, line);
    // comprueba si es un comando interno
    if (!check_internal(args))
    {
        // Si no es un comando interno se crea un hilo para ejecutar el comando
        pid_t pid = fork();
        int status;

        if (pid == 0) // Proceso hijo
        {
            if (DEBUGN3)
            {
                fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)]\n" RESET, getpid(), line);
            }
            signal(SIGCHLD, SIG_DFL); // Asocia la acción por defecto a SIGCHLD
            signal(SIGINT, SIG_IGN);  // ignorará la señal SIGINT
            execvp(args[0], args);
            perror("No se encontro la orden");
            exit(FAILURE);
        }
        else if (pid > 0) // Proceso padre
        {
            if (DEBUGN3)
            {
                fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n" RESET, getppid(), mi_shell);
            }
            signal(SIGINT, ctrlc); // Asocia el manejador ctrlc a la señal SIGINT
            jobs_list_update(0, pid, 'E', line);
            while (jobs_list[0].pid > 0)
            { // cuando finaliza el proceso en primer plano sale del while
                reaper(SIGINT);
            }
        }
        else // Error
        {
        }
        if (DEBUGN3)
        {
            fprintf(stderr, GRIS_T "[execute_line()→ Proceso hijo %d (%s) finalizado con exit(), estado: %d]\n" RESET, pid, line, status);
        }
    }
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
        if (DEBUGN1)
        {
            fprintf(stderr, GRIS_T "[parse_args()→token %d corregido: (null)]\n" RESET, i, args[i]);
        }
        args[i] = NULL;
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
            if (DEBUGN1)
            {
                fprintf(stderr, GRIS_T "[parse_args()→token %d corregido: (null)]\n" RESET, i, args[i]);
            }
            args[i] = NULL;
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
        /*if (strcmp(cwd, home)) // si ya se está en el directorio HOME no se puede subir más
        {
            perror("internal_cd() error, access denied into a folder above HOME");
            return FAILURE;
        }*/
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
    else
    {
        fflush(file);
        while (fgets(line, COMMAND_LINE_SIZE, file))
        {
            line[strlen(line) - 1] = '\0';

            if (DEBUGN3)
            {
                fprintf(stderr, GRIS_T "[Internal_source()→ LINE: %s]\n" RESET, line);
            }

            execute_line(line);
            fflush(file);
        }

        fclose(file);
    }
    return SUCCESS;
}

int internal_jobs(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_jobs()→Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
    }
    return 1;
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
    return 1;
}

void reaper(int signum)
{                            // Manejador propio para la señal SIGCHLD (señal enviada a un proceso cuando uno de sus procesos hijos termina)
    signal(SIGCHLD, reaper); // asignamos de nuevo a reaper como manejador de la señal
    pid_t ended;
    printf("reaper()→ Recibida señal %d.\n", signum); // la señal 17 es SIGCHILD
    int status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // si ended es el pid del hijo en primer plano entonces resetea jobs_list[0]

        printf("reaper()→ Enterrado proceso con PID %d.", ended);
        jobs_list_reset(0);
    }
}

void ctrlc(int signum)
{                          // Manejador propio para la señal SIGINT (Ctrl+C).
    signal(SIGINT, ctrlc); // asignamos de nuevo a ctrlc como manejador de la señal

    if (jobs_list[0].pid > 0)
    { // si hay un proceso en primer plano
        if (jobs_list[0].pid != getppid())
        {                                              // Si el proceso en foreground NO es el mini shell entonces
            signal(SIGTERM, ctrlc);                    // enviala señal SIGTERM
            printf(stderr, GRIS_T "proceso abortado"); // notificarlo por pantalla
        }
        else
        {
            fprintf(stderr, GRIS_T "Señal SIGTERM no enviada debido a que el proceso en foreground es el shell");
        }
    }
    else
    {
        fprintf(stderr, GRIS_T "Señal SIGTERM no enviada debido a que no hay proceso en foreground");
    }
}