#ifndef EX_TABLE_H
#define EX_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define WARN_BIN 20
#define WARN_CAP 0
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

int flag_bin = 0;
int flag_cap = 0;

ht_entry *ht_lookup(ht_table *table, char *key) {
	size_t hash = ht_hash(table, key);
	size_t count = 0;
    for (ht_entry *entry = table->data[hash]; entry; entry = entry->next) {
#if WARN_BIN
		if (++count >= WARN_BIN && !flag_bin) {
			fprintf(stderr, "WARNING: bin size over: %lu\n", count);
			flag_bin = 1;
		}
#endif
#if WARN_CAP
		if (table->count / table->dsize > WARN_CAP && !flag_cap) {
			fprintf(stderr, "WARNING: capacity over: %lu%%\n", 100 * table->count / table->dsize);
			flag_cap = 1;
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

#endif
