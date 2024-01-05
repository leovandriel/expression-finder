#include <assert.h>
#include <time.h>

#include "table.h"

void ht_test()
{
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
		for (int i = 0; i < count; i++)
		{
			sprintf(buffer, "%d", i);
			ht_set(&table, buffer, buffer);
			assert((int)table.count == i + 1);
		}
		for (int i = 0; i < count; i++)
		{
			sprintf(buffer, "%d", i);
			assert(strcmp(ht_get(&table, buffer), buffer) == 0);
		}
		for (int i = 0; i < count; i++)
		{
			sprintf(buffer, "%d", i);
			ht_del(&table, buffer);
			assert((int)table.count == count - i - 1);
		}
		ht_free(&table);
	}

	{ // low capacity
		int count = 9;
		char buffer[4];
		for (int c = 1; c < count; c++)
		{
			ht_table table;
			ht_init(&table, c);
			for (int i = 0; i < count; i++)
			{
				sprintf(buffer, "%d", i);
				ht_set(&table, buffer, buffer);
				assert((int)table.count == i + 1);
			}
			for (int i = 0; i < count; i++)
			{
				sprintf(buffer, "%d", i);
				assert(strcmp(ht_get(&table, buffer), buffer) == 0);
			}
			for (int i = 0; i < count; i++)
			{
				sprintf(buffer, "%d", i);
				ht_del(&table, buffer);
				assert((int)table.count == count - i - 1);
			}
			ht_free(&table);
		}
	}
}

void ht_perf(size_t table_size, size_t key_count, size_t get_count, int key_length)
{
	int buffer_size = key_length + 1;
	char *buffer = malloc(sizeof(char) * buffer_size);
	char *keys = (char *)malloc(sizeof(char *) * key_count * buffer_size);
	srand(1);
	for (size_t i = 0; i < key_count; i++)
	{
		sprintf(buffer, "%.*e", key_length - 6, (double)rand() / RAND_MAX);
		memcpy(keys + i * buffer_size, buffer, buffer_size);
	}
	ht_table table;
	ht_init(&table, table_size);
	clock_t start = clock();
	for (size_t i = 0; i < key_count; i++)
	{
		char *key = keys + (i % key_count) * buffer_size;
		ht_set(&table, key, key);
	}
	double delta = (clock() - start) / (double)CLOCKS_PER_SEC;
	printf("%s: table_size:%8lu key_count:%8lu get_count:%10lu key_length:%3i ", get_count ? "get" : "set", table_size, key_count, get_count, key_length);
	if (get_count == 0)
	{
		printf("time:%.3fs unit:%.1fns\n", delta, 1e9 * delta / key_count);
		return;
	}
	start = clock();
	for (size_t i = 0; i < get_count; i++)
	{
		ht_get(&table, keys + (i % key_count) * buffer_size);
	}
	delta = (clock() - start) / (double)CLOCKS_PER_SEC;
	printf("time:%.3fs unit:%.1fns\n", delta, 1e9 * delta / get_count);
	free(keys);
	free(buffer);
	ht_free(&table);
}

void ht_perfs()
{
	size_t base_table_size = 1e3;
	size_t base_key_count = 1e3;
	size_t base_get_count = 1e8;
	int base_key_length = 10;

	printf("baseline\n");
	for (int i = 0; i <= 1; i++)
	{
		ht_perf(base_table_size, base_key_count, i * base_get_count, base_key_length);
	}

	printf("table size\n");
	for (int i = 0; i <= 1; i++)
	{
		for (size_t table_size = base_table_size / 8; table_size <= base_table_size * 8; table_size *= 2)
		{
			ht_perf(table_size, base_key_count, i * base_get_count, base_key_length);
		}
	}

	printf("key count\n");
	for (int i = 0; i <= 1; i++)
	{
		for (size_t key_count = base_key_count / 8; key_count <= base_key_count * 8; key_count *= 2)
		{
			ht_perf(base_table_size, key_count, i * base_get_count, base_key_length);
		}
	}

	printf("key length\n");
	for (int i = 0; i <= 1; i++)
	{
		for (int key_length = base_key_length / 1; key_length <= base_key_length * 8; key_length *= 2)
		{
			ht_perf(base_table_size, base_key_count, i * base_get_count, key_length);
		}
	}

	printf("large table\n");
	for (size_t table_size = 1e3; table_size <= 1e9; table_size *= 10)
	{
		ht_perf(table_size, table_size, 0, base_key_length);
		ht_perf(table_size, table_size, 1e6, base_key_length);
	}
}

int main()
{
	ht_test();
	ht_perfs();
}
