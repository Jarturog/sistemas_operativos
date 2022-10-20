#include <stdio.h>
struct my_stack *my_stack_init (int sizee);
int my_stack_push (struct my_stack *stack, void *data);

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
  
  int my_stack_push (struct my_stack *stack, void *data){
    if(stack && stack->size>0){
      struct my_stack_node *node = malloc(sizeof(struct my_stack_node));
      node->data = data;
      node->next = stack->top;
      stack->top = node;
      return 0;
    }
    return -1;
  }
#include <stdio.h>

struct my_stack *my_stack_init (int size);
int my_stack_push (struct my_stack *stack, void *data);


