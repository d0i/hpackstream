#ifndef HTYPES_H
#define HTYPES_H


// double linked list entry
#define HT_DLIST_ENTRY_TYPE_UNSPEC 0
#define HT_DLIST_ENTRY_TYPE_DESTROYED -1
#define HT_DLIST_ENTRY_TYPE_SENTINEL -2

typedef void (*ht_dlist_value_free_func)(void *);
struct ht_dlist_entry {
  struct ht_dlist_entry *prev;
  struct ht_dlist_entry *next;
  int value_type; // don't use negative numbers;
  void *value;
  ht_dlist_value_free_func free_func; // called on destructor
};

struct ht_dlist_entry *ht_dlist_entry_new(int value_type, void *value, ht_dlist_value_free_func free_func);
void ht_dlist_entry_append(struct ht_dlist_entry *prev, struct ht_dlist_entry *newent);
void ht_dlist_entry_destroy(struct ht_dlist_entry *ent);
struct ht_dlist_entry *ht_dlist_entry_detach(struct ht_dlist_entry *ent);
void ht_dlist_entry_remove(struct ht_dlist_entry *ent);

// double linked list
struct ht_dlist {
  struct ht_dlist_entry *head;
  struct ht_dlist_entry *tail;
  size_t entry_count;
};

void ht_dlist_destroy(struct ht_dlist *list_p);
struct ht_dlist *ht_dlist_new(void);
void ht_dlist_append_new_entry(struct ht_dlist *list_p, struct ht_dlist_entry *newent);
void ht_dlist_prepend_new_entry(struct ht_dlist *list_p, struct ht_dlist_entry *newent);
struct ht_dlist_entry *ht_dlist_pop_first(struct ht_dlist *list_p);
struct ht_dlist_entry *ht_dlist_pop_last(struct ht_dlist *list_p);


// naive implementation using double linked list as tables
struct ht_strtable {
  struct ht_dlist *keys_table_p;
  struct ht_dlist *values_table_p;
};

// entry of string instance
struct ht_str {
  int refcnt;
  char *s; // string instance
};


struct ht_strtuple {
  struct ht_strtable *table_p;
  struct ht_str *key_p;
  struct ht_str *value_p;
};




#endif
