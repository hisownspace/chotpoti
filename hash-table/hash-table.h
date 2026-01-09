#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <stdio.h>

typedef struct ht ht;

ht * create_ht(void);
void destroy_ht(ht * table);
char * set_entry(ht * table, char * key, char * strVal, int intVal);
int get_int_entry(ht * table, char * key);
char * get_str_entry(ht * table, char * key);
void remove_entry(ht * table, char * key);
int get_ht_length(ht * table);

void print_table(ht * table);

#endif
