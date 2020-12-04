all:main.c child.c
	gcc -pthread -O2 -o main main.c -lrt
	gcc -pthread -O2 -o child child.c -lrt