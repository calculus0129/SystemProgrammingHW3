CFLAGS += -Wall -g -O5
pgm:	multireader-submit.o #multiprocess-reader.o
	gcc -o $@ $^
.phony:	clean
clean: *.o
	rm pgm $^
