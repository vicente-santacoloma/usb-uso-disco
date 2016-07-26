CC=gcc
CFLAGS=-I.
DEPS = Lista.h
OBJ = UsoDisco.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

UsoDisco: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o UsoDisco
	rm -rf *.o Lista