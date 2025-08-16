.PHONY: clean

CFLAGS=-Wall -g -DSENTINEL

driver: driver.o rbtree.o

rbtree.o: rbtree.c rbtree.h
	$(CC) $(CFLAGS) -c rbtree.c -o rbtree.o

clean:
	rm -f driver *.o
