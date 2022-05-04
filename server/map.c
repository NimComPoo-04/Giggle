#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"

map_t map_create(int length, int(*hash)(char *key))
{
	map_t m = {0};

	m.length = length;
	m.bkts = calloc(sizeof(struct bkt_t *), length);
	m.hash = hash;

	return m;
}

char *map_get(map_t *m, char *key)
{
	int hsh = m->hash(key) % m->length;

	struct bkt_t *b = m->bkts[hsh];
	while(b)
	{
		if(strcmp(key, b->key) == 0)
			return b->value;
		b = b->next;
	}

	return NULL;
}

void map_set(map_t *m, char *key, char *value)
{
	int hsh = m->hash(key) % m->length;

	struct bkt_t *b = m->bkts[hsh];
	while(b)
	{
		if(strcmp(key, b->key) == 0)
		{
			b->value = value;
			break;
		}
		b = b->next;
	}
}

void map_add(map_t *m, char *key, char *value)
{
	int hsh = m->hash(key) % m->length;

	struct bkt_t *b = m->bkts[hsh];

	if(b == NULL)
	{
		m->bkts[hsh] = calloc(sizeof(struct bkt_t), 1);
		m->bkts[hsh]->key = key;
		m->bkts[hsh]->value= value;
		m->bkts[hsh]->next = NULL;
		return;
	}

	struct bkt_t *n = b->next;

	b->next = calloc(sizeof(struct bkt_t), 1);
	b->next->key = key;
	b->next->value = value;
	b->next->next = n;
}

void map_rm(map_t *m, char *key)
{
	int hsh = m->hash(key) % m->length;

	struct bkt_t *b = m->bkts[hsh];

	if(strcmp(key, b->key) == 0)
	{
		m->bkts[hsh] = b->next;
		free(b);
		return;
	}

	while(b->next)
	{
		if(strcmp(key, b->next->key) == 0)
		{
			void *x = b->next;
			b->next = b->next->next;
			free(x);
			break;
		}
		b = b->next;
	}
}

void map_destroy(map_t *m)
{
	for(int i = 0; i < m->length; i++)
	{
		struct bkt_t *b = m->bkts[i];
		while(b)
		{
			void *g = b->next;
			free(b);
			b = g;
		}
	}
	free(m->bkts);
}
