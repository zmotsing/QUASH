quash:quash.o
	g++ quash.o -o quash

quash.o:quash.c
	g++ -c -g quash.c

clean:
	rm quash *.o
