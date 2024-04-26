#include "wabt/stream.h"

class MemoryStringStream : public wabt::Stream
{
public:
  WABT_DISALLOW_COPY_AND_ASSIGN( MemoryStringStream )
  explicit MemoryStringStream( Stream* log_stream = nullptr )
    : Stream( log_stream )
  {}

  std::string&& ReleaseStringBuf() { return std::move( buf_ ); }
  std::string_view Buf() { return { buf_ }; }

protected:
  wabt::Result WriteDataImpl( size_t dst_offset, const void* src, size_t size ) override
  {
    if ( size == 0 ) {
      return wabt::Result::Ok;
    }
    const size_t end = dst_offset + size;
    if ( end > buf_.size() ) {
      buf_.resize( end );
    }
    char* dst = &buf_[dst_offset];
    memcpy( dst, src, size );
    return wabt::Result::Ok;
  }

  wabt::Result MoveDataImpl( size_t dst_offset, size_t src_offset, size_t size ) override
  {
    if ( size == 0 ) {
      return wabt::Result::Ok;
    }
    const size_t src_end = src_offset + size;
    const size_t dst_end = dst_offset + size;
    const size_t end = src_end > dst_end ? src_end : dst_end;
    if ( end > buf_.size() ) {
      buf_.resize( end );
    }

    char* dst = &buf_[dst_offset];
    char* src = &buf_[src_offset];
    memmove( dst, src, size );
    return wabt::Result::Ok;
  }

  wabt::Result TruncateImpl( size_t size ) override
  {
    if ( size > buf_.size() ) {
      return wabt::Result::Error;
    }
    buf_.resize( size );
    return wabt::Result::Ok;
  }

private:
  std::string buf_ {};
};
