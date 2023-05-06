CFLAGS += -Wall -g -O5
pgm:	multiprocess-reader.o
	gcc -o $@ $^
.phony:	clean
clean: *.o
	rm pgm $^
