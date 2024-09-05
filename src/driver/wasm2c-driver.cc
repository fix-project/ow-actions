#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"
#include "aws/core/Aws.h"

#include "nlohmann/json.hpp"

#include "wasm-to-c.hh"
#include "s3.hh"

#include <atomic>
#include <functional>
#include <string>

using namespace std;
using json = nlohmann::json;

size_t generated_files = 0;
async_context s3_context;

void write_c_output( Aws::S3::S3Client* client, string output_bucket, size_t index, string content )
{
  if ( content.rfind( "/* Empty wasm2c", 0 ) == 0 ) {
    return;
  }

  put_object_async(&s3_context, client, output_bucket,
                   "function" + to_string(generated_files) + ".c", content);
  generated_files++;
}

string get_wasm_input( Aws::S3::S3Client* client, string input_bucket, string file_name )
{
  return get_object( client, input_bucket, file_name );
}

void do_wasm_to_c(string input_bucket, string file_name, string output_bucket,
                  string minio_url) {
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

  auto wasm_content = get_wasm_input( &client, input_bucket, file_name );

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec );

  auto stream_finish_callback = bind( write_c_output, &client, output_bucket, placeholders::_1, placeholders::_2 );

  auto [h_header, h_impl_header, errors]
    = wasm_to_c( wasm_content.data(), wasm_content.length(), stream_finish_callback );

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld Outputting\n", now.tv_sec, now.tv_nsec );

  if ( errors ) {
    printf("{ \"msg\": \"Error: wasm2c\", \"error\": %s }", errors->c_str()  );
    exit( -1 );
  }

  put_object_async(&s3_context, &client, output_bucket, "function-impl.h",
                   h_impl_header);
  put_object_async(&s3_context, &client, output_bucket, "function.h", h_header);

  unique_lock lk(s3_context.mutex);
  s3_context.cv.wait(lk, [&] { return s3_context.remaining_jobs == 0; });

  clock_gettime( CLOCK_REALTIME, &now );
  printf("%ld.%.9ld End\n", now.tv_sec, now.tv_nsec );

  printf("{ \"output_number\": %zu }", generated_files );
}

int main( int argc, char* argv[] )
{
  auto args = json::parse( argv[1] );
  auto input_bucket = args["input_bucket"].get<string>();
  auto input_file = args["input_file"].get<string>();
  auto output_bucket = args["output_bucket"].get<string>();
  auto minio_url = args["minio_url"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI( options );
  { do_wasm_to_c(input_bucket, input_file, output_bucket, minio_url); }
  Aws::ShutdownAPI( options );
  return 0;
}
