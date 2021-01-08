CC = gcc
test: main.o foo.o
	$(CC) -o $@ $^

.PHONY: clean tmpclean
clean: tmpclean
	rm -f *.o test
tmpclean:
	rm -f *~
