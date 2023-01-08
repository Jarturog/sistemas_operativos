#include <pthread.h>
#include "my_lib.c"
#define NUM_THREADS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // semaforo global
struct my_stack *s;
int N = 1000000;
// Declaracion de funciones
void *worker(void *ptr);

int main(int argc, char *argv[])
{

    int *data = 0;
    if (argv[1] != NULL)
    {                                   // verifica si la pila existe
        s = (struct my_stack *)argv[1]; // asigna la pila cuyo nombre del fichero se ha pasado por consola
        if (s->size < 10) // si la pila tiene menos de 10 elementos o ninguno
        { 
            for (int i = s->size; i < 10; i++)
            {
                if (!(data = malloc(sizeof(data)))) // reserva espacio para el entero
                { 
                    fprintf(stderr, "Error reservando memoria con malloc()");
                    return -1; // si ha ocurrido un error para el proceso y lo imprime
                }
                data = malloc(sizeof(data));
                if (my_stack_push(s, data) < 0) // se agregan los restantes individualmente
                {
                    fprintf(stderr, "Error metiendo el elemento en la pila");
                    return -1; // si ha ocurrido un error para el proceso y lo imprime
                }
            }
        }
        my_stack_purge(s); // libera el espacio reservado con malloc()
    }
    else
    {
        fprintf(stderr, "No se ha especificado un nombre de fichero: int argc, char *argv[]");
        s = my_stack_init(NUM_THREADS);
    }

    pthread_t hilos[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&hilos[i], NULL, worker, NULL);  
    }
    for(int i=0; i<NUM_THREADS;i++){
        pthread_join(hilos[i], NULL);
    }
    
    my_stack_write(s, argv[1]);
    pthread_exit(0);
}

void *worker(void *ptr)
{
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);
        int *data = (int *)my_stack_pop(s); // extrae un elemento de la pila
        (*data)++;                            // incrementa el valor
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        my_stack_push(s, data); // se almacena de nuevo en la pila
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}