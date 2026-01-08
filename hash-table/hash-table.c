#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"
#include "hash-table.h"

#define MINCAP 8

typedef struct {
  char * key;
  void * value;
} ht_entry;

struct ht {
  ht_entry * entries;
  size_t capacity;
  size_t length;
};

struct ht * create_ht(void) {
  struct ht * table = malloc(sizeof(struct ht));
  if (table == NULL) {
    printf("Insufficient memory!");
    return NULL;
  }
  table->capacity = MINCAP;
  table->length = 0;
  
  table->entries = calloc(table->capacity, sizeof(ht_entry));

  if (table->entries == NULL) {
    free(table);
    printf("Insufficient memory!");
    return NULL;
  }
  return table;
}

void destroy_ht(struct ht * table) {
  size_t i;
  for (i = 0; i < table->capacity; i++) {
    free((void *) table->entries[i].key);
  }
  free(table->entries);
  free(table);
}

int get_key_idx(char * key, struct ht * table, ht_entry * entries) {
  char hex[SHA256_HEX_SIZE];

  sha256_hex(key, strlen(key), hex);

  hex[15] = 0;

  long idx =  strtol(hex, NULL, 16) & (table->capacity - 1);

  while (entries[idx].key != NULL) {
    if (strcmp(entries[idx].key, key) == 0)
      break;
    else if (idx >= table->capacity - 1)
      idx = 0;
    else
      idx++;
  }
  return idx;
}

void * resize_table(struct ht * table) {

  table->capacity *= 2;

  ht_entry * new_entries = calloc(table->capacity, sizeof(ht_entry));
  if (new_entries == NULL) {
    printf("Insufficient memory!");
    return NULL;
  }

  int i;
  for (i = 0; i < table->capacity/2; i++) {
    if (table->entries[i].key == NULL)
      continue;
    int idx = get_key_idx(table->entries[i].key, table, new_entries);
    new_entries[idx].key = table->entries[i].key;
    new_entries[idx].value = table->entries[i].value;
  }
  free(table->entries);
  table->entries = new_entries;
  return new_entries;
}

char * set_entry(char * key, char * value, struct ht * table) {
  if ((int) table->length > (table->capacity * 2/3))
    if (resize_table(table) == NULL)
      return NULL;

  int idx = get_key_idx(key, table, table->entries);

  if (table->entries[idx].key == NULL) {
    key = strdup(key);
    if (key == NULL) {
      printf("Insufficient memory!");
      return NULL;
    }
    table->entries[idx].key = key;
  } else {
    // printf("Existing Key: %s\n'", table->entries[idx].key);
  }
    table->length++;
  table->entries[idx].value = value;
    
  return key;
}

void remove_entry(char * key, ht * table) {
  int idx = get_key_idx(key, table, table->entries);
  free(table->entries[idx].key);
  table->length--;
  if (idx >= table->capacity - 1)
    idx = -1;
  for (int cnt = idx + 1; (key = table->entries[cnt].key) != NULL; cnt++) {
    if (cnt >= table->capacity) {
      cnt = -1;
      continue;
    }
    int new_idx = get_key_idx(key, table, table->entries);
    if (new_idx == cnt)
      break;
    table->entries[new_idx].key = strdup(key);
    table->entries[new_idx].value = table->entries[cnt].value;
    key = NULL;
  }
}

char * get_entry(char * key, struct ht * table) {
  int idx = get_key_idx(key, table, table->entries);

  return table->entries[idx].value;
}

void print_table(ht * table) {
  printf("Hash Table:\nCapacity: %ld\nLength: %ld\n", table->capacity, table->length);
}
