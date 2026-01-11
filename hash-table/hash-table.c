#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"
#include "hash-table.h"

#define MINCAP 128
#define INT 0
#define STR 1

typedef struct {
  char * key;
  char * strVal;
  int intVal;
  int type;
} ht_entry ;

struct ht {
  ht_entry * entries;
  size_t capacity;
  size_t length;
};

ht * create_ht(void) {
  ht * table = malloc(sizeof(ht));
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

int get_key_idx(struct ht * table, ht_entry * entries, char * key) {
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
    int idx = get_key_idx(table, new_entries, table->entries[i].key);
    new_entries[idx].key = strdup(table->entries[i].key);
    if (table->entries[i].strVal == NULL)
      new_entries[idx].intVal = table->entries[i].intVal;
    else
      new_entries[idx].strVal = table->entries[i].strVal;
  }
  free(table->entries);
  table->entries = new_entries;
  return new_entries;
}

char * set_entry(struct ht * table, char * key, char * strVal, int intVal) {
  if ((int) table->length > (table->capacity * 2/3))
    if (resize_table(table) == NULL)
      return NULL;

  int idx = get_key_idx(table, table->entries, key);
  if (table->entries[idx].key == NULL) {
    if (key == NULL) {
      printf("Insufficient memory!");
      return NULL;
    }
    table->entries[idx].key = strdup(key);
    table->length++;
  } 
  if (intVal) {
    table->entries[idx].intVal = intVal;
    table->entries[idx].type = INT;
  }
  else {
    table->entries[idx].strVal = strVal;
    table->entries[idx].type = STR;
  }
    
  return key;
}

void remove_entry(ht * table, char * key) {
  int idx = get_key_idx(table, table->entries, key);
  free(table->entries[idx].key);
  table->length--;
    // free(key);
    // free(value);
  if (idx >= table->capacity - 1)
    idx = -1;
  for (int cnt = idx + 1; (key = table->entries[cnt].key) != NULL; cnt++) {
    if (cnt >= table->capacity) {
      cnt = -1;
      continue;
    }
    int new_idx = get_key_idx(table, table->entries, key);
    if (new_idx == cnt)
      break;
    table->entries[new_idx].strVal = table->entries[cnt].strVal;
    table->entries[new_idx].intVal = table->entries[cnt].intVal;
    key = NULL;
  }
}

char * get_str_entry(struct ht * table, char * key) {
  int idx = get_key_idx(table, table->entries, key);

  printf("idx in get_str_entry %d\n", idx);
  return table->entries[idx].strVal;
}

int get_int_entry(struct ht * table, char * key) {
  int idx = get_key_idx(table, table->entries, key);

  return table->entries[idx].intVal;
}

int get_entry_type(struct ht * table, char * key) {
  int idx = get_key_idx(table, table->entries, key);
  printf("%d\n", table->entries[idx].type);
  return table->entries[idx].type;
}

void print_length(ht * table) {
  printf("%d\n", (int) table->length);
}

void print_table(ht * table) {
  printf("{\n");
  for (int i = 0; i < table->capacity; i++) {
    if (table->entries[i].key == NULL)
      continue;
    if (table->entries[i].type == STR)
      printf("%s: \"%s\",\n", table->entries[i].key, table->entries[i].strVal);
    else
      printf("%s: %d,\n", table->entries[i].key, table->entries[i].intVal);
  }
  printf("}\n");
}
