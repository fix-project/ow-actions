#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <string>

#include "nlohmann/json.hpp"

#include "link-elfs.hh"
#include "s3.hh"

using namespace std;
using json = nlohmann::json;

async_context s3_context;

void do_link(string bucket, size_t last_index, string output_name,
             string minio_url, bool logging) {
  if (logging) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    printf("%ld.%.9ld Inputting\n", now.tv_sec, now.tv_nsec);
  }

  // Credential: minioadmin, minioadmin
  const char* key = "minioadmin";
  Aws::Auth::AWSCredentials credential( key, key );

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = minio_url;
  config.verifySSL = false;

  Aws::S3::S3Client client( credential, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false );

  vector<string> dep_files;
  dep_files.resize(last_index + 1);
  vector<char*> dep_file_ptrs;
  dep_file_ptrs.resize(last_index + 1);
  vector<size_t> dep_file_sizes;
  dep_file_sizes.resize(last_index + 1);

  for (size_t i = 0; i <= last_index; i++) {
    get_object_async(&s3_context, &dep_files[i], &client, bucket,
                     "function" + to_string(i) + ".o");
  }

  unique_lock lk(s3_context.mutex);
  s3_context.cv.wait(lk, [&] { return s3_context.remaining_jobs == 0; });

  for (size_t i = 0; i <= last_index; i++) {
    dep_file_ptrs[i] = dep_files[i].data();
    dep_file_sizes[i] = dep_files[i].size();
  }

  if (logging) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec);
  }

  auto [success, res] = link_elfs( dep_file_ptrs, dep_file_sizes );

  if (logging) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    printf("%ld.%.9ld Outputting\n", now.tv_sec, now.tv_nsec);
  }

  if ( not success ) {
    fprintf(stderr, "Error: link elfs error: %s }", res.c_str());
    exit( -1 );
  }

  put_object(&client, bucket, output_name, res);

  if (logging) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    printf("%ld.%.9ld End\n", now.tv_sec, now.tv_nsec);
  }

  printf("{ \"msg\": \"Linked ELF of size %ld\" }", res.size());
}

int main( int argc, char* argv[] )
{
  auto args = json::parse( argv[1] );
  auto bucket = args["bucket"].get<string>();
  auto last_index = args["last_index"].get<size_t>();
  auto output_name = args["output_name"].get<string>();
  auto minio_url = args["minio_url"].get<string>();
  auto logging = args.value("logging", false);

  Aws::SDKOptions options;
  Aws::InitAPI( options );
  { do_link(bucket, last_index, output_name, minio_url, logging); }
  Aws::ShutdownAPI( options );
  return 0;
}
