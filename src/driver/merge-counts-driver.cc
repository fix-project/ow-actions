#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"

#include "nlohmann/json.hpp"

#include "s3.hh"

#include <functional>
#include <string>

using namespace std;
using json = nlohmann::json;

pair<char *, size_t> merge_counts(size_t, char *, size_t, char *);

string get_input(Aws::S3::S3Client *client, string input_bucket,
                 string file_name) {
  return get_object(client, input_bucket, file_name);
}

void do_merge_counts(string input_bucket, string fnX, string fnY,
                     string output_bucket, string output_file) {
  // Credential: minioadmin, minioadmin
  const char *key = "minioadmin";
  Aws::Auth::AWSCredentials credential(key, key);

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = "172.31.8.132:9000";
  config.verifySSL = false;

  Aws::S3::S3Client client(
      credential, config,
      Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

  auto fX = get_input(&client, input_bucket, fnX);
  auto fY = get_input(&client, input_bucket, fnY);

  printf("fnX: %s, fX.size: %zu\n", fnX.c_str(), fX.size());
  printf("fnY: %s, fY.size: %zu\n", fnY.c_str(), fY.size());

  auto [out, out_size] =
      merge_counts(fX.size(), fX.data(), fY.size(), fY.data());

  printf("output_size: %zu\n", out_size);

  put_object(&client, output_bucket, output_file, {out, out_size});
  free(out);

  printf("{ \"output_size\": %zu }", out_size);
}

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_bucket = args["input_bucket"].get<string>();
  auto input_file_x = args["input_file_x"].get<string>();
  auto input_file_y = args["input_file_y"].get<string>();
  auto output_bucket = args["output_bucket"].get<string>();
  auto output_file = args["output_file"].get<string>();

  printf("input_bucket: %s, input_file_x: %s, input_file_y: %s, output_bucket: %s, output_file: %s\n", input_bucket.c_str(), input_file_x.c_str(), input_file_y.c_str(), output_bucket.c_str(), output_file.c_str());

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    do_merge_counts(input_bucket, input_file_x, input_file_y, output_bucket,
                    output_file);
  }
  Aws::ShutdownAPI(options);
  return 0;
}
