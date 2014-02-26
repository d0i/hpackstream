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
void ht_str_destroy(struct ht_str *hstr_p){
  if (hstr_p->refcnt > 0){
    free(hstr_p->s);
    hstr_p->s = NULL;
  }
  free(hstr_p);
  return;
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
  int free_string = 0;
  assert(hstr_p->refcnt != 0);
  if (hstr_p->refcnt > 0){
    hstr_p->refcnt --;
    free_string = 1;
  } else {
    hstr_p->refcnt ++;
  }
  if (hstr_p->refcnt == 0){
    if (free_string){
      free(hstr_p->s);
      hstr_p->s = NULL;
    }
    ht_str_destroy(hstr_p);
    return 0;
  }
  return abs(hstr_p->refcnt);
}

struct ht_strtable *ht_strtable_new(){
  struct ht_strtable *stable;
  if ((stable = malloc(sizeof(struct ht_strtable))) == NULL){
    return NULL;
  }
  if ((stable->values_list_p = ht_dlist_new()) == NULL){
    free(stable);
    return NULL;
  }
  return stable;
}
struct ht_str* ht_strtable_lookup_index_ref(struct ht_strtable *stable, int index){
  int i = 0;
  struct ht_dlist_entry *entry_p;
  struct ht_str *str_p;

  for (i = 1, entry_p = stable->values_list_p->head->next; 
       entry_p->next != NULL && entry_p->value_type >= 0 && i <= index;
       entry_p = entry_p->next){
    if (i == index){
      str_p = (struct ht_str *)entry_p->value;
      ht_str_ref(str_p);
      return str_p;
    }
  }
  return NULL;
}
struct ht_str* ht_strtable_lookup_str_ref(struct ht_strtable *stable, char *s, size_t slen){
  int i = 0;
  struct ht_dlist_entry *entry_p;
  struct ht_str *str_p;

  for (i = 1, entry_p = stable->values_list_p->head->next; 
       entry_p->next != NULL && entry_p->value_type >= 0;
       entry_p = entry_p->next){
    str_p = (struct ht_str *)entry_p->value;
    if ((slen == str_p->len) && (strcmp(s, str_p->s) == 0)){
      ht_str_ref(str_p);
      return str_p;
    }
  }
  return NULL;
}
struct ht_str* ht_strtable_add_new_copystr_ref(struct ht_strtable *stable, char *copystr, size_t slen){
  struct ht_str *str_p = NULL;
  struct ht_dlist_entry *entry_p = NULL;
  if ((str_p = ht_strtable_lookup_str_ref(stable, copystr, slen)) == NULL){
    // make a new entry and prepend
    if ((str_p = ht_str_new_copystr(copystr, slen)) == NULL){
      return NULL;
    }
    if ((entry_p = ht_dlist_entry_new(0, (void*)str_p, (ht_dlist_value_free_func)ht_str_unref)) == NULL){
      ht_str_destroy(str_p);
      return NULL;
    }
    ht_dlist_prepend_new_entry(stable->values_list_p, entry_p);
    ht_str_ref(str_p);
  }
  return str_p;
}

void ht_strtable_destroy(struct ht_strtable *stable){
  ht_dlist_destroy(stable->values_list_p);
  free(stable);
  return;
}




#ifdef HTYPES_TEST

void test_dlist(void){
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
void test_dstr(void){
  struct ht_str *str1_p;
  struct ht_str *str2_p;
  struct ht_str *str3_p;
  int i;

  str1_p = ht_str_new_copystr("hoi", 3);
  str2_p = ht_str_new_copystr("hoge", 4);
  str3_p = ht_str_new_statstr("foobar", 6);

  for (i = 0; i < 10; i++){
    ht_str_ref(str1_p);
    ht_str_ref(str2_p);
    ht_str_ref(str3_p);
  }
  assert(strcmp(str1_p->s, "hoi") == 0);
  assert(strcmp(str2_p->s, "hoge") == 0);
  assert(strcmp(str3_p->s, "foobar") == 0);
  
  ht_str_destroy(str1_p);
  for (i = 0; i < 11; i++){
    ht_str_unref(str2_p);
    ht_str_unref(str3_p);
  }
}
void test_strtable(void){
  struct ht_strtable *stable = NULL;
  struct ht_str *hstr;
  int i;

  stable = ht_strtable_new();
  assert(stable != NULL);

  for (i = 0; i < 1000; i++){
    hstr = ht_strtable_add_new_copystr_ref(stable, "hoge1", 5);
    assert(hstr != NULL);
    ht_str_unref(hstr);
  }
  hstr = ht_strtable_add_new_copystr_ref(stable, "hoge2", 5);
  assert(hstr != NULL);
  ht_str_unref(hstr);

  hstr = ht_strtable_lookup_index_ref(stable, 1);
  assert(strcmp(hstr->s, "hoge2") == 0);
  ht_str_unref(hstr);
  hstr = ht_strtable_lookup_str_ref(stable, "hoge1", 5);
  assert(strcmp(hstr->s, "hoge1") == 0);
  ht_str_unref(hstr);

  ht_strtable_destroy(stable);
  return;
}


// test it with valgrind to make sure no memory is leaked.
int main(int argc, char **argv){
  test_dlist();
  test_dstr();
  test_strtable();
  

  return;
}

#endif
