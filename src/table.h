#ifndef EX_TABLE_H
#define EX_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define WARN_COLLISION 10
typedef struct ht_entry {
    struct ht_entry *next;
    char *key;
    char *value;
} ht_entry;

typedef struct ht_table {
	ht_entry **data;
	size_t dsize;
	size_t count;
} ht_table;

typedef struct ht_iterator {
	ht_table *table;
	size_t index;
	ht_entry *entry;
} ht_iterator;

ht_iterator ht_iterate(ht_table *table) {
	ht_iterator iterator;
	iterator.table = table;
	iterator.index = -1;
	iterator.entry = NULL;
	return iterator;
}

ht_entry *ht_next(ht_iterator *iter) {
	if (iter->entry) {
		iter->entry = iter->entry->next;
		if (iter->entry) {
			return iter->entry;
		}
	}
	ht_table *table = iter->table;
	while (++iter->index < table->dsize) {
		iter->entry = table->data[iter->index];
		if (iter->entry) {
			return iter->entry;
		}
	}
	return NULL;
}

void ht_init(ht_table *table, size_t dsize) {
	size_t s = sizeof(*(table->data)) * dsize;
	table->data = (ht_entry **)malloc(s);
	memset(table->data, 0, s);
	table->dsize = dsize;
	table->count = 0;
}

void ht_free(ht_table *table) {
	for (size_t i = 0; i < table->dsize; i++) {
		ht_entry *entry = table->data[i];
		while (entry) {
			free(entry->key);
			free(entry->value);
			ht_entry *next = entry->next;
			free(entry);
			entry = next;
		}
	}
	free(table->data);
	table->data = NULL;
}

size_t ht_hash(ht_table *table, char *value) {
    size_t hash;
    for (hash = 0; *value != '\0'; value++) {
    	hash = *value + 31 * hash;
    }
    return hash % table->dsize;
}

char *ht_copy(char *s) {
    char *p = (char *)malloc((strlen(s) + 1) * sizeof(char));
    if (p) {
        strcpy(p, s);
    }
    return p;
}

int collision_flag = 0;

ht_entry *ht_lookup(ht_table *table, char *key) {
	size_t hash = ht_hash(table, key);
	int count = 0;
    for (ht_entry *entry = table->data[hash]; entry; entry = entry->next) {
#if WARN_COLLISION
		if (++count >= WARN_COLLISION && !collision_flag) {
			fprintf(stderr, "WARNING: hash collision over: %d\n", count);
			collision_flag = 1;
		}
#endif
        if (!strcmp(key, entry->key)) {
            return entry;
        }
    }
    return NULL;
}

char *ht_get(ht_table *table, char *key) {
    ht_entry *entry = ht_lookup(table, key);
    if (!entry) {
    	return NULL;
    }
    return entry->value;
}

ht_entry *ht_set(ht_table *table, char *key, char *value) {
    ht_entry *entry = ht_lookup(table, key);
    size_t hash;
    if (!entry) {
        entry = (ht_entry *)malloc(sizeof(*entry));
        if (!entry) {
            return NULL;
        }
        entry->key = ht_copy(key);
        if (!entry->key) {
        	free(entry);
            return NULL;
        }
        hash = ht_hash(table, key);
        entry->next = table->data[hash];
        table->data[hash] = entry;
        table->count++;
    } else {
        free(entry->value);
    }
    entry->value = ht_copy(value);
    if (!entry->value) {
        return NULL;
    }
    return entry;
}

void ht_del(ht_table *table, char *key) {
	size_t hash = ht_hash(table, key);
	ht_entry *entry = table->data[hash];
    if (!strcmp(key, entry->key)) {
    	table->data[hash] = entry->next;
    } else {
    	ht_entry *prev = entry;
	    for (entry = entry->next; entry; prev = entry, entry = entry->next) {
	        if (!strcmp(key, entry->key)) {
		    	prev->next = entry->next;
		    	break;
	        }
	    }
    }
    if (entry) {
		table->count--;
		free(entry->key);
		free(entry->value);
		free(entry);
    }
}

void ht_test() {
	{ // empty table get
		ht_table table;
		ht_init(&table, 1);
		assert(table.count == 0);
		assert(ht_get(&table, "a") == NULL);
		ht_free(&table);
    }

	{ // set and get 1
		ht_table table;
		ht_init(&table, 1);
		ht_set(&table, "a", "1");
		assert(table.count == 1);
		assert(strcmp(ht_get(&table, "a"), "1") == 0);
		ht_free(&table);
    }

	{ // set and get 2
		ht_table table;
		ht_init(&table, 2);
		ht_set(&table, "a", "1");
		ht_set(&table, "b", "2");
		assert(table.count == 2);
		assert(strcmp(ht_get(&table, "a"), "1") == 0);
		assert(strcmp(ht_get(&table, "b"), "2") == 0);
		ht_free(&table);
    }

	{ // delete
		ht_table table;
		ht_init(&table, 1);
		ht_set(&table, "a", "1");
		assert(table.count == 1);
		ht_del(&table, "a");
		assert(table.count == 0);
		assert(ht_get(&table, "a") == NULL);
		ht_free(&table);
    }

	{ // iterate
		ht_table table;
		ht_init(&table, 2);
		ht_iterator iter = ht_iterate(&table);
		assert(ht_next(&iter) == NULL);
		ht_set(&table, "a", "1");
		iter = ht_iterate(&table);
		ht_entry *entry = ht_next(&iter);
		assert(strcmp(entry->key, "a") == 0);
		assert(strcmp(entry->value, "1") == 0);
		assert(ht_next(&iter) == NULL);
		ht_set(&table, "b", "2");
		iter = ht_iterate(&table);
		entry = ht_next(&iter);
		assert(strcmp(entry->key, "b") == 0);
		assert(strcmp(entry->value, "2") == 0);
		entry = ht_next(&iter);
		assert(strcmp(entry->key, "a") == 0);
		assert(strcmp(entry->value, "1") == 0);
		assert(ht_next(&iter) == NULL);
		ht_free(&table);

	}

	{ // high capacity
		int count = 9;
		char buffer[4];
		ht_table table;
		ht_init(&table, 100);
		for (int i = 0; i < count; i++) {
			sprintf(buffer, "%d", i);
			ht_set(&table, buffer, buffer);
			assert((int)table.count == i + 1);
		}
		for (int i = 0; i < count; i++) {
			sprintf(buffer, "%d", i);
			assert(strcmp(ht_get(&table, buffer), buffer) == 0);
		}
		for (int i = 0; i < count; i++) {
			sprintf(buffer, "%d", i);
			ht_del(&table, buffer);
			assert((int)table.count == count - i - 1);
		}
		ht_free(&table);
    }

	{ // low capacity
		int count = 9;
		char buffer[4];
		for (int c = 1; c < count; c++) {
			ht_table table;
			ht_init(&table, c);
			for (int i = 0; i < count; i++) {
				sprintf(buffer, "%d", i);
				ht_set(&table, buffer, buffer);
				assert((int)table.count == i + 1);
			}
			for (int i = 0; i < count; i++) {
				sprintf(buffer, "%d", i);
				assert(strcmp(ht_get(&table, buffer), buffer) == 0);
			}
			for (int i = 0; i < count; i++) {
				sprintf(buffer, "%d", i);
				ht_del(&table, buffer);
				assert((int)table.count == count - i - 1);
			}
			ht_free(&table);
		}
    }
}

#endif
