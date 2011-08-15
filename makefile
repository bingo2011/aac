include ../makefile.macros

#	Directories
LIBFAAC_DIR	=	src/libfaac/src

CFLAGS += -Iinc -Isrc/libfaac/include

C_SOURCES = src/aacencoder_wrapper.c	\
			$(LIBFAAC_DIR)/aacquant.c          \
            $(LIBFAAC_DIR)/backpred.c       \
            $(LIBFAAC_DIR)/bitstream.c       \
            $(LIBFAAC_DIR)/channels.c       \
            $(LIBFAAC_DIR)/fft.c       \
            $(LIBFAAC_DIR)/filtbank.c       \
            $(LIBFAAC_DIR)/frame.c       \
            $(LIBFAAC_DIR)/huffman.c       \
            $(LIBFAAC_DIR)/ltp.c       \
            $(LIBFAAC_DIR)/midside.c       \
            $(LIBFAAC_DIR)/psychkni.c       \
            $(LIBFAAC_DIR)/tns.c       \
	        $(LIBFAAC_DIR)/util.c

OBJS:=$(subst .c,.o,$(C_SOURCES))
LIB:=../aacCodec.a

all: $(LIB)

$(LIB): $(OBJS) 
	ar r $@ $?

include $(subst .c,.d,$(C_SOURCES))

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<

%.d: %.c
	$(CCDEP) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	

clean:
	rm -f $(OBJS); rm -f $(LIB); rm -rf $(SRC_DIR)/*.d; rm -rf $(API_SRC_DIR)/*.d

