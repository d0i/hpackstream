#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "htypes.h"


struct ht_dlist_entry *ht_dlist_entry_new(int value_type, void *value, ht_dlist_value_free_func free_func){
  struct ht_dlist_entry *entry_p = malloc(sizeof(struct ht_dlist_entry));
  if (! entry_p) return NULL;
  memset(entry_p, 0, sizeof(struct ht_dlist_entry));
  entry_p->value_type = value_type;
  entry_p->value = value;
  entry_p->free_func = free_func;
  return entry_p;
}
void ht_dlist_entry_append(struct ht_dlist_entry *prev, struct ht_dlist_entry *newent){
  struct ht_dlist_entry *next = prev->next;
  assert (newent != NULL);
  prev->next = newent;
  newent->next = next;
  if (next) next->prev = newent;
  newent->prev = prev;
}
void ht_dlist_entry_destroy(struct ht_dlist_entry *ent){
  if (ent->free_func) ent->free_func(ent->value);
  ent->value = ent->next = ent->prev = NULL;
  ent->value_type = HT_DLIST_ENTRY_TYPE_DESTROYED;
  free(ent);
}
struct ht_dlist_entry *ht_dlist_entry_detach(struct ht_dlist_entry *ent){
  ent->next->prev = ent->prev;
  ent->prev->next = ent->next;
  ent->next = ent->prev = NULL;
  return ent;
}
void ht_dlist_entry_remove(struct ht_dlist_entry *ent){
  ht_dlist_entry_detach(ent);
  ht_dlist_entry_destroy(ent);
}
void ht_dlist_destroy(struct ht_dlist *list_p){
  while (list_p->head->next->value_type != HT_DLIST_ENTRY_TYPE_SENTINEL){
    ht_dlist_entry_remove(list_p->head->next);
  }
  ht_dlist_entry_destroy(list_p->head);
  ht_dlist_entry_destroy(list_p->tail);
  list_p->head = NULL;
  list_p->tail = NULL;
  free(list_p);
}
struct ht_dlist *ht_dlist_new(void){
  struct ht_dlist *list_p = NULL;
  if (! (list_p = malloc(sizeof(struct ht_dlist)))) return NULL;
  memset(list_p, 0, sizeof(struct ht_dlist));
  list_p->head = ht_dlist_entry_new(HT_DLIST_ENTRY_TYPE_SENTINEL, NULL, NULL);
  list_p->tail = ht_dlist_entry_new(HT_DLIST_ENTRY_TYPE_SENTINEL, NULL, NULL);
  if (list_p->head == NULL || list_p->tail == NULL){
    ht_dlist_destroy(list_p);
    return NULL;
  }
  list_p->head->next = list_p->tail;
  list_p->tail->prev = list_p->head;
  return list_p;
}
void ht_dlist_append_new_entry(struct ht_dlist *list_p, struct ht_dlist_entry *newent){
  ht_dlist_entry_append(list_p->tail->prev, newent);
  list_p->entry_count ++;
}
void ht_dlist_prepend_new_entry(struct ht_dlist *list_p, struct ht_dlist_entry *newent){
  ht_dlist_entry_append(list_p->head, newent);
  list_p->entry_count ++;
}
struct ht_dlist_entry *ht_dlist_pop_first(struct ht_dlist *list_p){
  struct ht_dlist_entry *ent;
  if (list_p->head->next->value_type < 0) return NULL;
  ent = ht_dlist_entry_detach(list_p->head->next);
  list_p->entry_count --;
  return ent;
}
struct ht_dlist_entry *ht_dlist_pop_last(struct ht_dlist *list_p){
  struct ht_dlist_entry *ent;
  if (list_p->tail->prev->value_type < 0) return NULL;
  ent = ht_dlist_entry_detach(list_p->tail->prev);
  list_p->entry_count --;
  return ent;
}


struct ht_str *ht_str_new_copystr(char *copystr, size_t slen){
  struct ht_str *hstr_p = NULL;
  if (! (hstr_p = malloc(sizeof(struct ht_str)))) return NULL;
  if (! (hstr_p->s = malloc(slen+1))){
    free(hstr_p);
    return NULL;
  }
  memset(hstr_p->s, 0, slen+1);
  strncpy(hstr_p->s, copystr, slen+1);
  hstr_p->len = slen;
  hstr_p->refcnt = 1;
  return hstr_p;
}
struct ht_str *ht_str_new_statstr(char *static_str, size_t slen){
  struct ht_str *hstr_p = NULL;
  if (! (hstr_p = malloc(sizeof(struct ht_str)))) return NULL;
  hstr_p->s = static_str;
  hstr_p->len = slen;
  hstr_p->refcnt = -1;
  return hstr_p;
}
int ht_str_ref(struct ht_str *hstr_p){
  assert(hstr_p->refcnt != 0);
  if (hstr_p->refcnt > 0){
    hstr_p->refcnt++;
  } else {
    hstr_p->refcnt--;
  }
  return abs(hstr_p->refcnt);
}
int ht_str_unref(struct ht_str *hstr_p){
  assert(hstr_p->refcnt != 0);
  if (hstr_p->refcnt > 0){
    hstr_p->refcnt --;
    if (hstr_p->refcnt == 0){
      free(hstr_p->s);
      hstr_p->s = NULL;
    }
  } else {
    hstr_p->refcnt ++;
  }
  return abs(hstr_p->refcnt);
}

#ifdef HTYPES_TEST
int main(int argc, char **argv){
  struct ht_dlist *list_p;
  struct ht_dlist_entry *ent_p;
  
  list_p = ht_dlist_new();
  ht_dlist_append_new_entry(list_p, ht_dlist_entry_new(0, "hoge0", NULL));
  ht_dlist_append_new_entry(list_p, ht_dlist_entry_new(0, "hoge1", NULL));
  ht_dlist_append_new_entry(list_p, ht_dlist_entry_new(0, "hoge2", NULL));
  ent_p = ht_dlist_pop_first(list_p);
  assert(strcmp(ent_p->value, "hoge0") == 0);
  ht_dlist_entry_destroy(ent_p);
  ent_p = ht_dlist_pop_last(list_p);
  assert(strcmp(ent_p->value, "hoge2") == 0);
  ht_dlist_entry_destroy(ent_p);
  assert(list_p->entry_count == 1);
  ht_dlist_destroy(list_p);

  return;
}

#endif
