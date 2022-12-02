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
int parse_args(char **args, char *line){
  return 1;
}
int execute_line(char *line){
  return 1;
}
int check_internal(char **args){
  return 1;
}
int internal_cd(char **args){
  return 1;
}
int internal_export(char **args){
  return 1;
}
int internal_source(char **args){
  return 1;
}
int internal_jobs(char **args){
  return 1;
}
int internal_fg(char **args){
  return 1;
}
int internal_bg(char **args){
  return 1;
}
