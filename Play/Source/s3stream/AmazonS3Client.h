#pragma once

#include <string>
#include <vector>
#include "http/HttpClient.h"

struct GetBucketLocationRequest
{
	std::string bucket;
};

struct GetBucketLocationResult
{
	std::string locationConstraint;
};

struct GetObjectRequest
{
	std::string bucket;
	std::string object;
	std::pair<uint64, uint64> range;
};

struct GetObjectResult
{
	std::vector<uint8> data;
};

struct HeadObjectRequest
{
	std::string bucket;
	std::string object;
};

struct HeadObjectResult
{
	uint64 contentLength = 0;
	std::string etag;
};

struct Object
{
	std::string key;
};

struct ListObjectsResult
{
	std::vector<Object> objects;
};

class CAmazonS3Client
{
public:
	CAmazonS3Client(std::string, std::string, std::string = "us-east-1");

	GetBucketLocationResult GetBucketLocation(const GetBucketLocationRequest&);
	GetObjectResult GetObject(const GetObjectRequest&);
	HeadObjectResult HeadObject(const HeadObjectRequest&);
	ListObjectsResult ListObjects(std::string);

private:
	struct Request
	{
		Framework::Http::HTTP_VERB method;
		std::string host;
		std::string urlHost;
		std::string uri;
		std::string query;
		Framework::Http::HeaderMap headers;
	};

	Framework::Http::RequestResult ExecuteRequest(const Request&);

	std::string m_accessKeyId;
	std::string m_secretAccessKey;
	std::string m_region;
};
