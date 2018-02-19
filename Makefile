all: program

program: program.o
	gcc -D_REENTRANT -o program program.o -lpthread -lm

program.o: program.c
	gcc -c program.c

clean:
	rm -f *.o program 
