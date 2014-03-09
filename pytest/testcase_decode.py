from htypes import *

import json
import sys
import pdb


def run_case(test_case):
    initial_table_size = int(test_case['cases'][0]['header_table_size'])
    for case in test_case['cases']:
        if int(case['header_table_size']) != initial_table_size:
            raise SystemExit, "altering table size on the fly is not yet supported"

    stable = hps.ht_strtable_new()
    ctx = hps.hpack_context_new(4096, stable)

    for case in test_case['cases']:
        (data0, data0_len) = mkData(case['wire'])
        read_len = 0
        
        headers = [] # list of strtuple pointer
        while read_len < data0_len:
            (data, data_len) = mkData(case['wire'], read_len)
            (strtuple, rlen) = parse_once(ctx, data, data_len)
            read_len += rlen
            headers.append(strtuple)
        
        print 'no check yet.'
        for header in headers:
            k = c_char_p(hps.ht_strtuple_getkey(header))
            v = c_char_p(hps.ht_strtuple_getvalue(header))
            print 'k:', k.value
            print 'v:', v.value

    hps.hpack_context_destroy(ctx)
    hps.ht_strtable_destroy(stable)
    return

if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit, "%s [testcase_encoded]"%(sys.argv[0])
    # else
    f = open(sys.argv[1], 'r')
    test_case = json.load(f)
    f.close()

    run_case(test_case)
