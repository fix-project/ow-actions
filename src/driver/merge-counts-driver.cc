#include "nlohmann/json.hpp"

#include "s3.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_x = args["input_x"]["count"].get<size_t>();
  auto input_y = args["input_y"]["count"].get<size_t>();

  printf("{ \"count\": %zu }", input_x + input_y );
  return 0;
}
