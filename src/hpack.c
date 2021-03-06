#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "hpack.h"

#ifndef FIXME
#define FIXME assert(0)
#endif

static const char *hpack_static_header_tables[] = {
  ":authority", NULL, // index=1
  ":method", NULL,
  ":method", "POST",
  ":path", "/",
  ":path", "/index.html",
  ":scheme", "http",
  ":scheme", "https",
  ":status", "200",
  ":status", "500",
  ":status", "404",
  ":status", "403",
  ":status", "400",
  ":status", "401",
  "accept-charset", NULL,
  "accept-encoding", NULL,
  "accept-language", NULL,
  "accept-ranges", NULL,
  "accept", NULL,
  "access-control-allow-origin", NULL,
  "age", NULL,
  "allow", NULL,
  "authorization", NULL,
  "cache-control", NULL,
  "content-disposition", NULL,
  "content-encoding", NULL,
  "content-language", NULL,
  "content-length", NULL,
  "content-location", NULL,
  "content-range", NULL,
  "content-type", NULL,
  "cookie", NULL,
  "date", NULL,
  "etag", NULL,
  "expect", NULL,
  "expires", NULL,
  "from", NULL,
  "host", NULL,
  "if-match", NULL,
  "if-modified-since", NULL,
  "if-none-match", NULL,
  "if-range", NULL,
  "if-unmodified-since", NULL,
  "last-modified", NULL,
  "link", NULL,
  "location", NULL,
  "max-forwards", NULL,
  "proxy-authenticate", NULL,
  "proxy-authorization", NULL,
  "range", NULL,
  "referer", NULL,
  "refresh", NULL,
  "retry-after", NULL,
  "server", NULL,
  "set-cookie", NULL,
  "strict-transport-security", NULL,
  "transfer-encoding", NULL,
  "user-agent", NULL,
  "vary", NULL,
  "via", NULL,
  "www-authenticate", NULL, // index=60
  NULL, NULL};

#define HPACK_STATIC_TABLE_LEN 60

#define HPACK_HEADER_TABLE_MININDEX(ctx) 1
#define HPACK_HEADER_TABLE_MAXINDEX(ctx) ((ctx)->ht->len)
#define HPACK_STATIC_TABLE_MININDEX(ctx) (HPACK_HEADER_TABLE_MAXINDEX(ctx)+1)
#define HPACK_STATIC_TABLE_MAXINDEX(ctx) (HPACK_HEADER_TABLE_MAXINDEX(ctx)+HPACK_STATIC_TABLE_LEN)


const char *hpack_static_table_key_of(struct hpack_context *ctx, int idx){
  int i = (idx - HPACK_STATIC_TABLE_MININDEX(ctx));
  return hpack_static_header_tables[(i)*2];
}
const char *hpack_static_table_value_of(struct hpack_context *ctx, int idx){
  int i = (idx - HPACK_STATIC_TABLE_MININDEX(ctx));
  return hpack_static_header_tables[((i)*2)+1];
}

struct hpack_context *hpack_context_new(size_t maximum_size, struct ht_strtable *stable_ref){
  struct hpack_context *ctx = NULL;
  struct ht_strtable *stable_new = NULL;
  if ((ctx = malloc(sizeof(struct hpack_context))) == NULL){
    goto fail;
  }
  if ((ctx->ht = hpt_header_table_new(maximum_size, stable_ref)) == NULL){
    goto fail;
  }
  
  return ctx;

 fail:
  if (ctx->ht) hpt_header_table_destroy(ctx->ht);
  if (ctx) free(ctx);
  return NULL;
}

void hpack_context_destroy(struct hpack_context *ctx){
  hpt_header_table_destroy(ctx->ht);
  ctx->ht = NULL;
  free(ctx);
  return;
}

int hpack_context_getsize(struct hpack_context *ctx){
  return ctx->ht->current_size;
}

int hpack_is_index_static(struct hpack_context *ctx, int idx){
  return (HPACK_STATIC_TABLE_MININDEX(ctx) <= idx && idx <= HPACK_STATIC_TABLE_MAXINDEX(ctx));
}

int hpack_is_index_htable(struct hpack_context *ctx, int idx){
  return (HPACK_HEADER_TABLE_MININDEX(ctx) <= idx && idx <= HPACK_HEADER_TABLE_MAXINDEX(ctx));
}

