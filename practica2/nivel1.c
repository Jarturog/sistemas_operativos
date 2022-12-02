#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "colores.h"
//#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 0
#define PROMPT '$'
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
        if (read_line(line))
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
    printf("%s %s %c ", getenv("USER"), cwd, PROMPT);
    fflush(stdout);
}
int execute_line(char *line)
{
    char **tokens;
    // fragmenta line en tokens
    parse_args(tokens, line);
    // comprueba si es un comando interno
    check_internal(tokens);
}
int parse_args(char **args, char *line)
{
    char **args[ARGS_SIZE];
    const char delim[4] = "\t\n\r"; // delimitadores
    int i = 0;                      // índice
    // primer token
    args[0] = strtok(line, delim);
    if (args[0][0] != '#')
    {
        if (DEBUGN1)
        {
            fprintf(stderr, GRIS_T "[parse_args()→token 0: %s]\n" RESET, args[0]);
        }
    }
    else
    {
        args[0] = NULL;
        if (DEBUGN1)
        {
            fprintf(stderr, GRIS_T "[parse_args()→token 0: #inexistente]\n[parse_args()→token 0 corregido: (null)]\n" RESET);
        }
    }
    // resto de tokens
    while (args[i] != NULL && i < ARGS_SIZE - 1)
    {
        i++;
        args[i] = strtok(NULL, delim);
        if (args[i][0] != '#')
        {
            if (DEBUGN1)
            {
                fprintf(stderr, GRIS_T "[parse_args()→token %d: %s]\n" RESET, i, args[i]);
            }
        }
        else
        {
            args[i] = NULL;
            if (DEBUGN1)
            {
                fprintf(stderr, GRIS_T "[parse_args()→token %d: #inexistente]\n[parse_args()→token %d corregido: (null)]\n" RESET, i);
            }
        }
    }
    args[ARGS_SIZE - 1] = NULL; // el último token siempre ha de ser NULL
    return i;
}
int check_internal(char **args)
{
    if (strcmp(args[0], "exit"))
    {
        exit(0);
        return 1;
    }
    else if (strcmp(args[0], "cd"))
    {
        internal_cd(args);
        return 1;
    }
    else if (strcmp(args[0], "export"))
    {
        internal_export(args);
        return 1;
    }
    else if (strcmp(args[0], "source"))
    {
        internal_source(args);
        return 1;
    }
    else if (strcmp(args[0], "jobs"))
    {
        internal_jobs(args);
        return 1;
    }
    else if (strcmp(args[0], "fg"))
    {
        internal_fg(args);
        return 1;
    }
    else if (strcmp(args[0], "bg"))
    {
        internal_bg(args);
        return 1;
    }
    return 0;
}
int internal_cd(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
    }
    return 1;
}
int internal_export(char **args)
{
    if (DEBUGN1)
    {
        fprintf(stderr, GRIS_T "[internal_export()→Esta función asignará valores a variablescd de entorno]\n" RESET);
    }
    return 1;
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
