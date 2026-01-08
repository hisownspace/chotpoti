#include <stdio.h>
#include <string.h>

#define MAXKEY 64
#define MAXVAL 256
#define MINSIZE 16


typedef struct {
  char key[MAXKEY];
  char strVal[MAXKEY];
  int idx;
} item;

int get_pairs (item * items) {
  char key[MAXKEY];
  char value[MAXKEY];
  char confirm[2];

  int cnt = 0;

  puts("Would you like to enter a key/value pair (y/n)?");
  while (!strcmp(fgets(confirm, 2, stdin), "y")) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    fputs("Please enter a key: ", stdout);
    fgets(key, MAXKEY, stdin);
    key[strcspn(key, "\n")] = 0;

    fputs("Please enter a value: ", stdout);
    fgets(value, MAXKEY, stdin);
    value[strcspn(value, "\n")] = 0;

    strcpy(items[cnt].strVal, value);
    strcpy(items[cnt].key, key);
    items[cnt].idx = items[cnt-1].idx + 1;

    puts("Would you like to enter another pair?");
    cnt++;
  }
  return cnt;
}

item * find_val(item * items, char * str, int len) {
  int i;
  printf("IN SEARCH FUNCTION\n");
  for (i = 0; i < len; i++) {
    printf("%s\n", items[i].key);
    if (!strcmp(items[i].key, str)) {
      printf("match!\n");
      return &items[i];
    }
  }
  return NULL;
}

int main(void) {
  item items[MINSIZE];
  memset(items, 0, sizeof(items));

  int cnt;
  cnt = get_pairs(items);

  int i = 0;
  while (i < cnt) {
    printf("Key: %s\nValue: %s\n\n", items[i].key, items[i].strVal);
    i++;
  }
  
  size_t num_items = sizeof(items) / sizeof(item);

  printf("Size of dictionary: %d\n", (int) num_items);

  item * pair = find_val(items, "key", num_items);
  if (pair) {
  printf("%s\n", pair->strVal);
  }
}
