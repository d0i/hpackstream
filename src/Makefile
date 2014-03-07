SRCS=htypes.c hptypes.c encoding.c
OBJS=htypes.o hptypes.o encoding.o

CC=gcc
CFLAGS=-fPIC -g

default: libhpstream.so.0

tests: test_htypes test_hptypes test_encoding

libhpstream.so.0: $(OBJS)
	$(CC) $(CFLAGS) -shared -o libhpstream.so.0 $(OBJS) 

test_htypes: htypes.c
	$(CC) $(CFLAGS) -DHTYPES_TEST -o test_htypes htypes.c
test_hptypes: hptypes.c htypes.o
	$(CC) $(CFLAGS) -DHPTYPES_TEST -o test_hptypes hptypes.c htypes.o

test_encoding: encoding.c
	$(CC) $(CFLAGS) -DENCODING_TEST -o test_encoding encoding.c


clean:
	rm -f *.o *~ test_htypes


