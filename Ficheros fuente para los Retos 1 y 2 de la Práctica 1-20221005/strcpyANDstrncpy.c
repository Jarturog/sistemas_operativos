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
  if(my_strlen(dest) < n){
    n = my_strlen(dest)+1;
  }
  while(src[i] && n > 0){
    dest[i] = src[i];
    i++;
    n--;
  }
  while(n > 0){
    dest[i++] = '\0';
    n--;
  }
  return dest;
}
