// código hecho por Juan Arturo Abaurrea Calafell, Pere Antoni Prats Villalonga y Marta González Juan
#include "my_lib.h"

struct my_stack *my_stack_init(int size)
{
  if (size < 1)
  {
    return NULL;
  }
  // Se crea una pila
  struct my_stack *s;
  // Se reserva la memoria pasada por valor
  s = (struct my_stack *)malloc(sizeof(struct my_stack));
  // Se assignan el tamaño y el puntero por defecto
  s->size = size;
  s->top = NULL;
  // Se devuelve el puntero
  return s;
};

int my_stack_push(struct my_stack *stack, void *data)
{
  int correcto;
  if (!stack)
  { // si no hay stack devuelve -1
    correcto = -1;
  }
  else
  {
    // Nodo a insertar en la pila
    struct my_stack_node *node;
    // Se reserva el espacio necesario
    node = (struct my_stack_node *)malloc(sizeof(struct my_stack_node));
    // Si hay espacio
    if (node)
    {
      // Se asignan los datos pasados por puntero
      node->data = data;
      // Se apunta al nodo que antes estaba encima
      node->next = stack->top;
      // El puntero de la pila se redirecciona al nuevo nodo
      stack->top = node;
      correcto = 0;
    }
  }

  return correcto;
};

void *my_stack_pop(struct my_stack *stack)
{
  // Datos del nodo superior
  void *data = NULL;
  if (stack)
  {
    // En caso de que haya nodos en la pila se pueden sacar
    if (stack->top)
    {
      // Se crea un nodo temporal
      struct my_stack_node *node;
      // Se asigna al nodo temporal la direccion del nodo superior de la pila
      node = stack->top;
      // Se asignan a datos los datos del nodo superior
      data = node->data;
      // Se cambia el valor del puntero de la pila al nodo siguiente
      stack->top = node->next;
      // Finalmente se libera el espacio del nodo superior
      free(node);
    }
  }
  return data;
};

int my_stack_len(struct my_stack *stack)
{
  // Contador para contar los nodos
  int counter = 0;
  // Si la pila no esta vacia se cuentan los nodos, si no se devuelve 0 directamente
  if (stack->top)
  {
    // Creamos un nodo temporal
    struct my_stack_node *node;
    // Le asignamos la direccion del nodo superior de la pila
    node = stack->top;
    // Mientras siga habiendo nodos se seguira recorriendo la pila
    while (node)
    {
      // Se aumenta en uno el contador
      counter++;
      // Se avanza al siguiente nodo
      node = node->next;
    }
    // Se libera el espacio del nodo temporal
    free(node);
  }

  return counter;
};

int my_stack_purge(struct my_stack *stack)
{
  int bytesliberados = 0;
  /*Mientras queden nodos en la pila se realiza lo siguiente*/
  while (stack->top)
  {
    // se crea un nodo temporal
    struct my_stack_node *node;
    // Se le assigna la direccion del original
    node = stack->top;
    // Se cambia la direccion del nod superior al siguiente
    stack->top = node->next;
    // Se cuentan los bytes que se van a liberar
    bytesliberados += sizeof(*node);
    bytesliberados += stack->size;
    // se limpia la direccion del nodo
    free(node);
  }
  // Se añade el tamaño de la pila
  bytesliberados += sizeof(*stack);
  // Se libera la direccion de la pila
  free(stack);

  return bytesliberados;
};

struct my_stack *my_stack_read(char *filename)
{
  int size, file, bytesLeidos;
  // Se abre el fichero
  if (!(file = open(filename, O_RDONLY)))
  {
    return NULL;
  }
  // Se lee el tamaño de los datos y se inicializa la pila con el tamaño leido
  if (read(file, &size, sizeof(int)) < 0)
  {
    return NULL;
  }
  struct my_stack *stack;
  if (!(stack = my_stack_init(size)))
  {
    return NULL;
  }
  // Se van leyendo los datos y haciendo un push hasta acabar el fichero
  void *data;
  if (!(data = malloc(size)))
  {
    return NULL;
  }
  bytesLeidos = read(file, data, size);
  while (bytesLeidos)
  {
    if (bytesLeidos < 0)
    { // bytesLeidos == -1 si ha habido error
      return NULL;
    }
    my_stack_push(stack, data);
    if (!(data = malloc(size)))
    {
      return NULL;
    }
    bytesLeidos = read(file, data, size);
  }
  // Se cierra el fichero
  if (close(file))
  {
    return NULL;
  }

  return stack;
};

int my_stack_write(struct my_stack *stack, char *filename)
{
  int file;
  /*Abrimos el fichero, en caso de que el fichero no se pueda abrir retorna -1,
  en caso contrari escribe los datos del fichero en la pila*/
  if (!(file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)))
  {
    return -1;
  }

  /*En una pila auxiliar se vuelca la pila original para que al
  leerla este en orden*/
  struct my_stack *aux_stack;
  if (!(aux_stack = my_stack_init(stack->size)))
  {
    return -1; // si error devuelve -1
  }
  struct my_stack_node *n = stack->top;
  for (int i = 0; i < my_stack_len(stack); i++)
  {
    if (my_stack_push(aux_stack, n->data))
    {
      return -1; // si error devuelve -1
    }
    n = n->next;
  }
  // Primero se escribe el tamaño que tendran los datos
  if (!write(file, &stack->size, sizeof(int)))
  {
    return -1;
  }
  // A continuación se recorre la pila
  int size = 0;
  while (aux_stack->top)
  {
    // Se hace un pop de cada nodo y se escriben los datos en el fichero
    if (!write(file, my_stack_pop(aux_stack), aux_stack->size))
    {
      return -1;
    }
    size++;
  }
  // Se cierra el fichero
  if (close(file))
  {
    return -1;
  }
  return size;
};

/**
 * Imprime el contenido de la pila mediante %d (para pilas de números) desde el top a stdout
 */
int my_stack_visualize(struct my_stack *stack)
{
  struct my_stack_node *nodoAuxiliar = stack->top; // nodo auxiliar inicializado al de la cima de la pila
  while (nodoAuxiliar != NULL)                    // mientras haya contenido en la pila
  {
    fprintf(stdout, "%d\n", *(int *)(nodoAuxiliar->data)); // imprime el contenido
    nodoAuxiliar = nodoAuxiliar->next;                     // pasa al siguiente elemento
  }
  return 0;
};
