#include "my_lib.c"
#include "limits.h"
#define NUM_THREADS 10
struct my_stack *s;
struct my_stack_node *n;

int min = INT_MAX;
int max = 0;
int sum = 0;
int average;
int data;

int main(int argc, char *argv[]){
    if (argv[1] != NULL){
        //Se crea la pila
        s = (struct my_stack *)argv[1];
        n = s->top;
        data = (int)n->data;
        for(int i = 0; i < NUM_THREADS; i++){
            //Se compara data con el mayor y menor numeros hasta el momento
            if(data > max){
                max = data;
            }
            if(data < min)
            {
                min = data;
            }

            //Se suma al total
            sum += data;

            //Se imprime el valor y se pasa al siguiente nodo
            printf("%d\n", data);
            n = n->next;
            data = (int)n->data;
        }
        //Una vez se sale del bucle se calcula la media y se imprime por pantalla
        average = sum / NUM_THREADS;
        printf("Items: %d Min: %d Max: %d Average: %d", NUM_THREADS, min, max, average);
    }
    else
    {
        char mensaje[12000];
        sprintf(mensaje, "No se ha especificado un nombre de fichero: int argc, char *argv[]");
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    }

    return 0;
}
