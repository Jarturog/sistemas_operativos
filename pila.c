// includes
#include <stdio.h>
// declaración de funciones
struct my_stack *my_stack_init (int size);
int my_stack_push (struct my_stack *stack, void *data);
void *my_stack_pop (struct my_stack *stack);
int my_stack_len (struct my_stack *stack);
int my_stack_purge (struct my_stack *stack);
// uso de la pila para comprobar su funcionamiento en el main
int main() {
  struct my_stack *stack = my_stack_init(10);
  char *data1 = "hola";
  char *data2 = "adios";
  my_stack_push (stack, data1);
  my_stack_push (stack, data2);
  int len = my_stack_len (stack);
  my_stack_purge (stack);
  //char *data3 = my_stack_pop (stack);
  //char *data4 = my_stack_pop (stack);
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
// inicialización de la pila
struct my_stack *my_stack_init(int size){
  if(size < 1){
    return NULL;
  }
  struct my_stack *s = malloc(sizeof(struct my_stack));
  s->size = size;
  s->top = NULL;
  return s;
};
  
int my_stack_push(struct my_stack *stack, void *data){
  if(!stack){ // si no hi ha stack retorna -1
    return -1;
  }
  struct my_stack_node *node; // si no ha pogut reservar espai retorna -1
  if(!(node = malloc(sizeof(struct my_stack_node)))){
    return -1;
  }
  node->data = data;
  node->next = stack->top;
  stack->top = node;
  return 0;
};
  
void *my_stack_pop (struct my_stack *stack){
  if(!stack){
    return -1;
  }
  struct my_stack_node *temp = stack->top;
  stack->top = temp->next;
  void *p = temp->data;
  free(temp);
  return p;
}

int my_stack_len (struct my_stack *stack){
  if(!stack){
    return -1;
  }
  int nodes = 0;
  struct my_stack_node *temp = stack->top;
  while(temp){
    nodes++;
    temp = temp->next;
  }
  return nodes;
}

int my_stack_purge (struct my_stack *stack){
  if(!stack){
    return -1;
  }
  int bytes = 0;
  struct my_stack_node *temp1 = stack->top;
  struct my_stack_node *temp2 = temp1;
  while(temp1){
    temp1 = temp1->next;
    bytes += sizeof(temp2->data);
    free(temp2->data);
    bytes += sizeof(temp2);
    free(temp2);
    temp2 = temp1;
  }
  bytes += sizeof(stack->size);
  free(stack->size);
  bytes += sizeof(stack);
  free(stack);
  return 0;
}
