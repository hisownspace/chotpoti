#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <stdio.h>
typedef struct ht ht;

ht * create_ht(void);
void destroy_ht(ht * table);
char * set_entry(char * key, char * value, ht * table);
char * get_entry(char * key, ht * table);
void remove_entry( char * key, ht * table);

void print_table(ht * table);

#endif
