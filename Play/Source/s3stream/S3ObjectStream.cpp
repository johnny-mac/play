#include <cassert>
#include <cstring>
#include "S3ObjectStream.h"
#include "AmazonS3Client.h"
#include "Singleton.h"
#include "AppConfig.h"
#include "PathUtils.h"
#include "string_format.h"
#include "StdStreamUtils.h"
#include "Log.h"

#define PREF_S3_OBJECTSTREAM_ACCESSKEYID "s3.objectstream.accesskeyid"
#define PREF_S3_OBJECTSTREAM_SECRETACCESSKEY "s3.objectstream.secretaccesskey"
#define CACHE_PATH "Play Data Files/s3objectstream_cache"

#define LOG_NAME "s3objectstream"

class CS3Config : public CSingleton<CS3Config>
{
public:
	CS3Config()
	{
		CAppConfig::GetInstance().RegisterPreferenceString(PREF_S3_OBJECTSTREAM_ACCESSKEYID, "");
		CAppConfig::GetInstance().RegisterPreferenceString(PREF_S3_OBJECTSTREAM_SECRETACCESSKEY, "");
	}

	std::string GetAccessKeyId()
	{
		return CAppConfig::GetInstance().GetPreferenceString(PREF_S3_OBJECTSTREAM_ACCESSKEYID);
	}

	std::string GetSecretAccessKey()
	{
		return CAppConfig::GetInstance().GetPreferenceString(PREF_S3_OBJECTSTREAM_SECRETACCESSKEY);
	}
};

CS3ObjectStream::CS3ObjectStream(const char* bucketName, const char* objectName)
    : m_bucketName(bucketName)
    , m_objectName(objectName)
{
	Framework::PathUtils::EnsurePathExists(GetCachePath());
	GetObjectInfo();
}

uint64 CS3ObjectStream::Read(void* buffer, uint64 size)
{
	auto range = std::make_pair(m_objectPosition, m_objectPosition + size - 1);
	auto readCacheFilePath = GetCachePath() / GenerateReadCacheKey(range);

#ifdef _TRACEGET
	static FILE* output = fopen("getobject.log", "wb");
	fprintf(output, "%ld,%ld,%ld\r\n", range.first, range.second, size);
	fflush(output);
#endif

	bool cachedReadSucceeded =
	    [&]() {
		    try
		    {
			    if(boost::filesystem::exists(readCacheFilePath))
			    {
				    auto readCacheFileStream = Framework::CreateInputStdStream(readCacheFilePath.native());
				    auto cacheRead = readCacheFileStream.Read(buffer, size);
				    assert(cacheRead == size);
				    return true;
			    }
		    }
		    catch(const std::exception& exception)
		    {
			    //Not a problem if we failed to read cache
			    CLog::GetInstance().Print(LOG_NAME, "Failed to read cache: '%s'.\r\n", exception.what());
		    }
		    return false;
	    }();

	if(!cachedReadSucceeded)
	{
		assert(size > 0);
		CAmazonS3Client client(CS3Config::GetInstance().GetAccessKeyId(), CS3Config::GetInstance().GetSecretAccessKey(), m_bucketRegion);
		GetObjectRequest request;
		request.object = m_objectName;
		request.bucket = m_bucketName;
		request.range = range;
		auto objectContent = client.GetObject(request);
		assert(objectContent.data.size() == size);
		memcpy(buffer, objectContent.data.data(), size);

		try
		{
			auto readCacheFileStream = Framework::CreateOutputStdStream(readCacheFilePath.native());
			readCacheFileStream.Write(objectContent.data.data(), size);
		}
		catch(const std::exception& exception)
		{
			//Not a problem if we failed to write cache
			CLog::GetInstance().Print(LOG_NAME, "Failed to write cache: '%s'.\r\n", exception.what());
		}
	}

	m_objectPosition += size;
	return size;
}

uint64 CS3ObjectStream::Write(const void*, uint64)
{
	throw std::runtime_error("Not supported.");
}

void CS3ObjectStream::Seek(int64 offset, Framework::STREAM_SEEK_DIRECTION direction)
{
	switch(direction)
	{
	case Framework::STREAM_SEEK_SET:
		assert(offset <= m_objectSize);
		m_objectPosition = offset;
		break;
	case Framework::STREAM_SEEK_CUR:
		m_objectPosition += offset;
		break;
	case Framework::STREAM_SEEK_END:
		m_objectPosition = m_objectSize;
		break;
	}
}

uint64 CS3ObjectStream::Tell()
{
	return m_objectPosition;
}

bool CS3ObjectStream::IsEOF()
{
	return (m_objectPosition == m_objectSize);
}

boost::filesystem::path CS3ObjectStream::GetCachePath()
{
	return Framework::PathUtils::GetCachePath() / CACHE_PATH;
}

std::string CS3ObjectStream::GenerateReadCacheKey(const std::pair<uint64, uint64>& range) const
{
	return string_format("%s-%llu-%llu", m_objectEtag.c_str(), range.first, range.second);
}

static std::string TrimQuotes(std::string input)
{
	if(input.empty()) return input;
	if(input[0] == '"')
	{
		input = std::string(input.begin() + 1, input.end());
	}
	if(input.empty()) return input;
	if(input[input.size() - 1] == '"')
	{
		input = std::string(input.begin(), input.end() - 1);
	}
	return input;
}

void CS3ObjectStream::GetObjectInfo()
{
	//Obtain bucket region
	{
		CAmazonS3Client client(CS3Config::GetInstance().GetAccessKeyId(), CS3Config::GetInstance().GetSecretAccessKey());

		GetBucketLocationRequest request;
		request.bucket = m_bucketName;

		auto result = client.GetBucketLocation(request);
		m_bucketRegion = result.locationConstraint;
	}

	//Obtain object info
	{
		CAmazonS3Client client(CS3Config::GetInstance().GetAccessKeyId(), CS3Config::GetInstance().GetSecretAccessKey(), m_bucketRegion);

		HeadObjectRequest request;
		request.bucket = m_bucketName;
		request.object = m_objectName;

		auto objectHeader = client.HeadObject(request);
		m_objectSize = objectHeader.contentLength;
		m_objectEtag = TrimQuotes(objectHeader.etag);
	}
}
