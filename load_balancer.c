/* Copyright 2023 Alin-Ioan Alexandru 312CA*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "load_balancer.h"
#include "server.h"
#include "utils.h"

struct load_balancer {
	server_memory **server_arr;
	int size;
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

// description in load_balancer.h
load_balancer *init_load_balancer()
{
	load_balancer *ld_bal = malloc(sizeof(load_balancer));
	DIE(!ld_bal, "malloc failed");
	ld_bal->server_arr = NULL;
	ld_bal->size = 0;
	return ld_bal;
}

// binary search on the hash-ring to find the server closest to the new_hash
// search stops when the next hash in array is greater than new_hash and
// previous hash is less than new_hash
int find_next_server(load_balancer *main, unsigned int new_hash,
					 int left_pos, int right_pos)
{
	// check if hash is ouside of array
	if (main->size == 0)
		return 0;
	if (new_hash > hash_function_servers(&main->server_arr[right_pos]->id) ||
		new_hash < hash_function_servers(&main->server_arr[left_pos]->id))
		return 0;
	int mid_pos;
	unsigned int h_mid, h_mid_prev;

	while (left_pos <= right_pos) {
        mid_pos = (left_pos + right_pos) / 2;

		h_mid = hash_function_servers(&main->server_arr[mid_pos]->id);
		if (mid_pos - 1 > 0)
			h_mid_prev = hash_function_servers(&main->server_arr[mid_pos - 1]->id);
		else
			h_mid_prev = 0;  // lowest value for compare
		// stop conition
		if (new_hash > h_mid_prev && new_hash < h_mid) {
			return mid_pos;
		} else if (new_hash < h_mid) {
            right_pos = mid_pos - 1;
		} else {
            left_pos = mid_pos + 1;
		}
    }
	if (left_pos >= main->size)
		return 0;
	if (left_pos < 0)
		return 0;
	return left_pos;
}

// iterative search used to find the next server, only used when adding or
// removing servers
int find_new_server_pos(load_balancer *main, int server_id)
{
	if (main->size == 0)
		return 0;

	int i;
	unsigned int new_hash = hash_function_servers(&server_id), crt_hash;
	for (i = 0; i < main->size; i++) {
		crt_hash = hash_function_servers(&main->server_arr[i]->id);
		if (crt_hash >= new_hash)
			return i;
	}
	return i;
}

// adds new server to hash ring
void add_new_server(load_balancer *main, int server_id, int pos)
{
	int i;
	for (i = main->size; i > pos; i--) {
		main->server_arr[i] = main->server_arr[i - 1];
	}
	main->server_arr[pos] = init_server_memory();
	main->server_arr[pos]->id = server_id;
	main->size++;
}

// redistributes the objects from the next server to the new server that was
// added
void redistribute_objects_add(load_balancer *main, int server_pos)
{
	int i, next_pos;
	unsigned int object_hash, crt_hash, next_hash;
	server_memory *crt_server, *next_server;

	if (server_pos == main->size - 1)
		next_pos = 0;  // the next server is the first server (loop around)
	else
		next_pos = server_pos + 1;

	crt_server = main->server_arr[server_pos];
	next_server = main->server_arr[next_pos];
	crt_hash = hash_function_servers(&crt_server->id);
	next_hash = hash_function_servers(&next_server->id);

	// go through next server buckets
	for (i = 0; i < (int)next_server->ht->hmax; i++) {
		ll_node_t *crt = next_server->ht->buckets[i]->head, *crt_temp;
		// go through current bucket
		while (crt) {
			object_hash = hash_function_key(((info *)crt->data)->key);
			crt_temp = crt->next;
			// check if the hash is good to add to next server
			if ((next_pos != 0 && object_hash < crt_hash) ||
				(next_pos == 0 && object_hash < crt_hash && object_hash > next_hash) ||
				(next_pos == 1 && object_hash > next_hash)) {
				char *crt_key = (char *)((info *)crt->data)->key;
				char *crt_value = (char *)((info *)crt->data)->value;
				int key_len = strlen(crt_key) + 1;
				int val_len = strlen(crt_value) + 1;
				ht_put(crt_server->ht, crt_key, key_len, crt_value, val_len);
				ht_remove_entry(next_server->ht, crt_key);
			}
			crt = crt_temp;
		}
	}
}

// description in load_balancer.h
void loader_add_server(load_balancer *main, int server_id)
{
	int pos;
	if (!main->server_arr) {  // for first server malloc is used
		main->server_arr = malloc(sizeof(server_memory) * 3);
		DIE(!main->server_arr, "malloc failed!");
	} else {
		main->server_arr = realloc(main->server_arr,
								   sizeof(server_memory) * (main->size + 3));
		DIE(!main->server_arr, "malloc failed!");
	}
	// original
	pos = find_new_server_pos(main, server_id);
	add_new_server(main, server_id, pos);
	redistribute_objects_add(main, pos);
	// replica 1
	pos = find_new_server_pos(main, 100000 + server_id);
	add_new_server(main, 100000 + server_id, pos);
	redistribute_objects_add(main, pos);
	// replica 2
	pos = find_new_server_pos(main, 2*100000 + server_id);
	add_new_server(main, 2*100000 + server_id, pos);
	redistribute_objects_add(main, pos);
}

// removes a server from the hash ring
void remove_from_array(load_balancer *main, int pos)
{
	int i;
	ht_free(main->server_arr[pos]->ht);
	free(main->server_arr[pos]);
	for (i = pos; i < main->size; i++) {
        main->server_arr[i] = main->server_arr[i + 1];
    }
	main->size--;
}

// redistributes the objects from the current that will be deleted to the next
// server
void redistribute_objects_remove(load_balancer *main, int server_pos)
{
	int i, next_pos;
	server_memory *crt_server, *next_server;

	if (server_pos == main->size - 1)
		next_pos = 0;  // the next server is the first server (loop around)
	else
		next_pos = server_pos + 1;

	crt_server = main->server_arr[server_pos];
	next_server = main->server_arr[next_pos];

	// go through current server buckets
	for (i = 0; i < (int)crt_server->ht->hmax; i++) {
		ll_node_t *crt = crt_server->ht->buckets[i]->head;
		// all items in the bucket will be moved to the new hashtable
		while (crt) {
			char *crt_key = (char *)((info *)crt->data)->key;
			char *crt_value = (char *)((info *)crt->data)->value;
			ht_put(next_server->ht, crt_key, strlen(crt_key) + 1,
					crt_value, strlen(crt_value) + 1);
			crt = crt->next;
		}
	}
}

// description in load_balancer.h
void loader_remove_server(load_balancer *main, int server_id)
{
	int pos, replica_id;

	// original
	pos = find_new_server_pos(main, server_id);
	redistribute_objects_remove(main, pos);
	remove_from_array(main, pos);
	// replica 1
	replica_id = 100000 + server_id;
	pos = find_new_server_pos(main, replica_id);
	redistribute_objects_remove(main, pos);
	remove_from_array(main, pos);
	// replica 2
	replica_id = 2*100000 + server_id;
	pos = find_new_server_pos(main, replica_id);
	redistribute_objects_remove(main, pos);
	remove_from_array(main, pos);

	main->server_arr = realloc(main->server_arr,
							   sizeof(server_memory) * main->size);
}

// description in load_balancer.h
void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash_object = hash_function_key(key);
	int pos;
	pos = find_next_server(main, hash_object, 0, main->size - 1);
	server_store(main->server_arr[pos], key, value);
	*server_id = main->server_arr[pos]->id % 100000;
}

// description in load_balancer.h
char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash_object = hash_function_key(key);
	int pos;
	pos = find_next_server(main, hash_object, 0, main->size - 1);
	char *value = (char *)ht_get(main->server_arr[pos]->ht, key);
	*server_id = main->server_arr[pos]->id % 100000;
	return value;
}

// description in load_balancer.h
void free_load_balancer(load_balancer *main)
{
	int i;
	for (i = 0; i < main->size; i++) {
		ht_free(main->server_arr[i]->ht);
		free(main->server_arr[i]);
	}
	free(main->server_arr);
	free(main);
}
