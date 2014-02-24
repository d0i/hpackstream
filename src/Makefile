SRCS=htypes.c
OBJS=htypes.o

CC=gcc
CFLAGS=-fPIC -g

libhpstream.so.0: $(OBJS)
	$(CC) $(CFLAGS) -shared -o libhpstream.so.0 $(OBJS) 

test_htypes: htypes.c
	$(CC) $(CFLAGS) -DHTYPES_TEST -o test_htypes htypes.c
clean:
	rm -f *.o *~ test_htypes


