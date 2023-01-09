#include <pthread.h>
#include "my_lib.c"
#define NUM_THREADS 10 // 10 hilos
#define N 1000000      // 1000000 iteraciones

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // semáforo global
struct my_stack *pila;                             // pila

// Declaracion de funciones
void *worker(void *ptr);

int main(int argc, char *argv[])
{
    int *data; // los datos a introducir en la pila

    if (argv[1] != NULL) // verifica si la pila existe
    {
        pila = my_stack_read(argv[1]); // asigna la pila cuyo nombre del fichero se ha pasado por consola
    }
    else // en caso contrario
    {
        fprintf(stderr, "No se ha especificado un nombre de fichero: int argc, char *argv[]");
        pila = my_stack_init(sizeof(data)); // crea la pila desde cero
    }

    for (int i = my_stack_len(pila); i < NUM_THREADS; i++) // si la pila tiene menos de NUM_THREADS elementos o ninguno
    {
        if (!(data = malloc(sizeof(data)))) // reserva espacio para el entero
        {
            fprintf(stderr, "Error reservando memoria con malloc()");
            return -1; // si ha ocurrido un error para el proceso y lo imprime
        }
        *data = 0;
        if (my_stack_push(pila, data) < 0) // se agregan los restantes individualmente
        {
            fprintf(stderr, "Error metiendo el elemento en la pila");
            return -1; // si ha ocurrido un error para el proceso y lo imprime
        }
    }

    pthread_t hilos[NUM_THREADS]; // habrán NUM_THREADS de cantidad de hilos

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&hilos[i], NULL, (void *) &worker, NULL); // se crean los hilos
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(hilos[i], NULL); // el hilo principal espera a que acaben el resto de hilos
    }

    if (my_stack_write(pila, argv[1]) < 0) // escribe la pila en el fichero de nombre argv[1]
    {
        fprintf(stderr, "Error escribiendo la pila en %s", argv[1]);
        return -1; // si ha ocurrido un error para el proceso y lo imprime
    }
    free(data);           // libera espacio reservado por data
    my_stack_purge(pila); // libera el espacio reservado por la pila
    pthread_exit(NULL);   // fin de la ejecución
}

void *worker(void *ptr) // suma 1 al elemento de la pila
{
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);            // se bloquea la entrada a otros hilos en la siguiente sección crítica
        int *data = (int *)my_stack_pop(pila); // extrae un elemento de la pila
        (*data)++;                             // incrementa el valor
        my_stack_push(pila, data);             // se almacena de nuevo en la pila
        pthread_mutex_unlock(&mutex);          // se desbloquea la sección crítica para que otro hilo o el mismo haga la operación
    }
    pthread_exit(NULL); // fin del hilo
}