CC=gcc # compilador
CFLAGS=-c -g -Wall -std=c17 #flags para el compilador
LDFLAGS= #flags para enlazador

SOURCES= nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c my_shell.c
LIBRARIES= 
INCLUDES= colores.h
PROGRAMS= nivel1 nivel2 nivel3 nivel4 nivel5 nivel6 my_shell

OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

nivel1: nivel1.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

nivel2: nivel2.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

nivel3: nivel3.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

nivel4: nivel4.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

nivel5: nivel5.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

nivel6: nivel6.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

my_shell: my_shell.o $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

%.o: %.c $(INCLUDES) 
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o $(PROGRAMS)
