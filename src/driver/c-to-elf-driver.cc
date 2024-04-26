#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>

#include "mmap.hh"
#include "nlohmann/json.hpp"

#include "c-to-elf.hh"
#include "depfile.hh"
#include "s3.hh"

using namespace std;
using json = nlohmann::json;

void do_clang( string bucket, size_t index ) {
  // Credential: minioadmin, minioadmin
  const char* key = "minioadmin";
  Aws::Auth::AWSCredentials credential( key, key );

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = "172.31.8.132:9000";
  config.verifySSL = false;

  Aws::S3::S3Client client( credential, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false );

  printf("Getting inputs\n");

  auto h_impl_content = get_object(&client, bucket, "function-impl.h");
  auto h_content = get_object(&client, bucket, "function.h");
  auto c_content = get_object(&client, bucket, "function" + to_string( index ) + ".c" );

  printf("Loading deps\n");

  std::vector<NullTerminatedReadOnlyFile> system_dep_files;
  std::vector<char*> system_dep_content;
  for ( auto system_dep_path : system_deps ) {
    system_dep_files.push_back( NullTerminatedReadOnlyFile( &system_dep_path[1] ) );
    system_dep_content.push_back( system_dep_files.back().addr() );
  }
  std::vector<NullTerminatedReadOnlyFile> clang_dep_files;
  std::vector<char*> clang_dep_content;
  for ( auto clang_dep_path : clang_deps ) {
    clang_dep_files.push_back( NullTerminatedReadOnlyFile( &clang_dep_path[1] ) );
    clang_dep_content.push_back( clang_dep_files.back().addr() );
  }

  printf("Clanging\n");

  pair<bool, string> elf_res
    = c_to_elf( system_dep_content, clang_dep_content, c_content.data(), h_impl_content.data(), h_content.data() );

  if ( not elf_res.first ) {
    fprintf(stderr, "Error: clang %s\n", elf_res.second.c_str());
    return;
  }

  printf("Elf size %ld\n", elf_res.second.size());
  put_object( &client, bucket, "function" + to_string( index ) + ".o", elf_res.second );
  printf("{ \"msg\": \"Created ELF of size %ld\" }", elf_res.second.size());
}

// Dependency files packed within action
int main( int argc, char* argv[] )
{
  printf("This is an example log message from an arbitrary C program!\n");

  auto args = json::parse(argv[1]);
  auto bucket = args["bucket"].get<string>();
  auto index = args["index"].get<size_t>();

  printf("Input bucket %s, index %ld\n", bucket.c_str(), index);

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  { do_clang(bucket, index); }
  Aws::ShutdownAPI(options);
}
