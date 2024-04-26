#include <string>

static std::string get_base_name( std::string filename )
{
  size_t last_slash = filename.find_last_of( '/' );
  size_t last_backslash = filename.find_last_of( '\\' );
  if ( last_slash == std::string_view::npos && last_backslash == std::string_view::npos ) {
    return filename;
  }

  if ( last_slash == std::string_view::npos ) {
    if ( last_backslash == std::string_view::npos ) {
      return filename;
    }
    last_slash = last_backslash;
  } else if ( last_backslash != std::string_view::npos ) {
    last_slash = std::max( last_slash, last_backslash );
  }

  return filename.substr( last_slash + 1 );
}
