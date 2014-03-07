SRCS=htypes.c hptypes.c
OBJS=htypes.o hptypes.o

CC=gcc
CFLAGS=-fPIC -g

tests: test_htypes test_hptypes

libhpstream.so.0: $(OBJS)
	$(CC) $(CFLAGS) -shared -o libhpstream.so.0 $(OBJS) 

test_htypes: htypes.c
	$(CC) $(CFLAGS) -DHTYPES_TEST -o test_htypes htypes.c
test_hptypes: hptypes.c htypes.o
	$(CC) $(CFLAGS) -DHPTYPES_TEST -o test_hptypes hptypes.c htypes.o

clean:
	rm -f *.o *~ test_htypes


