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
  while(src[i] && n > 0){
    dest[i] = src[i];
    i++;
    n--;
  }
  while(src[i]){
    dest[i] = '\0';
    i++;
  }
  dest[i] = '\0';
  return dest;
}
