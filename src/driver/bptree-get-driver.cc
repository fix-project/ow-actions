#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"

#include "nlohmann/json.hpp"

#include "base16.hh"
#include "s3.hh"

#include <string>
#include <bitset>

using namespace std;
using json = nlohmann::json;

string get_fix_object( Aws::S3::S3Client* client, string input_bucket, string handle ) {
  auto raw = base16::decode( handle );
  return get_object( client, input_bucket, handle.substr( 0, 48 ) );
}

string get_entry( string data, size_t i ) {
  string result = base16::encode( data.substr( i * 32, 32 ) );
  return result;
}

int upper_bound( int32_t* keys, uint64_t size, int key )
{
  for ( int i = 0; i < size; i++ ) {
    if ( keys[i] > key ) {
      return i;
    }
  }

  return size;
}

void do_bptree_get(string input_bucket, string tree_root, string minio_url, int key, string output_bucket, string output_file) {
  // Credential: minioadmin, minioadmin
  const char *minio_key = "minioadmin";
  Aws::Auth::AWSCredentials credential(minio_key, minio_key);

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = minio_url;
  config.verifySSL = false;

  Aws::S3::S3Client client(
      credential, config,
      Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

  string curr_level = tree_root;
  string result;

  while ( true ) {
    auto data = get_fix_object( &client, input_bucket, curr_level );
    auto keys = get_fix_object( &client, input_bucket, get_entry( data, 0 ) );
    bool isleaf = ( keys[0] == 1 );
    int32_t* real_keys = reinterpret_cast<int32_t*>( &keys[1] );


    auto idx = upper_bound( real_keys, keys.size() / sizeof( int ), key );

    if ( isleaf ) {
      if ( idx != 0 and real_keys[idx - 1] == key ) {
        result = get_fix_object( &client, input_bucket, get_entry( data, idx ) );
      } else {
        result = "Not found";
      }
      break;
    } else {
      curr_level = get_entry( data, idx + 1 );
    }
  }

  put_object(&client, output_bucket, output_file, result );
  printf( "{ \"result\": \"%s\" }", result.c_str() );
}

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_bucket = args["input_bucket"].get<string>();
  auto tree_root = args["tree_root"].get<string>();
  auto minio_url = args["minio_url"].get<string>();
  auto key = args["key"].get<int>();
  auto output_bucket = args["output_bucket"].get<string>();
  auto output_file = args["output_file"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  { do_bptree_get(input_bucket, tree_root, minio_url, key, output_bucket, output_file); }
  Aws::ShutdownAPI(options);
  return 0;
}
