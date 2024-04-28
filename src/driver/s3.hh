#include "aws/s3/S3Client.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/PutObjectRequest.h"

#include <condition_variable>
#include <mutex>

struct async_context {
  std::size_t remaining_jobs{};
  std::mutex mutex{};
  std::condition_variable cv{};
};

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
    async_context *async, const Aws::S3::S3Client *s3Client,
    const Aws::S3::Model::PutObjectRequest &request,
    const Aws::S3::Model::PutObjectOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
  if (!outcome.IsSuccess()) {
    std::cerr << "Error: PutObjectAsyncFinished: "
              << outcome.GetError().GetMessage() << std::endl;
    exit(-1);
  }

  {
    std::unique_lock lock(async->mutex);
    async->remaining_jobs--;
    if (async->remaining_jobs == 0) {
      async->cv.notify_all();
    }
  }
}

static void put_object_async(async_context *async, Aws::S3::S3Client *s3Client,
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

  {
    std::unique_lock lock(async->mutex);
    async->remaining_jobs++;
  }

  s3Client->PutObjectAsync(
      request,
      std::bind(put_object_async_finished, async, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3,
                std::placeholders::_4),
      context);
}

static void get_object_async_finished(
    async_context *async, std::string *dst, const Aws::S3::S3Client *s3Client,
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

  {
    std::unique_lock lock(async->mutex);
    async->remaining_jobs--;
    if (async->remaining_jobs == 0) {
      async->cv.notify_all();
    }
  }
}

static void get_object_async(async_context *async, std::string *dst,
                             Aws::S3::S3Client *client, std::string bucket,
                             std::string key) {
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(bucket);
  request.SetKey(key);

  std::shared_ptr<Aws::Client::AsyncCallerContext> context =
      Aws::MakeShared<Aws::Client::AsyncCallerContext>(key.c_str());

  {
    std::unique_lock lock(async->mutex);
    async->remaining_jobs++;
  }

  client->GetObjectAsync(request,
                         std::bind(get_object_async_finished, async, dst,
                                   std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4),
                         context);
}
