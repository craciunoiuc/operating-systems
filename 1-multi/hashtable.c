/*<Cezar Craciunoiu 334CA - C Preprocessor>*/
#include "./hashtable.h"

int hashtable_init(hashtable_t *htable, unsigned int hashtable_size,
					unsigned int max_hash)
{
	unsigned int i;

	htable->arr_sz = hashtable_size;
	htable->max_hash = max_hash;
	htable->arr = malloc(sizeof(bucket_t) * hashtable_size);
	CLOSE(htable->arr, "Malloc failed!", 12);
	for (i = 0; i < htable->arr_sz; ++i)
		htable->arr[i].allocd = 0;
	htable->allocd = 1;
	return 0;
}

void hashtable_free(hashtable_t *htable)
{
	unsigned int i, j;

	if (htable->allocd == 1) {
		for (i  = 0; i < htable->arr_sz; ++i) {
			if (htable->arr[i].allocd == 1) {
				for (j = 0; j < htable->arr[i].b_sz; ++j) {
					if (htable->arr[i].bkt[j].allocd) {
						free(htable->arr[i].bkt[j].k);
						free(htable->arr[i].bkt[j].v);
					}
				}
				free(htable->arr[i].bkt);
			}
		}
		free(htable->arr);
	}
}

const char *hashtable_at(hashtable_t *htable, const char *k,
					unsigned int klen)
{
	unsigned int i;
	unsigned int hash;

	if (htable->allocd == 1) {
		hash = hash_function(k, klen, htable->max_hash);
		if (htable->arr[hash].allocd == 1) {
			for (i = 0; i < htable->arr[hash].b_sz; ++i) {
				if (htable->arr[hash].bkt[i].allocd == 1 &&
					klen == htable->arr[hash].bkt[i].klen &&
					!memcmp(k, htable->arr[hash].bkt[i].k,
							klen * sizeof(char))) {
					return htable->arr[hash].bkt[i].v;
				}
			}
		}
	}
	return NULL;
}

void hashtable_pop(hashtable_t *htable, char *k, unsigned int klen)
{
	unsigned int i;
	unsigned int hash;

	if (htable->allocd == 1) {
		hash = hash_function(k, klen, htable->max_hash);
		if (htable->arr[hash].allocd == 1) {
			for (i = 0; i < htable->arr[hash].b_sz; ++i) {
				if (htable->arr[hash].bkt[i].allocd == 1 &&
					klen == htable->arr[hash].bkt[i].klen &&
					!memcmp(k, htable->arr[hash].bkt[i].k,
							klen)) {
					htable->arr[hash].bkt[i].allocd = 0;
					free(htable->arr[hash].bkt[i].k);
					free(htable->arr[hash].bkt[i].v);
					return;
				}
			}
		}
	}
}

void hashtable_insert(hashtable_t *htable, char *k,
			unsigned int klen, char *v, unsigned int vlen)
{
	unsigned int i, curr_size;
	entry_t *new_space;
	char found = 0;
	unsigned int hash;

	if (htable->allocd == 1) {
		hash = hash_function(k, klen, htable->max_hash);
		if (htable->arr[hash].allocd == 1) {
			/* Search for entry to replace */
			for (i = 0; i < htable->arr[hash].b_sz; ++i) {
				if (htable->arr[hash].bkt[i].allocd == 1 &&
					klen == htable->arr[hash].bkt[i].klen &&
					memcmp(k, htable->arr[hash].bkt[i].k,
							klen) == 0) {
					found = 1;
					free(htable->arr[hash].bkt[i].v);
					htable->arr[hash].bkt[i].v =
						calloc(sizeof(char), vlen + 1);
					CLOSE(htable->arr[hash].bkt[i].v,
							"Malloc failed!", 12);

					memcpy(htable->arr[hash].bkt[i].v, v,
							vlen);
					htable->arr[hash].bkt[i].vlen =
							vlen;
				}
			}

			/* If it's a new entry */
			if (found  == 0) {
				curr_size = htable->arr[hash].b_sz;

				/* If the maximum dimension has been reached */
				if (curr_size == htable->arr[hash].b_sz) {
					htable->arr[hash].b_sz *= 2;
					new_space = malloc(sizeof(entry_t) *
						htable->arr[hash].b_sz);
					CLOSE(new_space, "Malloc failed!", 12);

					memcpy(new_space, htable->arr[hash].bkt,
						sizeof(entry_t) * curr_size);
					free(htable->arr[hash].bkt);
					htable->arr[hash].bkt = new_space;
				}

				htable->arr[hash].bkt[curr_size].k =
						calloc(sizeof(char), klen + 1);
				CLOSE(htable->arr[hash].bkt[curr_size].k,
						"Malloc failed!", 12);
				memcpy(htable->arr[hash].bkt[curr_size].k,
						k, sizeof(char) * klen);
				htable->arr[hash].bkt[curr_size].klen =
						klen;
				htable->arr[hash].bkt[curr_size].v =
						calloc(sizeof(char), vlen + 1);
				CLOSE(htable->arr[hash].bkt[curr_size].v,
						"Malloc failed!", 12);

				memcpy(htable->arr[hash].bkt[curr_size].v,
						v, sizeof(char) * vlen);
				htable->arr[hash].bkt[curr_size].vlen =
						vlen;
				htable->arr[hash].bkt[curr_size].allocd = 1;
				htable->arr[hash].b_sz++;
			}
		} else {
			/* If the bkt doesn't exist */
			htable->arr[hash].bkt = malloc(sizeof(bucket_t));
			CLOSE(htable->arr[hash].bkt, "Malloc failed!", 12);

			htable->arr[hash].bkt[0].allocd = 1;
			htable->arr[hash].bkt[0].k =
						calloc(sizeof(char), klen + 1);
			CLOSE(htable->arr[hash].bkt[0].k,
						"Malloc failed!", 12);
			memcpy(htable->arr[hash].bkt[0].k, k, klen);
			htable->arr[hash].bkt[0].klen = klen;
			htable->arr[hash].bkt[0].v =
						calloc(sizeof(char), vlen + 1);
			CLOSE(htable->arr[hash].bkt[0].v,
						"Malloc failed!", 12);
			memcpy(htable->arr[hash].bkt[0].v, v, vlen);
			htable->arr[hash].bkt[0].vlen = vlen;
			htable->arr[hash].bkt[0].allocd = 1;
			htable->arr[hash].b_sz = 1;
			htable->arr[hash].b_sz = 1;
			htable->arr[hash].allocd = 1;
		}
	} else {
		CLOSE(NULL, "No hashtable!", 1);
	}
}

/* Calculates the hash of a string */
unsigned int hash_function(const char *to_hash, unsigned int to_hash_length,
						unsigned int hash_length)
{
	unsigned int hash = 0;
	unsigned int i    = 0;

	for (i = 0; i != to_hash_length; ++i)
		hash += 31 * to_hash[i];
	hash = (unsigned int) (hash * 1.61803398875);
	hash += to_hash_length * (int) to_hash[0];
	hash += (int) to_hash[to_hash_length - 1];
	return hash % hash_length;
}
