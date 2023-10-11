/* Copyright 2023 Alin-Ioan Alexandru 312CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "Hashtable.h"

linked_list_t *ll_create(unsigned int data_size)
{
    linked_list_t* ll;

    ll = malloc(sizeof(*ll));

    ll->head = NULL;
    ll->data_size = data_size;
    ll->size = 0;

    return ll;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *prev, *curr;
    ll_node_t* new_node;

    if (!list) {
        return;
    }

    /* n >= list->size inseamna adaugarea unui nou nod la finalul listei. */
    if (n > list->size) {
        n = list->size;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    new_node = malloc(sizeof(*new_node));
    new_node->data = malloc(list->data_size);
    memcpy(new_node->data, new_data, list->data_size);

    new_node->next = curr;
    if (prev == NULL) {
        /* Adica n == 0. */
        list->head = new_node;
    } else {
        prev->next = new_node;
    }

    list->size++;
}


ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    if (!list || !list->head) {
        return NULL;
    }

    /* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei. */
    if (n > list->size - 1) {
        n = list->size - 1;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    if (prev == NULL) {
        /* Adica n == 0. */
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;

    return curr;
}


unsigned int ll_get_size(linked_list_t* list)
{
    if (!list) {
        return -1;
    }
    return list->size;
}


/*
 * Functii de comparare a cheilor:
 */
int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functie apelata pentru a elibera memoria ocupata de cheia si valoarea unei
 * perechi din hashtable. Daca cheia sau valoarea contin tipuri de date complexe
 * aveti grija sa eliberati memoria luand in considerare acest aspect.
 */
void key_val_free_function(void *data) {
	info *a = data;
	free(a->key);
	free(a->value);
	free(data);
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista si actualizeaza la
 * NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
void ll_free(linked_list_t** pp_list, hashtable_t *ht)
{
    ll_node_t* currNode;

    if (!pp_list || !*pp_list) {
        return;
    }

    while (ll_get_size(*pp_list) > 0) {
        currNode = ll_remove_nth_node(*pp_list, 0);
        ht->key_val_free_function(currNode->data);
        currNode->data = NULL;
        free(currNode);
        currNode = NULL;
    }

    free(*pp_list);
    *pp_list = NULL;
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t* dict = malloc(sizeof(hashtable_t));
	DIE(!dict, "malloc failed");
	dict->size = 0;
	dict->hmax = hmax;
	dict->hash_function = hash_function;
	dict->compare_function = compare_function;
	dict->key_val_free_function = key_val_free_function;
	dict->buckets = malloc(hmax * sizeof(linked_list_t *));
	DIE(!dict->buckets, "malloc failed");
	for (unsigned int i = 0; i < hmax; i++) {
		dict->buckets[i] = ll_create(sizeof(info));
	}
	return dict;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable
 * folosind functia put;
 * 0, altfel.
 */
int
ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key) % ht->hmax;

	ll_node_t *crt = ht->buckets[hash]->head;
	if (crt == NULL)
		return 0;
	info crt_info = *((info *)crt->data);
	while (crt && ht->compare_function(crt_info.key, key)) {
		crt = crt->next;
		if (crt)
			crt_info = *((info *)crt->data);
	}
	if (crt)
		return 1;
	return 0;
}

void *
ht_get(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key) % ht->hmax;

	ll_node_t *crt = ht->buckets[hash]->head;
	if (crt == NULL)
		return NULL;
	info crt_info = *((info *)crt->data);

	while (crt && ht->compare_function(crt_info.key, key)) {
		crt = crt->next;
		if (crt)
			crt_info = *((info *)crt->data);
	}
	if (crt == NULL)
		return NULL;
	return crt_info.value;
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune tipul ei), in momentul in care
 * se creeaza o noua intrare in hashtable (in cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie
 * a valorii la care pointeaza key si adresa acestei copii trebuie salvata in structura info asociata intrarii din ht.
 * Pentru a sti cati octeti trebuie alocati si copiati, folositi parametrul key_size.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel put(ht, key_actual, value_actual),
 * valoarea la care pointeaza key_actual poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia unei intrari din hashtable.
 * Nu ne dorim acest lucru, fiindca exista riscul sa ajungem in situatia in care nu mai stim la ce cheie este
 * inregistrata o anumita valoare.
 */
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int hash = ht->hash_function(key) % ht->hmax;
	info new_data;
	new_data.key = malloc(key_size);
	DIE(!new_data.key, "malloc failed");
	new_data.value = malloc(value_size);
	DIE(!new_data.value, "malloc failed");
	memcpy(new_data.key, key, key_size);
	memcpy(new_data.value, value, value_size);

	if (ht->buckets[hash]->head) {
		ll_node_t *crt = ht->buckets[hash]->head;
		info crt_info = *((info *)crt->data);

		while (crt && ht->compare_function(crt_info.key, key)) {
			crt = crt->next;
			if (crt)
				crt_info = *((info *)crt->data);
		}

		if (crt) {
			ht->key_val_free_function(crt->data);
			crt->data = malloc(sizeof(info));
			memcpy(crt->data, &new_data, sizeof(info));
		} else {
			ll_add_nth_node(ht->buckets[hash], ht->buckets[hash]->size, &new_data);
		}

	} else {
		ll_add_nth_node(ht->buckets[hash], 0, &new_data);
	}
	ht->size++;
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o
 * intrare din hashtable (adica memoria pentru copia lui key --vezi observatia
 * de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int hash = ht->hash_function(key) % ht->hmax;
	ll_node_t *crt = ht->buckets[hash]->head->next;
	ll_node_t *prev = ht->buckets[hash]->head;
	info crt_info;
	info prev_info = *((info *)prev->data);

	if (ht->compare_function(prev_info.key, key) == 0) {
		ht->key_val_free_function(prev->data);
		ht->buckets[hash]->head = ht->buckets[hash]->head->next;
		free(prev);
	} else {
		crt_info = *((info *)crt->data);
		while (crt->next && ht->compare_function(crt_info.key, key)) {
			prev = prev->next;
			crt = crt->next;
			crt_info = *((info *)crt->data);
		}
		if (ht->compare_function(crt_info.key, key) == 0) {
			prev->next = crt->next;
			ht->key_val_free_function(crt->data);
			free(crt);
		}
	}
	ht->buckets[hash]->size--;
	ht->size--;
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable,
 * dupa care elibereaza si memoria folosita pentru a stoca structura hashtable.
 */
void ht_free(hashtable_t *ht)
{
    unsigned int i;
	for (i = 0; i < ht->hmax; i++) {
		ll_free(&ht->buckets[i], ht);
	}
	free(ht->buckets);
	free(ht);
}