struct ht_str *hpack_lookup_key(struct hpack_context *ctx, int idx){
  if (hpack_is_index_static(ctx, idx)){
    return ht_str_new_statstr_strlen(hpack_static_table_key_of(ctx, idx));
  } else if (hpack_is_index_htable(ctx, idx)){
    // we only need a key here...
    struct ht_strtuple *tuple = NULL;
    struct ht_str *key = NULL;
    if ((tuple = hpt_header_table_lookup_by_index_ref(ctx->ht, idx)) == NULL){
      return NULL;
    }
    key = tuple->key;
    ht_str_ref(key);
    ht_strtuple_destroy(tuple);
    return key;
  }
  // else
  return NULL;
}

struct ht_strtuple *hpack_lookup_tuple(struct hpack_context *ctx, int idx){
  struct ht_strtuple *tuple = NULL;
  struct ht_str *key = NULL;
  struct ht_str *value = NULL;
  if (hpack_is_index_static(ctx, idx)){
    const char *cp;
    int cp_len;

    cp = hpack_static_table_key_of(ctx, idx);
    cp_len = strlen(cp);
    if ((key = ht_str_new_statstr(cp, cp_len)) == NULL){
      goto fail;
    }
    cp = hpack_static_table_value_of(ctx, idx);
    cp_len = strlen(cp);
    if ((value = ht_str_new_statstr(cp, cp_len)) == NULL){
      goto fail;
    }

    if ((tuple = ht_strtuple_new(ctx->ht->stable, key, value)) == NULL){
      goto fail;
    }
  } else if (hpack_is_index_htable(ctx, idx)){
    if ((tuple = hpt_header_table_lookup_by_index_ref(ctx->ht, idx)) == NULL){
      goto fail;
    }
  } else {
    return NULL;
  }
  return tuple;
 fail:
  if (tuple){
    ht_strtuple_destroy(tuple);
  } else {
    if (key) ht_str_unref(key);
    if (value) ht_str_unref(value);
  }
  return NULL;
}

// tuple_p_ret will point new tuple (should be destroyed)
// return consumed bytes or -1
int hpack_decode_tuple(struct hpack_context *ctx, u_int8_t *data, int data_len, struct ht_strtuple **tuple_p_ret){
  int flag_htupdate = 0;
  int64_t irep = -1; // indexed representation
  u_int8_t *rp = data;
  u_int8_t *end_p = data+data_len;
  int r;
  char *cp;
  u_int8_t b;
  struct ht_str *value = NULL;
  struct ht_str *key = NULL;
  struct ht_strtuple *tuple = NULL;
  struct ht_strtuple *tuple_reg = NULL;

  // 1000 0000 (content update)
  // 1[idx   ] (reference to key-value pair in header table)
  // 0100 0000 (literal without index, literal key, literal value)
  // 01[idx  ] (literal without index, referenced key, literal value)
  // 0000 0000 (literal with index, literal key, literal value)
  // 00[idx  ] (literal with index, referenced key, literal value)

  if ((*rp & 0x80) == 0x80){ // 1xxx xxxx
    // section 4.2: Indexed Header Field Representation
    irep = dec_integer(rp, end_p-rp, 1, &rp);

    // section 3.2
    if (irep == 0){
      // section 4.4
      b = *(rp++);
      if (b == 0x80){
	// ignore (we do not have reference set)
      } else if ((b & 0x80) == 0){
	// length change
	int64_t tbl_len = dec_integer(rp, end_p-rp, 1, &rp);
	if (tbl_len < 0) goto fail;
	if (hpt_header_table_size_update(ctx->ht, (size_t)tbl_len) < 0) goto fail;
      }
#if 0
    } else if (0){
      // if irep corresponding to an entry present in the reference set
      // the entry is removed from the reference set
      FIXME; // what's this? is this safe to ignore?
#endif
    } else {
      // tuple referenced by index
      if ((tuple = hpack_lookup_tuple(ctx, irep)) == NULL){
	goto fail;
      }
    }
  } else { // 0xxx xxxxx
    // section 4.3: Literal Header Field Representation
    if ((*rp & 0x40) == 0x00) { 
      // section 4.3.2
      flag_htupdate = 1;
    }

    irep = dec_integer(rp, end_p-rp, 2, &rp);
    if (irep == 0){
      // new name
      r = dec_string_nocopy(rp, end_p-rp, &cp, &rp);
      if ((key = ht_str_new_copystr(cp, r)) == NULL){
	goto fail;
      }
    } else {
      // Indexed Name
      key = hpack_lookup_key(ctx, irep);
    }

    // value
    r = dec_string_nocopy(rp, end_p-rp, &cp, &rp);
    if ((value = ht_str_new_copystr(cp, r)) == NULL){
      goto fail;
    }

    if ((tuple = ht_strtuple_new(ctx->ht->stable, key, value)) == NULL){
      goto fail;
    }
    // going to update the keyset?
    if (flag_htupdate){
      if ((tuple_reg = ht_strtuple_copy(tuple)) == NULL){
	goto fail;
      }
      if (hpt_header_table_add_new_field(ctx->ht, tuple_reg) < 0){
	goto fail;
      }
      tuple_reg = NULL; // it will be managed in header_table. forget it.
    }
  }

  *tuple_p_ret = tuple;
  return rp-data;
 fail:
  fprintf(stderr, "failure not implemented.\n");
  if (tuple) {
    ht_strtuple_destroy(tuple);
  } else {
    if (key) ht_str_unref(key);
    if (value) ht_str_unref(value);
  }
  if (tuple_reg) ht_strtuple_destroy(tuple_reg);
  *tuple_p_ret = NULL;
  return -1;
}


