#include "aws/s3/S3Client.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/PutObjectRequest.h"

inline void put_object( Aws::S3::S3Client* client, std::string bucket, std::string key, const std::string& content ) {
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

inline std::string get_object( Aws::S3::S3Client* client, std::string bucket, std::string key )
{
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
