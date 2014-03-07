#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "htypes.h"
#include "hptypes.h"
#include "hpack.h"

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
int hpt_header_table_add_new_field(HptHeaderTable *htable, struct ht_strtuple *hfield_ref){
  struct ht_dlist_entry *entry = NULL;
  struct ht_str *str = NULL;
  struct ht_strtuple *tuple;
  
  entry = ht_dlist_entry_new(HTL_ENTRY_TYPE_HDR, (void*)hfield_ref, (ht_dlist_value_free_func)ht_strtuple_destroy);
  if (entry == NULL){
    return -1;
  }
  ht_dlist_prepend_new_entry(htable->headers, entry);
  
  htable->current_size += HPACK_ENTRY_OVERHEAD+(hfield_ref->key->len)+(hfield_ref->value->len);
  
  // drop tail to keep the size
  while (htable->current_size > htable->maximum_size){
    entry = ht_dlist_pop_last(htable->headers);
    if (entry == NULL){
      return -1;
    }
    assert(entry->value_type == HTL_ENTRY_TYPE_HDR);
    htable->current_size -= HPACK_ENTRY_OVERHEAD;
    tuple = (struct ht_strtuple*)entry->value;
    str = (struct ht_str *)(tuple->key);
    htable->current_size -= str->len;
    str = (struct ht_str *)(tuple->value);
    htable->current_size -= str->len;
    ht_dlist_entry_destroy(entry);
  }

  return 0;
}


// returned ht_strtuple must be destroyed properly: IDX is 1-origined
struct ht_strtuple *hpt_header_table_lookup_by_index_ref(HptHeaderTable *htable, int idx){
  int i;
  struct ht_dlist_entry *entry;
  struct ht_strtuple *tuple;

  for (i = 0, entry = htable->headers->head; i < idx && entry->next != NULL; i++, entry = entry->next){
    ; // just skip
  }
  if (i == idx && entry->value_type >= 0){
    tuple = ht_strtuple_copy((struct ht_strtuple*) entry->value);
    return tuple; // may be NULL
  }
  return NULL;
}

int hpt_header_table_lookup_by_header_field(HptHeaderTable *htable, struct ht_strtuple *hfield){
  int idx;
  struct ht_dlist_entry *entry;
  struct ht_strtuple *tuple;

  for (idx = 0, entry = htable->headers->head; entry->next != NULL; idx++, entry = entry->next){
    tuple = (struct ht_strtuple*) entry->value;
    if (tuple != NULL && ht_str_cmp(tuple->key, hfield->key) == 0 && ht_str_cmp(tuple->value, hfield->value) == 0){
      return idx;
    }
  }
  return -1;
}

// does not free stable
void hpt_header_table_destroy(HptHeaderTable *htable){
  ht_dlist_destroy(htable->headers);
  memset(htable, 0, sizeof(HptHeaderTable));
  free(htable);
  return;
}

// just for debugging
void hpt_header_table_dump(HptHeaderTable *htable, FILE *outf){
  int i;
  struct ht_dlist_entry *entry;
  struct ht_strtuple *tuple;
  fprintf(outf, "Header_Table[%d/%d]:\n", htable->current_size, htable->maximum_size);
  for (i = 0, entry = htable->headers->head; entry->next != NULL; i++, entry = entry->next ){
    fprintf(outf, " %3d:", i);
    if (entry->value_type <= 0){
      fprintf(outf, " (not a value)\n");
    } else if (entry->value_type == HTL_ENTRY_TYPE_HDR){
      tuple = (struct ht_strtuple*) entry->value;
      fprintf(outf, " '%s'->'%s'\n", tuple->key->s, tuple->value->s);
    } else {
      fprintf(outf, " (unknown value)\n");
    }
  }
  return;
}

#ifdef HPTYPES_TEST

void test_hpt_header_table(void){
  HptHeaderTable *htable;
  struct ht_strtable *stable;
  struct ht_str *str_k;
  struct ht_str *str_v;
  struct ht_strtuple *tuple;

  assert((stable = ht_strtable_new()) != NULL);
  assert((htable = hpt_header_table_new(128, stable)) != NULL);
  // one entry takes 32+charlen, so it will drop 4th
  fprintf(stderr, "#0# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  assert((tuple = ht_strtuple_new_string(stable, "KEY1", 4, "VAL1", 4)) != NULL);
  hpt_header_table_add_new_field(htable, tuple);

  fprintf(stderr, "#1# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  tuple = hpt_header_table_lookup_by_index_ref(htable, 1);
  assert(strcmp(tuple->key->s, "KEY1") == 0);
  assert(strcmp(tuple->value->s, "VAL1") == 0);
  assert(hpt_header_table_lookup_by_header_field(htable, tuple) == 1);
  ht_strtuple_destroy(tuple);
  tuple = NULL;

  fprintf(stderr, "#2# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  tuple = ht_strtuple_new_string(stable, "KEY1", 4, "VAL2", 4);
  hpt_header_table_add_new_field(htable, tuple);
  tuple = ht_strtuple_new_string(stable, "KEY2", 4, "VAL3", 4);
  hpt_header_table_add_new_field(htable, tuple);

  fprintf(stderr, "#3# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  tuple = hpt_header_table_lookup_by_index_ref(htable, 3);
  assert(strcmp(tuple->key->s, "KEY1") == 0);
  assert(strcmp(tuple->value->s, "VAL1") == 0);
  assert(hpt_header_table_lookup_by_header_field(htable, tuple) == 3);
  ht_strtuple_destroy(tuple);
  tuple = NULL;

  fprintf(stderr, "#4# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  tuple = ht_strtuple_new_string(stable, "KEY2", 4, "VAL4", 4);
  hpt_header_table_add_new_field(htable, tuple);

  fprintf(stderr, "#5# ecount=%d\n", htable->headers->entry_count);
  hpt_header_table_dump(htable, stderr);

  tuple = hpt_header_table_lookup_by_index_ref(htable, 3);
  assert(strcmp(tuple->key->s, "KEY1") == 0);
  assert(strcmp(tuple->value->s, "VAL2") == 0);
  assert(hpt_header_table_lookup_by_header_field(htable, tuple) == 3);
  ht_strtuple_destroy(tuple);
  tuple = NULL;

  hpt_header_table_destroy(htable);
  ht_strtable_destroy(stable);
  return;
  
}
int main(int argc, char **argv){
  test_hpt_header_table();
}
#endif
