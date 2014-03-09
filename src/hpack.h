#ifndef HPACK_H
#define HPACK_H

#include "hptypes.h"

// hpack constants

#define HPACK_ENTRY_OVERHEAD 32
#define HPACK_TABLE_DEFAULT_MAX 4096

struct hpack_context {
  struct hpt_header_table *ht;
};


#endif
