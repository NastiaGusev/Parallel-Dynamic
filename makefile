build:
	mpicc -c main.c -lm
	mpicc -o exec main.o -lm

clean:
	rm *.o exec

run:
	mpiexec -np 21 exec 
