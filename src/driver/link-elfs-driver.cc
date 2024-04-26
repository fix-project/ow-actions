#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <string>

#include "nlohmann/json.hpp"

#include "link-elfs.hh"
#include "s3.hh"

using namespace std;
using json = nlohmann::json;

void do_link( string bucket, size_t last_index, string output_name ) {
  // Credential: minioadmin, minioadmin
  const char* key = "minioadmin";
  Aws::Auth::AWSCredentials credential( key, key );

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = "172.31.8.132:9000";
  config.verifySSL = false;

  Aws::S3::S3Client client( credential, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false );

  vector<string> dep_files;
  dep_files.resize( last_index );
  vector<char*> dep_file_ptrs;
  dep_file_ptrs.resize( last_index );
  vector<size_t> dep_file_sizes;
  dep_file_sizes.resize( last_index );

  for ( size_t i = 0; i < last_index; i++ ) {
    dep_files[last_index] = get_object(&client, bucket, "function" + to_string( i ) + ".o" );
    dep_file_ptrs[last_index] = dep_files[last_index].data();
    dep_file_sizes[last_index] = dep_files[last_index].size();
  }

  auto [success, res] = link_elfs( dep_file_ptrs, dep_file_sizes );

  if ( not success ) {
    printf("{ \"msg\": \"Error: wasm2c\", \"error\": %s }", res.c_str() );
    exit( -1 );
  }

  put_object(&client, bucket, output_name, res );
}

int main( int argc, char* argv[] )
{
  auto args = json::parse( argv[1] );
  auto bucket = args["bucket"].get<string>();
  auto last_index = args["last_index"].get<size_t>();
  auto output_name = args["output_name"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI( options );
  {
    do_link( bucket, last_index, output_name );
  }
  Aws::ShutdownAPI( options );
  return 0;
}