// tentative: only determined by number (for now)
int hpack_flag_update_strategy(struct hpack_context *ctx, struct ht_strtuple *tuple){
  if (tuple->value->len > ctx->ht->maximum_size/16){
    return 0;
  }
  return 1;
}

// return number of bytes written
// given tuple shall be destroyed on upper frame
int hpack_encode_tuple(struct hpack_context *ctx, u_int8_t *buf, int buf_len, struct ht_strtuple *tuple_copied){
  int idx_tuple_match = 0;
  int idx_key_match = 0;
  int i;
  char *v;
  u_int8_t *wp = buf;
  u_int8_t *ep = buf+buf_len;
  int flag_update = 0;
  struct ht_strtuple *tuple_new = NULL;
    
  // find the exact match
  i = hpt_header_table_lookup_by_header_field(ctx->ht, tuple_copied);
  if (i > 0){
    idx_tuple_match = i;
    goto resolved;
  } else if (i < 0){
    idx_key_match = -i;
  }

  for (i = HPACK_STATIC_TABLE_MININDEX(ctx); i <= HPACK_STATIC_TABLE_MAXINDEX(ctx); i++){
    if (strcmp(tuple_copied->key->s, hpack_static_table_key_of(ctx, i)) == 0){
      if (idx_key_match == 0){
	idx_key_match = i;
      }
      v = (char *)hpack_static_table_value_of(ctx, i);
      if (v != NULL && strcmp(tuple_copied->value->s, v) == 0){
	idx_tuple_match = i;
	goto resolved;
      }
    }
  }
  
  // if there's a key-match, save and reuse the result
 resolved:
  if (idx_tuple_match > 0){
    // section 4.2: Indexed Header Field Representation
    *wp = 0x80;
    if ((i = enc_integer(wp, ep-wp, idx_tuple_match, 1)) < 0) goto fail;
    wp += i;
  } else {
    flag_update = hpack_flag_update_strategy(ctx, tuple_copied);
    if (flag_update){
      *wp = 0;
    } else {
      *wp = 0x40; // 0100 0000
    }
    if (idx_key_match > 0){
      // indexed name
      if ((i = enc_integer(wp, ep-wp, idx_key_match, 1)) < 0) goto fail;
      wp += i;
    } else {
      // 0 and key name 
      wp ++; // already0
      if ((i = enc_string(wp, ep-wp, tuple_copied->key->s, tuple_copied->key->len)) < 0) goto fail;
      wp += i;
    }
    // value
    if ((i = enc_string(wp, ep-wp, tuple_copied->value->s, tuple_copied->value->len)) < 0) goto fail;
    wp += i;
  }

  if (flag_update){
    if ((tuple_new = ht_strtuple_copy(tuple_copied)) == NULL){
      goto fail;
    }
    if (hpt_header_table_add_new_field(ctx->ht, tuple_new) < 0){
      goto fail;
    }
  }

  return wp-buf;

 fail:
  if (tuple_new) ht_strtuple_destroy(tuple_new);
  return -1;
}



#ifdef HPACK_TEST

int main(int argc, char **argv){

  struct ht_strtable *stable = NULL;
  struct hpack_context *ctx = NULL;

  stable = ht_strtable_new();
  assert(stable);
  ctx = hpack_context_new(HPACK_TABLE_DEFAULT_MAX, stable);
  assert(ctx);

  fprintf(stderr, "no real test yet.\n");

  hpack_context_destroy(ctx);
  ht_strtable_destroy(stable);

  return 0;
}

#endif
