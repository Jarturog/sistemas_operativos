#include <stdio.h>
struct my_stack *my_stack_init (int sizee);

int main() {
  struct my_stack *stack = my_stack_init(3);
  return 0;
}

struct my_stack_node {      // nodo de la pila (elemento)
  void *data;
  struct my_stack_node *next;
};
 
struct my_stack {   // pila
  int size;       // tamaño del contenido apuntado por data, nos lo pasarán por parámetro
  struct my_stack_node *top;  // apunta al nodo de la parte superior
};

struct my_stack *my_stack_init (int size){
  struct my_stack *stack = malloc(sizeof(struct my_stack));
  stack->size = size;
  stack->top = NULL;
  return stack;
  }
  
  // inacabado
  int my_stack_push (struct my_stack *stack, void *data){
    if(stack && stack->size>0){
      struct my_stack_node *node = malloc(sizeof(struct my_stack_node));
      node->data = data;
      node->next = 
    }
    return -1;
    }
/*
Inserta un nuevo nodo en los elementos de la pila
(hay que reservar espacio de memoria para él).
El puntero a los datos de ese nodo (data) nos lo pasan como parámetro.
El puntero al struct my_stack, que nos lo pasan por parámetro (stack),
tiene que existir (stack!=NULL) y el tamaño de los datos ha de ser >0
(stack->size>0), o sea se tiene que haber ejecutado adecuadamente
my_stack_init()
Hay que reservar espacio de memoria para el nuevo nodo y enlazarlo
*/
adecuadamente por la parte superior.
