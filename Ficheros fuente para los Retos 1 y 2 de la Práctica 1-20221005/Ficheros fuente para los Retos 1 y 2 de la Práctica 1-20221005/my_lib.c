#include "my_lib.h"

size_t my_strlen(const char *str){
    size_t len = 0;
    int i = 0;
    //Mientras no se llegue al final se va sumando uno al contador
    while (str[i]) {
        i++;
        len++;
    }

   return len;
}

int my_strcmp(const char* str1, const char* str2){
    int i, val_ascii = 0;
    //Compara elemento a elemento si son iguales
    for(i=0;(i<my_strlen(str1)) && (val_ascii == 0);i++){
        /*Si detecta que son diferentes hace la resta de los valores ASCII
        y devuelve un numero negativo si str2 es mas grande
        y viceversa*/
        if(str1[i]!=str2[i]){
            val_ascii = (int) str1[i] - str2[i];
        }
    }

    return val_ascii;    
}

char *my_strcpy(char *dest, const char *src){
    return NULL;
}                  

char *my_strncpy(char *dest, const char *src, size_t n){
    return NULL;
} 

char *my_strcat(char *dest, const char *src){
    //Tamaño final de dest
    int MAX = (my_strlen(dest) + my_strlen(src)); 

    //El puntero se situa al final de dest sobre \0
    for(int i=strlen(dest), j=0; i <= MAX; i++, j++){
        //Se escribe el valor de src con indice j en el indice i de dest
        dest[i] = src[j];
    }

    return dest;
}

char *my_strchr(const char *s, int c){
    char *d = NULL;
    //Se recorre s
    for(int i = 0; i < my_strlen(s); i++){
        //si se encuentra c se devuelve el valor del puntero apuntando a c
        if (s[i] == c){
            s += i;
            d = (char *)s;
        }
    }
    //Si no se encuentra c se devuelve NULL
    return d;
}



struct my_stack *my_stack_init(int size){
    //Se crea una pila
    struct my_stack *s;
    //Se reserva la memoria pasada por valor
    s = (struct my_stack *) malloc(sizeof(struct my_stack));
    //Se assignan el tamaño y el puntero por defecto
    s->size = size;
    s->top = NULL;
    //Se devuelve el puntero
    return s;
};

int my_stack_push(struct my_stack *stack, void *data){
    int correcto = -1;
    //Nodo a insertar en la pila
    struct my_stack_node *node;
    //Se reserva el espacio necesario
    node = (struct my_stack_node *) malloc(sizeof(struct my_stack_node));
    //Si hay espacio
    if(node){
        //Se asignan los datos pasados por puntero
        node->data = data;
        //Se apunta al nodo que antes estaba encima
        node->next = stack->top;
        //El puntero de la pila se redirecciona al nuevo nodo
        stack->top = node;
        correcto = 0;
    }

    return correcto;
};

void *my_stack_pop(struct my_stack *stack){
    //Datos del nodo superior
    void *data = NULL;
    //En caso de que haya nodos en la pila se pueden sacar
    if(stack->top){
        //Se crea un nodo temporal
        struct my_stack_node *node;
        //Se asigna al nodo temporal la direccion del nodo superior de la pila
        node = stack->top;
        //Se asignan a datos los datos del nodo superior
        data = node->data;
        //Se cambia el valor del puntero de la pila al nodo siguiente
        stack->top = node->next;
        //Finalmente se libera el espacio del nodo superior
        free(node);
    }

    return data;
};

int my_stack_len(struct my_stack *stack){
    //Contador para contar los nodos
    int counter = 0;
    //Si la pila no esta vacia se cuentan los nodos, si no se devuelve 0 directamente
    if (stack->top){
        //Creamos un nodo temporal
        struct my_stack_node *node;
        //Le asignamos la direccion del nodo superior de la pila
        node = stack->top;
        //Mientras siga habiendo nodos se seguira recorriendo la pila
        while(node){
            //Se aumenta en uno el contador
            counter++;
            //Se avanza al siguiente nodo
            node = node->next;
        }
        //Se libera el espacio del nodo temporal
        free(node);
    }

    return counter;
};

int my_stack_purge(struct my_stack *stack){
    int bytesliberados = 0;
    /**/
    while(stack->top){
        struct my_stack_node *node;
        node = stack->top;
        stack->top = node->next;
        bytesliberados += sizeof(*node);
        bytesliberados += sizeof(node->data);
        free(node);
    }
    bytesliberados += sizeof(*stack);
    free(stack);

    return bytesliberados;
};
