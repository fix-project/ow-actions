#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentials.h"
#include "aws/s3/S3Client.h"

#include "nlohmann/json.hpp"

#include "s3.hh"

#include <string>

using namespace std;
using json = nlohmann::json;

pair<char *, size_t> count_words(size_t, char *);

string get_input(Aws::S3::S3Client *client, string input_bucket,
                 string file_name) {
  return get_object(client, input_bucket, file_name);
}

void do_count_words(string input_bucket, string file_name, string minio_url,
                    string query) {
  // Credential: minioadmin, minioadmin
  const char *key = "minioadmin";
  Aws::Auth::AWSCredentials credential(key, key);

  Aws::Client::ClientConfiguration config;
  config.scheme = Aws::Http::Scheme::HTTP;
  config.endpointOverride = minio_url;
  config.verifySSL = false;

  Aws::S3::S3Client client(
      credential, config,
      Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

  auto input_content = get_input(&client, input_bucket, file_name);
  size_t haystack_size = input_content.size();
  char *haystack = input_content.data();

  size_t needle_size = query.size();
  char *needle = query.data();

  size_t count = 0;
  if (needle_size <= haystack_size) {
    for (size_t i = 0; i < haystack_size - needle_size + 1; i++) {
      if (!memcmp(needle, haystack + i, needle_size)) {
        count++;
        i += needle_size - 1;
      }
    }
  }

  printf("{ \"count\": %zu }", count);
}

int main(int argc, char *argv[]) {
  auto args = json::parse(argv[1]);
  auto input_bucket = args["input_bucket"].get<string>();
  auto input_file = args["input_file"].get<string>();
  auto minio_url = args["minio_url"].get<string>();
  auto needle = args["query"].get<string>();

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  { do_count_words(input_bucket, input_file, minio_url, needle); }
  Aws::ShutdownAPI(options);
  return 0;
}
