#include "hash-table.h"
#include <stdio.h>
#include <string.h>

void create_str(char str[5], int num) {
    int seed = num;
    int idx = 0;
    while (seed > 1) {
      str[idx] = ((seed % 26) + 97);
      seed /= 26;
      idx++;
    }
    str[idx] = 0;
}

int main () {
  ht * table = create_ht();

  char * test;

  for (int i = 28; i < 1000028; i++) {
    char str[5];
    create_str(str, i);
    set_entry(str, str, table);
    // printf("%d.\t%d\n", i, str[0]);
    // if (i == 488) {
    //   test = strdup(str);
    // }
  }

  for (int i = 28; i < 1000028; i += 2) {
    char str[5];
    create_str(str, i);
    remove_entry(str, table);
    // printf("%d\n", i);
  }

  for (int i = 28; i < 1000028; i++) {
    char str[5];
    create_str(str, i);
    printf("%s: %s\n", str, get_entry(str, table));
  }

  print_table(table);

  return 0;
}
