#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <utility>

std::pair<char *, size_t> count_words(size_t size, char *file) {
  size_t output_bytes = 256 * sizeof(uint64_t);
  uint64_t *output = (uint64_t *)malloc(output_bytes);

  for (size_t i = 0; i < size; i++) {
    output[file[i]]++;
  }

  return {(char *)output, output_bytes};
}
