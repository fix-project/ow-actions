#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <ctime>

#include "mmap.hh"
#include "nlohmann/json.hpp"

#include "c-to-elf.hh"
#include "depfile.hh"
#include "s3.hh"

using namespace std;
using json = nlohmann::json;

void do_clang(string bucket, size_t index, string minio_url) {
  // Credential: minioadmin, minioadmin
  const char* key = "minioadmin";
  Aws::Auth::AWSCredentials credential( key, key );

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = minio_url;
  config.verifySSL = false;

  Aws::S3::S3Client client( credential, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false );

  struct timespec now;
  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld Inputting\n", now.tv_sec, now.tv_nsec );

  auto c_content = get_object(&client, bucket, "function" + to_string( index ) + ".c" );
  auto h_impl_content = get_object(&client, bucket, "function-impl.h");
  auto h_content = get_object(&client, bucket, "function.h");

  vector<string> system_dep_content;
  for ( auto system_dep_path : system_deps ) {
    system_dep_content.push_back( get_object( &client, "system-deps", string( &system_dep_path[1] ) ) );
  }
  vector<string> clang_dep_content;
  for ( auto clang_dep_path : clang_deps ) {
    clang_dep_content.push_back( get_object( &client, "clang-deps", string( &clang_dep_path[1] ) ) );
  }

  vector<char*> system_dep_ptrs;
  for ( size_t i = 0; i < system_dep_content.size(); i++ ) {
    system_dep_ptrs.push_back( system_dep_content[i].data() );
  }
  vector<char*> clang_dep_ptrs;
  for ( size_t i = 0; i < clang_dep_content.size(); i++ ) {
    clang_dep_ptrs.push_back( clang_dep_content[i].data() );
  }

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec );

  pair<bool, string> elf_res
    = c_to_elf( system_dep_ptrs, clang_dep_ptrs, c_content.data(), h_impl_content.data(), h_content.data() );

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld Outputting\n", now.tv_sec, now.tv_nsec );

  if ( not elf_res.first ) {
    fprintf(stderr, "Error: clang %s\n", elf_res.second.c_str());
    return;
  }

  put_object( &client, bucket, "function" + to_string( index ) + ".o", elf_res.second );

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld End\n", now.tv_sec, now.tv_nsec );

  printf("{ \"msg\": \"Created ELF of size %ld\" }", elf_res.second.size());
}

// Dependency files packed within action
int main( int argc, char* argv[] )
{
  auto args = json::parse(argv[1]);
  auto bucket = args["bucket"].get<string>();
  auto index = args["index"].get<size_t>();
  auto minio_url = args["minio_url"].get<string>();

  printf("Input bucket %s, index %ld\n", bucket.c_str(), index);

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  { do_clang(bucket, index, minio_url); }
  Aws::ShutdownAPI(options);
}
