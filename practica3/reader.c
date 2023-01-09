#include "my_lib.h"
#include "limits.h"
#define NUM_THREADS 10 // 10 hilos

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) // si no se ha introducido el nombre de la pila
    {
        fprintf(stderr, ROJO "USAGE: ./reader <stack_file>" RESET);
        return -1;
    }

    int min = INT_MAX;
    int max = INT_MIN;
    int sum = 0;
    struct my_stack *pila;
    if ((pila = my_stack_read(argv[1])) != 0) // se carga la pila
    {
        fprintf(stderr, ROJO "Couldn't open stack file %s" RESET, argv[1]);
        return -1;
    }
    struct my_stack_node *nodo = pila->top; // se coge el nodo superior como el que se va a tratar ahora
    int *data = (int *)nodo->data;          // se cogen sus datos

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
        fprintf(stdout, "%d\n", *data);
        nodo = nodo->next;
        data = (int *)nodo->data;
    }
    fprintf(stdout, "stack length: %d", my_stack_len(pila));
    my_stack_visualize(pila);
    // se calcula la media y se imprime por pantalla
    fprintf(stdout, "Items: %d Min: %d Max: %d Average: %d", NUM_THREADS, min, max, sum / NUM_THREADS);

    return 0;
}
