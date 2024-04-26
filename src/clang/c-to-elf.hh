#include <string>
#include <vector>

std::pair<bool, std::string> c_to_elf( const std::vector<char*>& system_dep_files,
                                       const std::vector<char*>& clang_dep_files,
                                       char* function_c_buffer,
                                       char* function_h_impl_buffer,
                                       char* function_h_buffer );
