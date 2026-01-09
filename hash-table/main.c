#include "hash-table.h"
#include <stdio.h>


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

  // for (int i = 28; i < 1000028; i++) {
  //   char str[5];
  //   create_str(str, i);
  //   set_entry(table, str, str);
  //   // printf("%d.\t%d\n", i, str[0]);
  //   // if (i == 488) {
  //   //   test = strdup(str);
  //   // }
  // }

  // for (int i = 28; i < 1000028; i += 2) {
  //   char str[5];
  //   create_str(str, i);
  //   remove_entry(table, str);
  //   // printf("%d\n", i);
  // }
  //
  // for (int i = 28; i < 1000028; i++) {
  //   char str[5];
  //   create_str(str, i);
  //   printf("%s: %s\n", str, get_entry(table, str));
  // }
  //
  // print_table(table);
  char * str1 = "num";
  char * str2 = "str";
  char * str3 = "str1";
  set_entry(table, str1, NULL, 5);
  set_entry(table, str2, "five", 0);
  set_entry(table, str3, "fiv3", 0);
  printf("%s: %d\n", str1, get_int_entry(table, str1));
  printf("%s: %s\n", str2, get_str_entry(table, str2));
  printf("%s: %s\n", str3, get_str_entry(table, str3));
  printf("%s: %s\n", "str", get_str_entry(table, "str"));
  // printf("%d\n", add(4, 5));

  return 0;
}
