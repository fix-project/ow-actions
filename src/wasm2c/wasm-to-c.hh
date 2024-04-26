#include <array>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#define NUM_OUTPUT 256

std::tuple<std::string, std::string, std::optional<std::string>> wasm_to_c(
  const void* wasm_source,
  size_t source_size,
  std::function<void( size_t, std::string )> driver_stream_finish_callback );
