#include <stdlib.h>
#include <string.h>
#include <utility>

int compare(const void *a, const void *b) {
  const char *s = *(const char **)a;
  const char *t = *(const char **)b;
  return strcmp(s, t);
}

std::pair<char *, size_t> count_words(size_t size, char *file) {
  // size: total size, file: allocated buffer for input of this size

  const char *delim = " \n\t,.!?;";

  size_t num = 1;
  for (size_t i = 0; i < size; i++)
    num += strchr(delim, file[i]) ? 1 : 0;

  char **strings = (char **)malloc(sizeof(char *) * num);

  size_t N = 0;
  char *p = strtok(file, delim);
  do {
    strings[N] = p;
    N++;
  } while ((p = strtok(NULL, delim)));

  qsort((void *)strings, N, sizeof(char *), compare);

  char **keys = (char **)malloc(sizeof(char *) * N);
  size_t *values = (size_t *)malloc(sizeof(size_t) * N);
  size_t i = 0;

  size_t char_count = 0;
  for (size_t j = 0; j < N;) {
    char *current = strings[j];
    char_count += strlen(current);
    size_t k = j;
    while (k < N && !strcmp(strings[k], current))
      k++;
    size_t n = k - j;
    keys[i] = current;
    values[i] = n;
    i++;
    j = k;
  }
  size_t n = i;

  // number of characters in words, plus ",0x%08x\n" on every line
  size_t output_bytes = char_count + n * (1 + 10 + 1);

  char *output = (char *)malloc(output_bytes);

  char *cursor = output;
  for (size_t i = 0; i < n; i++) {
    size_t m = strlen(keys[i]);
    strcpy(cursor, keys[i]);
    cursor += m;
    *cursor++ = ',';
    *cursor++ = '0';
    *cursor++ = 'x';
    for (size_t j = 0; j < 32; j += 4) {
      size_t shift = (28 - j);
      int x = values[i] >> shift;
      char y = "0123456789abcdef"[x];
      *cursor++ = y;
    }
    *cursor++ = '\n';
  }

  return {output, output_bytes};
}
