CC = gcc
CFLAGS = -Wall -I./include # -Iinclude でも問題ない。若干好みの問題。 

TARGET = bin/traverse
OBJS = src/main.o src/traverse.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^


.PHONY: clean tmpclean
clean: tmpclean
	rm -f $(OBJS) $(TARGET) 
tmpclean:
	find . -name *~ -exec rm -f {} \;
