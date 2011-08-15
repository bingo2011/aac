include ../../../../makefile.macros.base

INC=../../inc/
LIB=../../../aacCodec.a

all:
	$(CC) -o faac -I$(INC) main.c input.c $(LIB) -lm

clean:
	rm -f *.o; rm -f faac


