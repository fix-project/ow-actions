#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"

#include "nlohmann/json.hpp"

#include "s3.hh"

#include <functional>
#include <string>

using namespace std;
using json = nlohmann::json;

async_context s3_context;

pair<char *, size_t> merge_counts(size_t, char *, size_t, char *);

void do_merge_counts(string input_bucket, string fnX, string fnY,
                     string output_bucket, string output_file) {
  // Credential: minioadmin, minioadmin
  const char *key = "minioadmin";
  Aws::Auth::AWSCredentials credential(key, key);

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = "10.105.249.111:80";
  config.verifySSL = false;

  Aws::S3::S3Client client(
      credential, config,
      Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

  string fX;
  string fY;

  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Inputting\n", now.tv_sec, now.tv_nsec);

  get_object_async(&s3_context, &fX, &client, input_bucket, fnX);
  get_object_async(&s3_context, &fY, &client, input_bucket, fnY);

  unique_lock lk(s3_context.mutex);
  s3_context.cv.wait(lk, [&] { return s3_context.remaining_jobs == 0; });

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec);

  uint64_t *fX64 = (uint64_t *)(fX.data());
  uint64_t *fY64 = (uint64_t *)(fY.data());

  uint64_t f[256];
  for (int i = 0; i < 256; i++) {
    f[i] = fX64[i] + fY64[i];
  }

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Outputting\n", now.tv_sec, now.tv_nsec);

  put_object(&client, output_bucket, output_file, {(char *)(&f), sizeof(f)});

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld End\n", now.tv_sec, now.tv_nsec);
  printf("{ \"output_size\": %zu }", sizeof(f));
}

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_bucket = args["input_bucket"].get<string>();
  auto input_file_x = args["input_file_x"].get<string>();
  auto input_file_y = args["input_file_y"].get<string>();
  auto output_bucket = args["output_bucket"].get<string>();
  auto output_file = args["output_file"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    do_merge_counts(input_bucket, input_file_x, input_file_y, output_bucket,
                    output_file);
  }
  Aws::ShutdownAPI(options);
  return 0;
}
