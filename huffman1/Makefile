CC = gcc
CFLAGS = -Wall -I./include
OBJS = src/main1.o src/encode1.o
TARGET = bin/huffman1

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

.PHONY: tmpclean clean

tmpclean:
	find . -name *~ -exec rm -vf {} \;
clean: tmpclean
	rm -f $(OBJS) $(TARGET)
