#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "htypes.h"
#include "hptypes.h"

// stable is reference only (will not destroyed on table_desroy)
HptHeaderTable *hpt_header_table_new(size_t maximum_size, struct ht_strtable *stable_ref){
  HptHeaderTable *htable = NULL;
  if ((htable = malloc(sizeof(HptHeaderTable))) == NULL){
    return NULL;
  }
  memset(htable, 0, sizeof(HptHeaderTable));
  htable->maximum_size = maximum_size;
  htable->headers = ht_dlist_new();
  htable->stable = stable_ref;
  if (htable->headers == NULL || htable->stable == NULL){
    goto fail;
  }
  return htable;
 fail:
  if (htable->headers) ht_dlist_destroy(htable->headers);
  free(htable);
  return NULL;
}

// 0 to success -1 to error, hfield_ref will be held in the table don't destroy
int hpt_header_table_add_new_field(HptHeaderTable *htable, HptHeaderField *hfield_ref){
  struct ht_dlist_entry *entry = NULL;
  struct ht_str *str = NULL;
  
  entry = ht_dlist_entry_new(452, (void*)hfield_ref, (ht_dlist_value_free_func)ht_strtuple_destroy);
  if (entry == NULL){
    return -1;
  }
  ht_dlist_prepend_new_entry(htable->headers, entry);
  
  htable->current_size += 32+(hfield_ref->key->len)+(hfield_ref->value->len);
  
  // drop tail to keep the size
  while (htable->current_size > htable->maximum_size){
    entry = ht_dlist_pop_last(htable->headers);
    if (entry == NULL){
      return -1;
    }
    assert(entry->value_type == 452);
    str = (struct ht_str *)(entry->value);
    htable->current_size -= str->len;
    ht_dlist_entry_destroy(entry);
  }

  return 0;
}


// returned ht_strtuple must be destroyed properly
HptHeaderField *hpt_header_table_lookup_by_index_ref(HptHeaderTable *htable, int idx){
  assert(0);
}

int hpt_header_table_lookup_by_header_field(HptHeaderTable *htable, HptHeaderField *hfield){
  assert(0);
}
void hpt_header_table_destroy(HptHeaderTable *htable){
  ht_dlist_destroy(htable->headers);
  memset(htable, 0, sizeof(HptHeaderTable));
  free(htable);
  return;
}

#ifdef HPTYPES_TEST

int main(int argc, char **argv){
  assert(0);
}
#endif
