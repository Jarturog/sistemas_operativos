#include "my_lib.c"
#include "limits.h"
#define NUM_THREADS 10 // 10 hilos

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) // si no se ha introducido el nombre de la pila
    {
        fprintf(stderr, "No se ha especificado un nombre de fichero en los argumentos");
        return -1;
    }

    int min = INT_MAX;
    int max = INT_MIN;
    int sum = 0;
    struct my_stack *pila = my_stack_read(argv[1]); // se carga la pila
    struct my_stack_node *nodo = pila->top;         // se coge el nodo superior como el que se va a tratar ahora
    int *data = (int *)nodo->data;                  // se cogen sus datos

    for (int i = 0; i < NUM_THREADS; i++)
    {
        // Se compara data con el mayor y menor números hasta el momento
        if (*data > max)
        {
            max = *data;
        }
        if (*data < min)
        {
            min = *data;
        }

        // Se suma al total
        sum += *data;

        // Se imprime el valor y se pasa al siguiente nodo
        fprintf(stdout, "%d\n", (int)*data);
        nodo = nodo->next;
        data = (int *)nodo->data;
    }
    // Una vez se sale del bucle se calcula la media y se imprime por pantalla
    fprintf(stdout, "Items: %d Min: %d Max: %d Average: %d", NUM_THREADS, min, max, sum / NUM_THREADS);

    return 0;
}
