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
  dep_files.resize(last_index + 1);
  vector<char*> dep_file_ptrs;
  dep_file_ptrs.resize(last_index + 1);
  vector<size_t> dep_file_sizes;
  dep_file_sizes.resize(last_index + 1);

  for (size_t i = 0; i <= last_index; i++) {
    dep_files[i] =
        get_object(&client, bucket, "function" + to_string(i) + ".o");
    dep_file_ptrs[i] = dep_files[i].data();
    dep_file_sizes[i] = dep_files[i].size();
    printf("function%ld.o size:%ld\n", i, dep_file_sizes[i]);
  }

  auto [success, res] = link_elfs( dep_file_ptrs, dep_file_sizes );

  if ( not success ) {
    fprintf(stderr, "Error: link elfs error: %s }", res.c_str());
    exit( -1 );
  }

  put_object(&client, bucket, output_name, res);
  printf("{ \"msg\": \"Linked ELF of size %ld\" }", res.size());
}

int main( int argc, char* argv[] )
{
  auto args = json::parse( argv[1] );
  auto bucket = args["bucket"].get<string>();
  auto last_index = args["last_index"].get<size_t>();
  auto output_name = args["output_name"].get<string>();
  printf("bucket: %s, last_index: %ld, output_name: %s\n", bucket.c_str(),
         last_index, output_name.c_str());

  Aws::SDKOptions options;
  Aws::InitAPI( options );
  {
    do_link( bucket, last_index, output_name );
  }
  Aws::ShutdownAPI( options );
  return 0;
}
