// código hecho por Juan Arturo Abaurrea Calafell, Pere Antoni Prats Villalonga y Marta González Juan
#include "my_lib.h"

size_t my_strlen(const char *str){
    size_t len = 0;
    int i = 0;
    //Mientras no se llegue al final se va sumando uno al contador
    while (str[i]) {
        i++;
        len++;
    }

   return len;
}

int my_strcmp(const char* str1, const char* str2){
    int i, val_ascii = 0;
    //Compara elemento a elemento si son iguales
    for(i=0;(i<my_strlen(str1)) && (val_ascii == 0);i++){
        /*Si detecta que son diferentes hace la resta de los valores ASCII
        y devuelve un numero negativo si str2 es mas grande
        y viceversa*/
        if(str1[i]!=str2[i]){
            val_ascii = (int) str1[i] - str2[i];
        }
    }

    return val_ascii;    
}

char *my_strcpy(char *dest, const char *src){
    int i = 0;
    while(src[i]){
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}                  

char *my_strncpy(char *dest, const char *src, size_t n){
    int i = 0;
    /*Mientras haya contenido que copiar en src y el contenido copiado
    sea mas pequeño que el indicado se copia el char en la posición [i]
    de src a dest*/
    while(src[i] && n > 0){
        dest[i] = src[i];
        i++;
        n--;
    }
    /*Cuando el contenido en src deseado se haya copiado a dest se llenaran
    los espacios disponibles con \n*/
    while(n > 0){
        dest[i++] = '\0';
        n--;
    }
    return dest;
} 

char *my_strcat(char *dest, const char *src){
    //Tamaño final de dest
    int MAX = (my_strlen(dest) + my_strlen(src)); 

    //El puntero se situa al final de dest sobre \0
    for(int i=strlen(dest), j=0; i <= MAX; i++, j++){
        //Se escribe el valor de src con indice j en el indice i de dest
        dest[i] = src[j];
    }

    return dest;
}

char *my_strchr(const char *s, int c){
    char *d = NULL;
    //Se recorre s
    for(int i = 0; i < my_strlen(s); i++){
        //si se encuentra c se devuelve el valor del puntero apuntando a c
        if (s[i] == c){
            s += i;
            d = (char *)s;
        }
    }
    //Si no se encuentra c se devuelve NULL
    return d;
}


struct my_stack *my_stack_init(int size){
  if(size < 1){
    return NULL;
  } //Se crea una pila y se reserva la memoria pasada por valor
  struct my_stack *s = malloc(sizeof(struct my_stack));
  s->size = size; //Se asignan el tamaño y el puntero por defecto
  s->top = NULL;
  return s; //Se devuelve el puntero
};

int my_stack_push(struct my_stack *stack, void *data){
  if(!stack){ // si no hay stack devuelve -1
    return -1;
  }
  struct my_stack_node *node; // si no ha podido reservar espacio devuelve -1
  if(!(node = malloc(sizeof(struct my_stack_node)))){
    return -1;
  }
  node->data = data; //Se asignan los datos pasados por puntero
  node->next = stack->top; //Se apunta al nodo que antes estaba encima
  stack->top = node; //El puntero de la pila se redirecciona al nuevo nodo
  return 0;
};

void *my_stack_pop(struct my_stack *stack){
  if(!stack){
    return NULL;
  }
  if(!stack->top){ // faig aquesta comprovació després i no abans
    return NULL; // perquè si no hagués stack hauria un error
  }
  struct my_stack_node *node = stack->top; //Se crea un nodo temporal y se asigna la dirección del nodo superior de la pila
  void *data = node->data; //Se asignan a datos los datos del nodo superior
  stack->top = node->next; //Se cambia el valor del puntero de la pila al nodo siguiente
  free(node); //Finalmente se libera el espacio del nodo superior
  return data;
};

int my_stack_len(struct my_stack *stack){
  if(!stack){
    return -1;
  }
  struct my_stack_node *node; // nodo temporal
  if (!(node = stack->top)){ //Si la pila está vacía la longitud es 0
    return 0;
  }
  int counter = 0;
  while(node){ // mientras no sea NULL
    counter++;
    node = node->next;
  }
  return counter;
};

int my_stack_purge(struct my_stack *stack){
    int bytesliberados = 0;
    while(stack->top){ // mientras queden nodos
        struct my_stack_node *node = stack->top;
        stack->top = node->next; // ahora el top es el siguiente
        bytesliberados += sizeof(*node);
        bytesliberados += sizeof(node->data);
        free(node); // se purga el nodo
    }
    bytesliberados += sizeof(*stack);
    free(stack); // se purga lo que queda de pila
    return bytesliberados;
};

struct my_stack *my_stack_read(char *filename){
  int size, file, bytesLeidos; 
  // Se abre el fichero
  if(!(file = open(filename, O_RDONLY))){
    return NULL;
  }
  // Se lee el tamaño de los datos y se inicializa la pila con el tamaño leido
  if(read(file, &size, sizeof(int))<0){
    return NULL;
  }
  struct my_stack *stack;
  if(!(stack = my_stack_init(size))){
    return NULL;
  }
  // Se van leyendo los datos y haciendo un push hasta acabar el fichero
  void *data;
  if(!(data = malloc(size))){
    return NULL;
  }
  while (bytesLeidos = read(file, data, size)){
    if(bytesLeidos < 0){ // bytesLeidos == -1 si ha habido error
      return NULL;
    }
    my_stack_push(stack, data);
    if(!(data = malloc(size))){
      return NULL;
    }
  }
  // Se cierra el fichero
  if(close(file)){
    return NULL;
  }

  return stack;
};

int my_stack_write(struct my_stack *stack, char *filename){
  int file;
  /*Abrimos el fichero, en caso de que el fichero no se pueda abrir retorna -1,
  en caso contrari escribe los datos del fichero en la pila*/
  if (!(file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))){
    return -1;
  }
  
  /*En una pila auxiliar se vuelca la pila original para que al
  leerla este en orden*/
  struct my_stack *aux_stack;
  if(!(aux_stack = my_stack_init(stack->size))){
    return -1; // si error devuelve -1
  }
  struct my_stack_node *n = stack->top;
  for (int i = 0; i < my_stack_len(stack); i++){
    if(my_stack_push(aux_stack, n->data)){
      return -1; // si error devuelve -1
    }
    n = n->next;
  }
  // Primero se escribe el tamaño que tendran los datos
  if(!write(file, &stack->size, sizeof(int))){
    return -1;
  }
  // A continuación se recorre la pila
  int size = 0;
  while (aux_stack->top){
    // Se hace un pop de cada nodo y se escriben los datos en el fichero
    if(!write(file, my_stack_pop(aux_stack), aux_stack->size)){
      return -1;
    }
    size++;
  }
  // Se cierra el fichero
  if(close(file)){
    return -1;
  }
  return size;
};

