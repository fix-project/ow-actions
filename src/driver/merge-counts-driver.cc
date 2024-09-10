#include "nlohmann/json.hpp"

#include "s3.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_x = args["input_x"]["count"].get<size_t>();
  auto input_y = args["input_y"]["count"].get<size_t>();
  auto logging = args.value("logging", false);

  if (logging) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec);
  }

  printf("{ \"count\": %zu }", input_x + input_y);
  return 0;
}
