/* Copyright 2023 Alin-Ioan Alexandru 312CA */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

typedef struct ll_node_t
{
    void* data;
    struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
} linked_list_t;

struct hashtable_t {
	linked_list_t **buckets; /* Array de liste simplu-inlantuite. */
	/* Nr. total de noduri existente curent in toate bucket-urile. */
	unsigned int size;
	unsigned int hmax; /* Nr. de bucket-uri. */
	/* (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor. */
	unsigned int (*hash_function)(void*);
	/* (Pointer la) Functie pentru a compara doua chei. */
	int (*compare_function)(void*, void*);
	/* (Pointer la) Functie pentru a elibera
		memoria ocupata de cheie si valoare. */
	void (*key_val_free_function)(void*);
};
typedef struct hashtable_t hashtable_t;

typedef struct info info;
struct info {
	void *key;
	void *value;
};

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*));

int ht_has_key(hashtable_t *ht, void *key);

void *ht_get(hashtable_t *ht, void *key);

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

void ht_remove_entry(hashtable_t *ht, void *key);

void ll_free(linked_list_t** pp_list, hashtable_t *ht);

void ht_free(hashtable_t *ht);

int compare_function_strings(void *a, void *b);

void key_val_free_function(void *data);


#endif /* HASHTABLE_H_ */
