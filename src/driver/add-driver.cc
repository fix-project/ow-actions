#include "nlohmann/json.hpp"

#include "s3.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_x = args["x"].get<int>();
  auto input_y = args["y"].get<int>();

  printf("{ \"count\": %d }", input_x + input_y);
  return 0;
}
