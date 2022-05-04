#ifndef _MAP_H_
#define _MAP_H_

struct bkt_t
{
	char *key;
	char *value;
	struct bkt_t *next;
};

typedef struct
{
	int length;
	struct bkt_t **bkts;
	int(*hash)(char *key);
} map_t;

map_t map_create(int length, int(*hash)(char *key));
char *map_get(map_t *m, char *key);
void map_set(map_t *m, char *key, char *value);
void map_add(map_t *m, char *key, char *value);
void map_rm(map_t *m, char *key);
void map_destroy(map_t *m);

#endif
