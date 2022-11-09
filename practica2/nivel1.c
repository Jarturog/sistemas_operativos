#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <colores.h>
#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define DEBUGN1 0
#define PROMPT '$'
// declaraciones de funciones
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
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

char *read_line(char *line){
    printf(imprimir_prompt());
    fflush(stdout); // ni idea de lo que hace exactamente esto pero la profe lo puso
    if(fgets(line, COMMAND_LINE_SIZE, stdin)){
        line[COMMAND_LINE_SIZE-1] = '\0';
        return line;
    }
    if(feof(stdin)){ // CTRL+D
        printf("\radiós");
        exit(0);
    }
    return NULL;
}
char *imprimir_prompt(){
    char *s1 = getenv("USER");
    char *s2;
    getcwd(s2, COMMAND_LINE_SIZE);
    return ("%s %s %c ",s1,s2,PROMPT);
}
int execute_line(char *line)
{
    char **tokens;
    // fragmenta line en tokens
    parse_args(tokens, line);
    // comprueba si es un comando interno
    check_internal(tokens);
}
int parse_args(char **args, char *line);
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
    else if(strcmp(args[0], "jobs"))
    {
        internal_jobs(args);
        return 1;
    }
    else if(strcmp(args[0], "fg"))
    {
        internal_fg(args);
        return 1;
    }
    else if(strcmp(args[0], "bg"))
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
int internal_export(char **args);
int internal_source(char **args)
{
    char line[COMMAND_LINE_SIZE];
    FILE *file;

    if (!args[1])
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return -1;
    }
    if (!(file = fopen(args[1], "r")))
    {
        perror(ROJO_T "fopen");
        return -1;
    }
}
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
