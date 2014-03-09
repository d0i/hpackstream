import pdb
from ctypes import *


hps = cdll.LoadLibrary("../src/libhpstream.so.0")


def parse_once(ctx, data, data_len):
    """
    returns (strtuple, read_len)

    ctx is hpack_context
    data is c_char pointer (maybe mutable array)
    data_len is length left

    strtuple is in void pointer
    read_len is int
    """
    tuple_p = c_void_p()
    r=hps.hpack_decode_tuple(ctx, data, data_len, pointer(tuple_p))

    return (tuple_p, r)

def mkData(data_src, offset=0):
    "data_src is hex-str -> (data, data_len)"
    data_len=(len(data_src)/2)-offset
    data=create_string_buffer(data_len)
    for i in range(offset, len(data_src)/2):
        data[i-offset] = chr(int(data_src[i*2:i*2+2], 16));
    return (data, data_len)


def sample_once(data_src, expect_k, expect_v):
    stable = hps.ht_strtable_new()
    ctx = hps.hpack_context_new(4096, stable)

    (data, data_len) = mkData(data_src)

    (tuple_p, rlen) = parse_once(ctx, data, data_len)
    print 'read %d bytes.'%(rlen)
    k=c_char_p(hps.ht_strtuple_getkey(tuple_p))
    v=c_char_p(hps.ht_strtuple_getvalue(tuple_p))
    print 'k:', k.value
    print 'v:', v.value
    assert(k.value == expect_k)
    assert(v.value == expect_v)
    assert(rlen == data_len)

    hps.hpack_context_destroy(ctx)
    hps.ht_strtable_destroy(stable)

def D_1_1():
    data_src='000a637573746f6d2d6b65790d637573746f6d2d686561646572'
    sample_once(data_src, 'custom-key', 'custom-header')
    return

def D_1_2():
    data_src = '440c2f73616d706c652f70617468'
    sample_once(data_src, ':path', '/sample/path')
    return

def D_1_3():
    data_src = '82'
    sample_once(data_src, ':method', 'GET')
    return

if __name__ == '__main__':
    D_1_1()
    D_1_2()

