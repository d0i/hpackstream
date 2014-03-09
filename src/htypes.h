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


// entry of string instance
struct ht_str {
  int refcnt; // negative number means s shall not be free'd
  char *s; // string instance
  size_t len;
};

struct ht_str *ht_str_new_copystr(char *copystr, size_t slen);
struct ht_str *ht_str_new_statstr(const char *static_str, size_t slen);
struct ht_str *ht_str_new_statstr_strlen(const char *static_char);
int ht_str_ref(struct ht_str *hstr_p);
int ht_str_unref(struct ht_str *hstr_p);
void ht_str_destroy(struct ht_str *hstr_p);
#define ht_str_len(hstr_p) ((hstr_p)->len)
#define ht_str_cmp(h1, h2) (strcmp((h1)->s, (h2)->s))

// naive implementation using double linked list as tables
// should it be some btree for better search? FIXME
struct ht_strtable {
  struct ht_dlist *values_list_p;
};

// index is 1-origin (corresponds to HPACK spec)
struct ht_str *ht_strtable_lookup_index_ref(struct ht_strtable *stable, int index); // SHALL be unref'ed
struct ht_str *ht_strtable_lookup_str_ref(struct ht_strtable *stable, char *s, size_t slen); // SHALL be unref'ed
struct ht_str *ht_strtable_add_new_copystr_ref(struct ht_strtable *stable, char *copystr, size_t slen); // SHALL be unref'ed
//deferred struct ht_str* ht_strtable_add_new_statstr_ref(struct ht_strtable *stable, char *static_str, size_t slen); // SHALL be unref'ed
struct ht_strtable *ht_strtable_new();
void ht_strtable_destroy(struct ht_strtable *stable);

// key-value tuple
struct ht_strtuple {
  struct ht_strtable *stable;
  struct ht_str *key;
  struct ht_str *value;
};

struct ht_strtuple *ht_strtuple_new(struct ht_strtable *stable, struct ht_str *key, struct ht_str *value);
struct ht_strtuple *ht_strtuple_new_string(struct ht_strtable *stable, char *key, int key_len, char *value, int value_len);
// ref and malloc
struct ht_strtuple *ht_strtuple_copy(struct ht_strtuple *tuple);
// just pointer reference (convenient in python ctypes)
char* ht_strtuple_getkey(struct ht_strtuple *tuple);
char* ht_strtuple_getvalue(struct ht_strtuple *tuple);
// unref and free
void ht_strtuple_destroy(struct ht_strtuple *tuple); 
int ht_strtuple_is_identical(struct ht_strtuple *one, struct ht_strtuple *two);


#endif
