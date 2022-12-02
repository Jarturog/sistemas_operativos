#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "colores.h"
//#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 0
#define DEBUGN2 -1
#define PROMPT '$'
#define SUCCESS 0
#define FAILURE -1
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

int main()
{
    char line[COMMAND_LINE_SIZE];
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
    if (fgets(line, COMMAND_LINE_SIZE, stdin) != NULL)
    {
        line[COMMAND_LINE_SIZE - 1] = '\0'; // substituyo el carácter final por \0
        return line;
    } // si es NULL
    if (feof(stdin) )
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
    // fragmenta line en tokens
    if(parse_args(args, line)) // si hay tokens
    { 
        // comprueba si es un comando interno
        check_internal(args);
    }
    return 0;
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
    if(args[i] != NULL && args[i][0] == '#')
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
        if(args[i] != NULL && args[i][0] == '#')
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
    printf("\n%c|%c",args[1][0],args[1][1]);
    printf("\d%c|%d",46-args[1][0],46-args[1][1]);
        fflush(stdout);
    while (args[1][0] == args[1][1] == '.') // si hay ..
    {
        printf("%s",home);
        fflush(stdout);
        if (strcmp(cwd, home) == 0) // si ya se está en el directorio HOME no se puede subir más
        {
            perror("internal_cd() error, access denied into a folder above HOME");
            return FAILURE;
        }
        printf("\n%s",cwd);
        fflush(stdout);
        cwd[strlen(cwd) - 1] = '\0';        // elimino la barra
        while (cwd[strlen(cwd) - 1] != '/') // vuelve atrás una carpeta
        {
            printf("\n%s",cwd);
            fflush(stdout);
            cwd[strlen(cwd) - 1] = '\0';
        }
        printf("\n%s",cwd);
        if (args[1][2] == '/') // si hay dos puntos más
        {
            printf("\n%s",args[1]);
            fflush(stdout);
            args[1] = strchr(args[1], '/'); // no puede dar error porque se ha comprobado que está
            printf("\n%s",args[1]);
            fflush(stdout);
        }
        else
        {
            break;
        }
    }
    printf("\nantes espacios\n");
    // comprobación de espacios en los argumentos y creación del string que se pondrá en el chdir
    char argsToCwd[COMMAND_LINE_SIZE * ARGS_SIZE]; // como máximo el conjunto de los argumentos tendrá este tamaño
    int i = 1;                                     // índice del argumento que se está comprobando
    while (args[i] != NULL)
    {
        if (args[i][strlen(args[i]) - 1] == 92 && args[i + 1] != NULL) // si hay la barra inclinada del revés
        { 
            args[i][strlen(args[i]) - 1] = ' ';
            strcat(argsToCwd, "/");
            strcat(argsToCwd, args[i]);
            strcat(argsToCwd, args[i + 1]);
            i++;
        }
        else if (args[i+1]!=NULL && (args[i][0] == args[i + 1][strlen(args[i + 1]) - 1] == 1) || (args[i][0] == args[i + 1][strlen(args[i + 1]) - 1] == 6))
        { // si hay comillas simples o dobles
            args[i][0] = '/';
            strcat(argsToCwd, args[i]);
            strcat(argsToCwd, " ");
            args[i][strlen(args[i + 1]) - 1] = '\0'; // al quitarle substituir la comilla por el \0 reduzco el tamaño del string
            strcat(argsToCwd, args[i + 1]);
            i++;
        }
        else
        {
            strcat(argsToCwd, "/");
            strcat(argsToCwd, args[i]);
        }
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
    char *nombre;
    char valor[COMMAND_LINE_SIZE];
    nombre = strtok(args[1], "=");
    for(int i = strlen(nombre)+1, j = 0; args[1][i]!='\0';i++,j++){
        valor[j] = args[1][i];
    }
    if (DEBUGN2)
    {
        fprintf(stderr, ROJO_T "[internal_export()→ nombre: %s]\n[internal_export()→ valor: %s]\n" RESET, nombre, getenv(nombre));
    }
    if ((nombre == NULL) || (valor == NULL) || (nombre[0] == '\0') || (valor[0] == '\0'))
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return FAILURE;
    }
    char *vInicial;
    if(!(vInicial = getenv(nombre))){
        perror("getenv() error");
        return FAILURE;
    }
    if(!setenv(nombre,valor,1)){
        perror("setenv() error");
        return FAILURE;
    }
    if (DEBUGN2)
    {
        fprintf(stderr, ROJO_T "[internal_export()→ antiguo valor para %s: %s]\n[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, vInicial, nombre, getenv(nombre));
    }
    return SUCCESS;
}
int internal_source(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_source()→Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
    }
    return 1;
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
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_bg()→Esta función reactivará un proceso detenido para que siga ejecutándose pero en segundo plano.]\n" RESET);
    }
    return 1;
}
