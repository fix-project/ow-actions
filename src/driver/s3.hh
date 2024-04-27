#include "aws/s3/S3Client.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/PutObjectRequest.h"

static void put_object(Aws::S3::S3Client *client, std::string bucket,
                       std::string key, std::string_view content) {
  const std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::StringStream>( "" );

  inputData->write(content.data(), content.size());

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket( bucket );
  request.SetKey( key );
  request.SetBody( inputData );

  Aws::S3::Model::PutObjectOutcome outcome = client->PutObject(request);

  if (!outcome.IsSuccess()) {
    fprintf(stderr, "Error: PutObjectBuffer: %s, %s", key.c_str(),
            outcome.GetError().GetMessage().c_str());
    exit( -1 );
  }
}

static std::string get_object(Aws::S3::S3Client *client, std::string bucket,
                              std::string key) {
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket( bucket );
  request.SetKey( key );

  auto outcome = client->GetObject( request );

  if (!outcome.IsSuccess()) {
    printf("{ \"msg\": \"Error: GetObject: %s\", \"error\": %s }", key.c_str(), outcome.GetError().GetMessage().c_str() );
    exit( -1 );
  } else {
    auto& content = outcome.GetResult().GetBody();
    std::stringstream ss;
    ss << content.rdbuf();
    return ss.str();
  }
}

static void put_object_async_finished(
    std::atomic<size_t> *counter, const Aws::S3::S3Client *s3Client,
    const Aws::S3::Model::PutObjectRequest &request,
    const Aws::S3::Model::PutObjectOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
  if (!outcome.IsSuccess()) {
    std::cerr << "Error: PutObjectAsyncFinished: "
              << outcome.GetError().GetMessage() << std::endl;
    exit(-1);
  }

  *counter--;
  counter->notify_all();
}

static void put_object_async(std::atomic<size_t> *counter,
                             const Aws::S3::S3Client &s3Client,
                             std::string bucket, std::string key,
                             std::string_view content) {
  const std::shared_ptr<Aws::IOStream> inputData =
      Aws::MakeShared<Aws::StringStream>("");

  inputData->write(content.data(), content.size());

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(bucket);
  request.SetKey(key);
  request.SetBody(inputData);

  std::shared_ptr<Aws::Client::AsyncCallerContext> context =
      Aws::MakeShared<Aws::Client::AsyncCallerContext>(key.c_str());

  *counter++;

  s3Client.PutObjectAsync(
      request,
      std::bind(put_object_async_finished, counter, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3,
                std::placeholders::_4),
      context);
}

static void get_object_async_finished(
    std::atomic<size_t> *counter, std::string *dst,
    const Aws::S3::S3Client *s3Client,
    const Aws::S3::Model::GetObjectRequest &request,
    const Aws::S3::Model::GetObjectOutcome outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
  if (!outcome.IsSuccess()) {
    std::cerr << "Error: GetObjectAsyncFinished: "
              << outcome.GetError().GetMessage() << std::endl;
    exit(-1);
  }

  auto &content = outcome.GetResult().GetBody();
  std::stringstream ss;
  ss << content.rdbuf();
  *dst = ss.str();

  *counter--;
  counter->notify_all();
}

static void get_object_async(std::atomic<size_t> *counter, std::string *dst,
                             Aws::S3::S3Client *client, std::string bucket,
                             std::string key) {
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(bucket);
  request.SetKey(key);

  std::shared_ptr<Aws::Client::AsyncCallerContext> context =
      Aws::MakeShared<Aws::Client::AsyncCallerContext>(key.c_str());

  *counter++;

  client->GetObjectAsync(request,
                         std::bind(get_object_async_finished, counter, dst,
                                   std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4),
                         context);
}
