#pragma once

#include <cstdint>
#include <immintrin.h>
#include <stdexcept>
#include <string>

namespace base16 {

static constexpr char base16_chars[] = "0123456789abcdef";

static uint8_t pos_of_char( const unsigned char chr )
{
  if ( chr >= 'A' && chr <= 'Z' )
    return chr - 'A' + 10;
  else if ( chr >= 'a' && chr <= 'z' )
    return chr - 'a' + 10;
  else if ( chr >= '0' && chr <= '9' )
    return chr - '0';
  else
    throw std::runtime_error( "Input is not valid base16-encoded data: contains byte "
                              + std::to_string( (size_t)chr ) );
}

__attribute__( ( unused ) ) static std::string encode( std::string name )
{
  std::string ret;
  ret.reserve( 64 );

  for ( size_t i = 0; i < 32; i++ ) {
    unsigned char c = name[i] & 0xff;
    ret.push_back( base16_chars[c >> 4] );
    ret.push_back( base16_chars[c & 0xf] );
  }

  return ret;
}

__attribute__( ( unused ) ) static std::string decode( std::string_view encoded_name )
{
  std::string res;
  res.reserve( 32 );

  if ( encoded_name.size() != 64 ) {
    throw std::runtime_error( "Input encoded data is not of length 64." );
  }

  for ( size_t i = 0; i < 32; i++ ) {
    auto x = pos_of_char( encoded_name[2 * i] );
    auto y = pos_of_char( encoded_name[2 * i + 1] );

    res.push_back( x << 4 | y );
  }

  return res;
}

}
