#include <pthread.h>
#include "my_lib.c"
#define NUM_THREADS 10

pthread_mutex_t *mutex = PTHREAD_MUTEX_INITIALIZER; // semaforo global
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
        if (s->size < 10)
        { // si la pila tiene menos de 10 elementos o ninguno
            for (int i = s->size; i < 10; i++)
            {
                malloc(sizeof(data));   // reserva espacio para el entero
                my_stack_push(s, data); // se agregan los restantes individualmente
            }
        }
        free(my_stack_purge(s)); // libera el espacio reservado con malloc()
    }
    else
    {

        char mensaje[12000];
        sprintf(mensaje, "No se ha especificado un nombre de fichero: int argc, char *argv[]");
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr


        s = my_stack_init(NUM_THREADS);
    }
    pthread_t *hilos[NUM_THREADS];
      
    for (int i = 0; i < NUM_THREADS; i++)
    {
        
        pthread_create(*hilos[i], NULL, worker, NULL);
    }
    pthread_join(mutex, NULL);
    my_stack_write(s,argv);
    pthread_exit(0);
}
void *worker(void *ptr)
{
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(mutex) ;
        int *data;
        data = (int) my_stack_pop(s);  //extrae un elemento de la pila
        data++;  //incrementa el valor
        pthread_mutex_unlock(mutex);
        pthread_mutex_lock(mutex) ;
        my_stack_push(s,data);  //se almacena de nuevo en la pila
        pthread_mutex_unlock(mutex);
    }
    pthread_exit(NULL);
}