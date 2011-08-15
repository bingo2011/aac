include ../../../makefile.macros.base

BASE_DIR=/host/POLYCOM/PORTING/rss4000v7.5-H264Streaming/mcu/ampSoft
INC=../inc/
LIB=../../aacCodec.a ../../audioFilter.a

all:
#	$(CC) -g -o aacEncTester -I. -I$(INC) main.cpp input.c $(LIB) -lm
	g++ -g -o aacEncTester -I. -I$(INC) main.cpp input.c ../src/aacencoder_wrapper.c $(LIB) -lm

clean:
	rm -f *.o; rm -f aacEncTester 


