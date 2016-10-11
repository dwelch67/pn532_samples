

all : simple complex lultra usimple

simple : simple.c ser.c ser.h 
	gcc -Wall -O2 simple.c ser.c -o simple

usimple : usimple.c ser.c ser.h 
	gcc -Wall -O2 usimple.c ser.c -o usimple

complex : complex.c ser.c ser.h 
	gcc -Wall -O2 complex.c ser.c -o complex

lultra : lultra.c ser.c ser.h 
	gcc -Wall -O2 lultra.c ser.c -o lultra


clean :
	rm -f simple
	rm -f usimple
	rm -f complex
	rm -f lultra


