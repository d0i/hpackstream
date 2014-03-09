SRCS=htypes.c hptypes.c encoding.c hpack.c
OBJS=htypes.o hptypes.o encoding.o hpack.o
TESTS=test_htypes test_hptypes test_encoding test_hpack

CC=gcc
CFLAGS=-fPIC -g

default: libhpstream.so.0

tests: $(TESTS) 

libhpstream.so.0: $(OBJS)
	$(CC) $(CFLAGS) -shared -o libhpstream.so.0 $(OBJS) 

test_htypes: htypes.c
	$(CC) $(CFLAGS) -DHTYPES_TEST -o test_htypes htypes.c
test_hptypes: hptypes.c htypes.o
	$(CC) $(CFLAGS) -DHPTYPES_TEST -o test_hptypes hptypes.c htypes.o
test_encoding: encoding.c
	$(CC) $(CFLAGS) -DENCODING_TEST -o test_encoding encoding.c
test_hpack: hpack.c htypes.o hptypes.o encoding.o
	$(CC) $(CLAGS) -DHPACK_TEST -o test_hpack hpack.c htypes.o hptypes.o encoding.o


clean:
	rm -f *.o *~ $(TESTS) libhpstream.so.0


