# makefile for wl6

(CC) = cc -Wall

wl6: wlfiler6.o wlmain.o wl.h
	$(CC) wlfiler6.o wlmain.o -o wl6

wlfiler6.o: wlfiler6.c
	$(CC) -c wlfiler6.c

wlmain.o: wlmain.c
	$(CC) -c wlmain.c

clean: 
	rm *.o
