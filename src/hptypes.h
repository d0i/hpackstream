#ifndef INCLUDE_HPTYPES
#define INCLUDE HPTYPES
#include "htypes.h"

// arbitrary number
#define HTL_ENTRY_TYPE_HDR 1247

// based on header-compression-06
typedef struct hpt_header_table {
  size_t current_size; // 3.3.1 Maximum Table Size
  size_t maximum_size;
  struct ht_dlist *headers; // active headers
  struct ht_strtable *stable; // just reference
} HptHeaderTable;

// stable is reference only (will not destroyed on table_desroy)
HptHeaderTable *hpt_header_table_new(size_t maximum_size, struct ht_strtable *stable_ref);
// 0 to success -1 to error, hfield_ref will be held in the table so don't destroy it
int hpt_header_table_add_new_field(HptHeaderTable *htable, struct ht_strtuple *hfield_ref);
// must be destroied reterned value is ref'ed. must be destroied properly
struct ht_strtuple *hpt_header_table_lookup_by_index_ref(HptHeaderTable *htable, int idx);
// returns index
int hpt_header_table_lookup_by_header_field(HptHeaderTable *htable, struct ht_strtuple *hfield);
void hpt_header_table_destroy(HptHeaderTable *htable);

#endif

