/* Copyright 2023 Alin-Ioan Alexandru 312CA */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "Hashtable.h"
#include "load_balancer.h"
#include "utils.h"

#define HMAX 500

// description in server.h
server_memory *init_server_memory()
{
	server_memory *server = malloc(sizeof(server_memory));
	DIE(!server, "malloc failed");
	server->ht = ht_create(HMAX, hash_function_key,
						   compare_function_strings, key_val_free_function);
	server->id = 0;
	return server;
}

// description in server.h
void server_store(server_memory *server, char *key, char *value) {
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

// description in server.h
char *server_retrieve(server_memory *server, char *key) {
	return (char *)ht_get(server->ht, key);
}

// description in server.h
void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server->ht, key);
}

// description in server.h
void free_server_memory(server_memory *server) {
	ht_free(server->ht);
	free(server);
}
