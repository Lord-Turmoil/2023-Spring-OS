.PHONY: clean-obj

out: fibo main
	gcc main.o fibo.o -o ../fibo

fibo: fibo.c
	gcc -I ../include/ -c fibo.c -o fibo.o

main: main.c
	gcc -I ../include/ -c main.c -o main.o

clean:
	rm -f *.o