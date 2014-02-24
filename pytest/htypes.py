from ctypes import *

hpstream = cdll.LoadLibrary("../src/libhpstream.so.0")

dlist = hpstream.ht_dlist_new()
print "## no real test yet implemented. ht_dlist_new returned", dlist
hpstream.ht_dlist_destroy(dlist)
print "## done."


