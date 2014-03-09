#ifndef HPACK_H
#define HPACK_H

#include "hptypes.h"

// hpack constants

#define HPACK_ENTRY_OVERHEAD 32
#define HPACK_TABLE_DEFAULT_MAX 4096

struct hpack_context {
  struct hpt_header_table *ht;
};

struct hpack_context *hpack_context_new(size_t maximum_size, struct ht_strtable *stable_ref);
void hpack_context_destroy(struct hpack_context *ctx);
int hpack_context_getsize(struct hpack_context *ctx);
int hpack_decode_tuple(struct hpack_context *ctx, u_int8_t *data, int data_len, struct ht_strtuple **tuple_p_ret);





#endif
