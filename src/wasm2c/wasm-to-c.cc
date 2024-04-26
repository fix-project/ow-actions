#include "wabt/apply-names.h"
#include "wabt/binary-reader-ir.h"
#include "wabt/binary-reader.h"
#include "wabt/c-writer.h"
#include "wabt/error-formatter.h"
#include "wabt/feature.h"
#include "wabt/filenames.h"
#include "wabt/generate-names.h"
#include "wabt/ir.h"
#include "wabt/option-parser.h"
#include "wabt/sha256.h"
#include "wabt/stream.h"
#include "wabt/validator.h"
#include "wabt/wast-lexer.h"

#include "initcomposer.hh"
#include "memorystringstream.hh"
#include "wasminspector.hh"

#include "wasm-to-c.hh"
#include <functional>

using namespace wabt;
using namespace std;

static vector<size_t> consistent_hashing_name_to_output_file_index( vector<Func*>::const_iterator func_begin,
                                                                    vector<Func*>::const_iterator func_end,
                                                                    size_t num_imports,
                                                                    size_t num_streams,
                                                                    size_t num_parallelism )
{
  vector<size_t> result;
  size_t num_outputs = min( num_streams, num_parallelism );
  result.resize( distance( func_begin, func_end ) );
  if ( num_outputs == 1 ) {
    return result;
  }

  map<string, pair<bool, size_t>> hash_to_index;

  // Insert file indexes to the map
  for ( uint32_t i = 0; i < num_outputs; i++ ) {
    string hash_key;
    wabt::sha256( { reinterpret_cast<char*>( &i ), sizeof( uint32_t ) }, hash_key );
    hash_to_index[hash_key] = { true, i };
  }

  // Insert function indexes to the map
  size_t func_index = 0;
  for ( auto func = func_begin; func != func_end; func++ ) {
    string hash_key;
    wabt::sha256( ( *func )->name, hash_key );
    hash_to_index[hash_key] = { false, func_index };
    func_index++;
  }

  // Find the file with the largest hash key
  size_t file_index = 0;
  for ( auto entry = hash_to_index.rbegin(); entry != hash_to_index.rend(); entry++ ) {
    if ( entry->second.first ) {
      file_index = entry->second.second;
      break;
    }
  }

  // A function should be write to the file on the left side
  for ( auto entry = hash_to_index.begin(); entry != hash_to_index.end(); entry++ ) {
    if ( entry->second.first ) {
      file_index = entry->second.second;
    } else {
      result[entry->second.second] = file_index;
    }
  }

  return result;
}

string fixpoint_c;

static void stream_finish_callback( function<void( size_t, string )> driver_stream_finish_callback,
                                    size_t stream_index,
                                    Stream* stream )
{
  if ( stream_index == 0 ) {
    stream->WriteData( fixpoint_c.data(), fixpoint_c.size() );
  }

  driver_stream_finish_callback( stream_index, static_cast<MemoryStringStream*>( stream )->ReleaseStringBuf() );
}

tuple<string, string, optional<string>> wasm_to_c( const void* wasm_source,
                                                   size_t source_size,
                                                   function<void( size_t, string )> driver_stream_finish_callback )
{
  Errors errors;
  Module module;

  size_t parallelism = 1 + ( source_size >> 14 );

  ReadBinaryOptions options;
  options.features.enable_multi_memory();
  options.features.enable_exceptions();
  options.read_debug_names = true;

  ReadBinaryIr( "function", wasm_source, source_size, options, &errors, &module );

  array<MemoryStringStream, NUM_OUTPUT> c_streams;
  vector<Stream*> c_stream_ptrs;
  for ( auto& s : c_streams ) {
    c_stream_ptrs.emplace_back( &s );
  }
  MemoryStringStream h_impl_stream;
  MemoryStringStream h_stream;

  ValidateModule( &module, &errors, options.features );
  GenerateNames( &module );
  ApplyNames( &module );

  wasminspector::WasmInspector inspector( &module, &errors );
  inspector.Validate();

  for ( auto index : inspector.GetExportedROMemIndex() ) {
    module.memories[index]->bounds_checked = true;
  }
  for ( auto index : inspector.GetExportedRWMemIndex() ) {
    module.memories[index]->bounds_checked = true;
  }

  fixpoint_c = initcomposer::compose_header( "function", &module, &errors, &inspector );

  WriteCOptions write_c_options;
  write_c_options.module_name = "function";
  write_c_options.name_to_output_file_index = bind( consistent_hashing_name_to_output_file_index,
                                                    placeholders::_1,
                                                    placeholders::_2,
                                                    placeholders::_3,
                                                    placeholders::_4,
                                                    parallelism );
  write_c_options.stream_finish_callback
    = bind( stream_finish_callback, driver_stream_finish_callback, placeholders::_1, placeholders::_2 );
  WriteC( std::move( c_stream_ptrs ),
          &h_stream,
          &h_impl_stream,
          "function.h",
          "function-impl.h",
          &module,
          write_c_options );

  string error_string = FormatErrorsToString( errors, Location::Type::Text );
  return { h_stream.ReleaseStringBuf(),
           h_impl_stream.ReleaseStringBuf(),
           errors.empty() ? nullopt : make_optional( error_string ) };
}
