CC = gcc
CFLAGS = -Wall -I./include
OBJS = src/main0.o src/encode0.o
TARGET = bin/huffman0

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

.PHONY: tmpclean clean

tmpclean:
	find . -name *~ -exec rm -vf {} \;
clean: tmpclean
	rm -f $(OBJS) $(TARGET)
