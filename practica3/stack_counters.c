#include <pthread.h>   // hilos
#include "my_lib.h"    // colores y stack
#define NUM_THREADS 10 // 10 hilos
#define N 1000000      // 1000000 iteraciones

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // semáforo global
struct my_stack *pila;                             // pila

// Declaracion de funciones
void *worker(void *ptr);

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) // verifica si la pila existe
    {
        fprintf(stderr, ROJO "USAGE: ./stack_counters <stack_file>\n" RESET);
        return -1;
    }
    fprintf(stdout, "Threads: %d, Iterations: %d\n", NUM_THREADS, N);
    pila = my_stack_read(argv[1]);
    if ((pila == NULL) || (my_stack_len(pila) < NUM_THREADS)) // si la pila es menor que NUM_THREADS o si no existe
    {
        int *data;        // los datos a introducir en la pila
        if (pila == NULL) // si no existe crea una
        {
            pila = my_stack_init(sizeof(data)); // nueva pila
        }
        if (my_stack_len(pila) == 0) // si la longitud es 0 (nueva pila o existía pero vacía)
        {
            fprintf(stdout, "stack->size: %d\n", pila->size); // imprime el tamaño que ocupa cada campo de datos
        }
        fprintf(stdout, "initial stack length: %d\n", my_stack_len(pila));
        fprintf(stdout, "initial stack content:\n");
        my_stack_visualize(pila);             // se visualiza el contenido de la pila
        if ((my_stack_len(pila) < NUM_THREADS) && (my_stack_len(pila) > 0)) // si hay que añadir elementos
        {
            fprintf(stdout, "number of elements added to inital stack: %d\n", (NUM_THREADS - my_stack_len(pila)));
        }
        fprintf(stdout, "stack content for treatment:\n");
        for (int i = my_stack_len(pila); i < NUM_THREADS; i++) // si la pila tiene menos de NUM_THREADS elementos o ninguno
        {
            data = malloc(sizeof(data)); // reserva espacio para el entero
            if (data == NULL) // si ha ocurrido un error para el proceso y lo imprime
            {
                fprintf(stderr, ROJO "Error reservando memoria con malloc()" RESET);
                return -1; 
            }
            *data = 0;                         // se inicializa a 0
            if (my_stack_push(pila, data) < 0) // se agregan los restantes individualmente
            {
                fprintf(stderr, ROJO "Error metiendo el elemento en la pila" RESET);
                return -1; // si ha ocurrido un error para el proceso y lo imprime
            }
        }
        my_stack_visualize(pila);                                      // se visualizan los elementos
        fprintf(stdout, "new stack length: %d\n\n", my_stack_len(pila)); // se imprime la longitud que será NUM_THREADS
    }
    else // en caso de que existe y el contenido sea suficiente solamente se imprimen sus datos y su longitud
    {
        fprintf(stdout, "original stack content:\n");
        my_stack_visualize(pila);
        fprintf(stdout, "original stack length: %d\n\n", my_stack_len(pila));
    }

    pthread_t hilos[NUM_THREADS]; // habrán NUM_THREADS de cantidad de hilos

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&hilos[i], NULL, worker, NULL); // se crean los hilos
        fprintf(stdout, "%d) Thread %ld created\n", i, hilos[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(hilos[i], NULL); // el hilo principal espera a que acaben el resto de hilos
    }
    fprintf(stdout, "\nstack content after threads iterations:\n");
    my_stack_visualize(pila);
    fprintf(stdout, "stack length: %d\n", my_stack_len(pila));

    if (my_stack_write(pila, argv[1]) < 0) // escribe la pila en el fichero de nombre argv[1]
    {
        fprintf(stderr, ROJO "Error escribiendo la pila en %s" RESET, argv[1]);
        return -1; // si ha ocurrido un error para el proceso y lo imprime
    }
    fprintf(stdout, "\nWritten elements from stack to file: %d\n", my_stack_len(pila)); // libera el espacio reservado por la pila
    fprintf(stdout, "Released bytes: %d\n", my_stack_purge(pila));                    // libera el espacio reservado por la pila
    fprintf(stdout, "Bye from main\n");
    pthread_exit(NULL); // fin de la ejecución
}

/**
 * Suma N veces al top en el momento de la pila protegiendo el acceso a ella mediante un semáforo mutex
 */
void *worker(void *ptr) // suma 1 al elemento de la pila
{
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);            // se bloquea la entrada a otros hilos en la siguiente sección crítica
        int *data = (int *)my_stack_pop(pila); // extrae un elemento de la pila
        pthread_mutex_unlock(&mutex);

        (*data)++; // incrementa el valor

        pthread_mutex_lock(&mutex);
        my_stack_push(pila, data);    // se almacena de nuevo en la pila
        pthread_mutex_unlock(&mutex); // se desbloquea la segunda sección crítica
    }
    pthread_exit(NULL); // fin del hilo
}
