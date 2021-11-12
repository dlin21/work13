all: main.o
	gcc -o structrw main.o

main.o: main.c
	gcc -c main.c

run:
	./structrw

clean:
	rm *.o
	rm structrw
	rm *.data
