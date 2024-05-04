#include <cstdint>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <utility>

std::pair<char *, size_t> merge_counts(size_t sX, char *fX, size_t sY,
                                       char *fY) {
  size_t output_bytes = 256 * sizeof(uint64_t);
  uint64_t *output = (uint64_t *)malloc(output_bytes);

  uint64_t *fX64 = (uint64_t *)fX;
  uint64_t *fY64 = (uint64_t *)fY;

  for (int i = 0; i < 256; i++) {
    output[i] = fX64[i] + fY64[i];
  }

  return {(char *)output, output_bytes};
}
