/*<Cezar Craciunoiu 334CA - C Preprocessor>*/
#ifndef __HASHTABLE__
#define __HASHTABLE__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*Closes the execution when cond is NULL, printing msg and returning code*/
#define CLOSE(cond, msg, code) do { if (cond == NULL) {perror(msg); \
					exit(code);\
					} } while (0)

/* A key - value entry in the hashtable*/
typedef struct {
	char *k;
	unsigned int klen;
	char *v;
	unsigned int vlen;
	char allocd;
} entry_t;

/*A bucket, which contains a variable ammount of entries*/
typedef struct {
	unsigned int hash;
	entry_t *bkt;
	unsigned int b_sz;
	unsigned int b_curr_sz;
	char allocd;
} bucket_t;

/*The hashtable, which has a fixed size array made of buckets*/
typedef struct {
	bucket_t *arr;
	unsigned int arr_sz;
	unsigned int max_hash;
	char allocd;
} hashtable_t;

/*Initialises a hashtable with hashtable_size
 *elements and a maximum hash of max_hash
 */
int hashtable_init(hashtable_t *hashtable,
			unsigned int hashtable_size, unsigned int max_hash);

/*Frees a hashtable by passing through all elements
 *and calling free on them if they are allocated
 */
void hashtable_free(hashtable_t *hashtable);

/*Inserts a pair of key&value with their corresponding sizes in a hashtable*/
void hashtable_insert(hashtable_t *hashtable, char *key,
	unsigned int key_length, char *value, unsigned int value_length);

/*Removes the entry with the key if it is found in the hashtable*/
void hashtable_pop(hashtable_t *hashtable, char *key, unsigned int key_length);

/*Returns the value at a given key*/
const char *hashtable_at(hashtable_t *hashtable, const char *key,
						unsigned int key_length);

/*Calculates the hash of a string of bytes*/
unsigned int hash_function(const char *to_hash, unsigned int to_hash_length,
						unsigned int hash_length);

#endif
