#include "my_lib.h"
#include "limits.h"
#define NUM_THREADS 10 // 10 hilos

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) // si no se ha introducido el nombre de la pila
    {
        fprintf(stderr, ROJO "USAGE: ./reader <stack_file>\n" RESET);
        return -1;
    }

    int min = INT_MAX;
    int max = INT_MIN;
    int sum = 0;
    struct my_stack *pila = my_stack_read(argv[1]); // se carga la pila
    if (pila == NULL) // si no existe
    {
        fprintf(stderr, ROJO "Couldn't open stack file %s\n" RESET, argv[1]);
        return -1;
    }
    for (int i = 0; (i < NUM_THREADS) ; i++)
    {
        int *data = my_stack_pop(pila); // se cogen datos
        // Se compara data con el mayor y menor números hasta el momento
        if (*data > max)
        {
            max = *data;
        }
        if (*data < min)
        {
            min = *data;
        }
        sum += *data; // Se suma al total
    }
    fprintf(stdout, "stack length: %d\n", my_stack_len(pila));
    my_stack_visualize(pila);
    // se calcula la media y se imprime por pantalla
    fprintf(stdout, "\nItems: %d Min: %d Max: %d Average: %d\n", NUM_THREADS, min, max, sum / NUM_THREADS);
    my_stack_purge(pila);
    return 0;
}
