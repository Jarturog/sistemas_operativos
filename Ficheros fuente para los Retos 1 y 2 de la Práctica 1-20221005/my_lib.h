/* lib.h librería con las funciones equivalentes a las
de <string.h> y las funciones y estructuras para el
manejo de una pila */

#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <fcntl.h>     /* Modos de apertura de función open()*/
#include <sys/stat.h>  /* Permisos función open() */
#include <sys/types.h> /* Definiciones de tipos de datos como size_t*/
#include <unistd.h>    /* Funciones read(), write(), close()*/
#include <errno.h>     /* COntrol de errores (errno) */

//declaraciones funciones libreria string
size_t my_strlen(const char *str);                              //Fet
int my_strcmp(const char *str1, const char *str2);              //Marta
char *my_strcpy(char *dest, const char *src);                   //Arturo
char *my_strncpy(char *dest, const char *src, size_t n);        //Arturo
char *my_strcat(char *dest, const char *src);                   //P.A
char *my_strchr(const char *s, int c);                          //P.A

// char *my_strncat(char *dest, const char *src, size_t n);

int my_strcmp(const char* str1, const char* str2){
    int i;
    //Compara elemento a elemento si son iguales
    for(i=0;i<strlen(str1);i++){
        /*Si detecta que son diferentes hace la resta de los valores ASCII
        y devuelve un numero negativo si str2 es mas grande
        y viceversa*/
        if(str1[i]!=str2[i]){
            return str1[i] - str2[i];
        }
    }
    //en caso de salir del for significa que son iguales
    return 0;    
}


char *my_strcat(char *dest, const char *src){
    //Tamaño final de dest
    int MAX = (strlen(dest) + strlen(src)); 

    //El puntero se situa al final de dest sobre \0
    for(int i=strlen(dest), j=0; i <= MAX; i++, j++){
        //Se escribe el valor de src con indice j en el indice i de dest
        dest[i] = src[j];
    }

    return dest;
}

char *my_strchr(const char *s, int c){
    //Se recorre s
    for(int i = 0; i < strlen(s); i++){
        //si se encuentra c se devuelve el valor del puntero apuntando a c
        if (s[i] == c){
            s += i;
            return (char *)s;
        }
    }
    //Si no se encuentra c se devuelve NULL
    return NULL;
}

//Estructuras para gestor de pila
struct my_stack_node {      // nodo de la pila (elemento)
    void *data;
    struct my_stack_node *next;
};

struct my_stack {   // pila
    int size;       // tamaño de data, nos lo pasarán por parámetro
    struct my_stack_node *top;  // apunta al nodo de la parte superior
};  

//declaraciones funciones gestor de pila
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack); 
struct my_stack *my_stack_read(char *filename);
int my_stack_write(struct my_stack *stack, char *filename);
    
