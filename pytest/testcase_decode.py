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
        
        dec_headers = [] # list of strtuple pointer
        while read_len < data0_len:
            (data, data_len) = mkData(case['wire'], read_len)
            (strtuple, rlen) = parse_once(ctx, data, data_len)
            read_len += rlen
            if strtuple:
                dec_headers.append(strtuple)

        case_headers = case['headers']
        assert(len(case_headers) == len(dec_headers))
        for i in range(len(case_headers)):
            k = c_char_p(hps.ht_strtuple_getkey(dec_headers[i]))
            v = c_char_p(hps.ht_strtuple_getvalue(dec_headers[i]))
            try:
                assert(len(case_headers[i].keys()) == 1)
                assert(k.value == case_headers[i].keys()[0])
                assert(v.value == case_headers[i][k.value])
            except AssertionError:
                print 'k"dec","case": "%s","%s"'%(k.value, case_headers[i].keys()[0])
                if k.value not in case_headers[i]:
                    print 'case keys: %r'%(case_headers[i])
                else:
                    print 'v"dec","case": "%s","%s"'%(v.value, case_headers[i][k.value])
                raise

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
    print 'ok.'

