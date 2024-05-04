#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"

#include "nlohmann/json.hpp"

#include "s3.hh"

#include <functional>
#include <string>

using namespace std;
using json = nlohmann::json;

pair<char *, size_t> count_words(size_t, char *);

string get_input(Aws::S3::S3Client *client, string input_bucket,
                 string file_name) {
  return get_object(client, input_bucket, file_name);
}

void do_count_words(string input_bucket, string file_name, string output_bucket,
                    string output_file) {
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

  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Inputting\n", now.tv_sec, now.tv_nsec);

  auto input_content = get_input(&client, input_bucket, file_name);

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Starting real compute\n", now.tv_sec, now.tv_nsec);

  auto [out, out_size] =
      count_words(input_content.size(), input_content.data());

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld Outputting\n", now.tv_sec, now.tv_nsec);

  put_object(&client, output_bucket, output_file, {out, out_size});
  free(out);

  clock_gettime(CLOCK_REALTIME, &now);
  printf("%ld.%.9ld End\n", now.tv_sec, now.tv_nsec);
  printf("{ \"output_size\": %zu }", out_size);
}

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_bucket = args["input_bucket"].get<string>();
  auto input_file = args["input_file"].get<string>();
  auto output_bucket = args["output_bucket"].get<string>();
  auto output_file = args["output_file"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  { do_count_words(input_bucket, input_file, output_bucket, output_file); }
  Aws::ShutdownAPI(options);
  return 0;
}
