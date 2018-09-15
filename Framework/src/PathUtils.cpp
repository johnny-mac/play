#include "PathUtils.h"
#include "Utf8.h"

using namespace Framework;

#ifdef _WIN32

#if WINAPI_FAMILY_PARTITION(WINAPI_FAMILY_APP)

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	auto localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	return boost::filesystem::path(localFolder->Path->Data());
}

#else	// !WINAPI_PARTITION_APP

#include <shlobj.h>

boost::filesystem::path PathUtils::GetPathFromCsidl(int csidl)
{
	wchar_t userPathString[MAX_PATH];
	if(FAILED(SHGetFolderPathW(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, userPathString)))
	{
		throw std::runtime_error("Couldn't get path from csidl.");
	}
	return boost::filesystem::wpath(userPathString, boost::filesystem::native);
}

boost::filesystem::path PathUtils::GetRoamingDataPath()
{
	return GetPathFromCsidl(CSIDL_APPDATA);
}

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	return GetPathFromCsidl(CSIDL_PERSONAL);
}

boost::filesystem::path PathUtils::GetAppResourcesPath()
{
	return boost::filesystem::path(".");
}

boost::filesystem::path PathUtils::GetCachePath()
{
	return GetPathFromCsidl(CSIDL_LOCAL_APPDATA);
}

#endif	// !WINAPI_PARTITION_APP

#endif	// _WIN32

#if defined(_POSIX_VERSION)

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

boost::filesystem::path PathUtils::GetRoamingDataPath()
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	std::string directory = [[paths objectAtIndex: 0] UTF8String];
	return boost::filesystem::path(directory);
}

boost::filesystem::path PathUtils::GetAppResourcesPath()
{
	NSBundle* bundle = [NSBundle mainBundle];
	NSString* bundlePath = [bundle resourcePath];
	return boost::filesystem::path([bundlePath UTF8String]);
}

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	return GetRoamingDataPath();
}

boost::filesystem::path PathUtils::GetCachePath()
{
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
	std::string directory = [[paths objectAtIndex: 0] UTF8String];
	return boost::filesystem::path(directory);
}

#elif defined(__ANDROID__)

static boost::filesystem::path s_filesDirPath;

boost::filesystem::path PathUtils::GetAppResourcesPath()
{
	//This won't work for Android
	return boost::filesystem::path();
}

boost::filesystem::path PathUtils::GetRoamingDataPath()
{
	return s_filesDirPath;
}

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	return s_filesDirPath;
}

boost::filesystem::path PathUtils::GetCachePath()
{
	throw std::runtime_error("Not implemented.");
}

void PathUtils::SetFilesDirPath(const char* filesDirPath)
{
	s_filesDirPath = filesDirPath;
}

#elif defined(__linux__) || defined(__FreeBSD__)

// TODO: is this an appropriate translation?
boost::filesystem::path PathUtils::GetAppResourcesPath()
{
	return boost::filesystem::path(getenv("HOME")) / ".local/share";
}

boost::filesystem::path PathUtils::GetRoamingDataPath()
{
	return boost::filesystem::path(getenv("HOME")) / ".local/share";
}

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	return boost::filesystem::path(getenv("HOME")) / ".local/share";
}

boost::filesystem::path PathUtils::GetCachePath()
{
	return boost::filesystem::path(getenv("HOME")) / ".cache";
}

#else	// !DEFINED(__ANDROID__) || !DEFINED(__APPLE__) || !DEFINED(__linux__) || !DEFINED(__FreeBSD__)

#include <pwd.h>

boost::filesystem::path PathUtils::GetPersonalDataPath()
{
	passwd* userInfo = getpwuid(getuid());
	return boost::filesystem::path(userInfo->pw_dir);
}

#endif	// !DEFINED(__APPLE__)

#endif	// !DEFINED(_POSIX_VERSION)

void PathUtils::EnsurePathExists(const boost::filesystem::path& path)
{
	typedef boost::filesystem::path PathType;
	PathType buildPath;
	for(PathType::iterator pathIterator(path.begin());
		pathIterator != path.end(); pathIterator++)
	{
		buildPath /= (*pathIterator);
		boost::system::error_code existsErrorCode;
		bool exists = boost::filesystem::exists(buildPath, existsErrorCode);
		if(existsErrorCode)
		{
#ifdef _WIN32
			if(existsErrorCode.value() == ERROR_ACCESS_DENIED)
			{
				//No problem, it exists, but we just don't have access
				continue;
			}
			else if(existsErrorCode.value() == ERROR_FILE_NOT_FOUND)
			{
				exists = false;
			}
#else
			if(existsErrorCode.value() == ENOENT)
			{
				exists = false;
			}
#endif
			else
			{
				throw std::runtime_error("Couldn't ensure that path exists.");
			}
		}
		if(!exists)
		{
			boost::filesystem::create_directory(buildPath);
		}
	}
}

////////////////////////////////////////////
//NativeString <-> Path Function Utils
////////////////////////////////////////////

template <typename StringType>
static std::string GetNativeStringFromPathInternal(const StringType&);

template <>
std::string GetNativeStringFromPathInternal(const std::string& str)
{
	return str;
}

template <>
std::string GetNativeStringFromPathInternal(const std::wstring& str)
{
	return Framework::Utf8::ConvertTo(str);
}

template <typename StringType>
static StringType GetPathFromNativeStringInternal(const std::string&);

template <>
std::string GetPathFromNativeStringInternal(const std::string& str)
{
	return str;
}

template <>
std::wstring GetPathFromNativeStringInternal(const std::string& str)
{
	return Framework::Utf8::ConvertFrom(str);
}

////////////////////////////////////////////
//NativeString <-> Path Function Implementations
////////////////////////////////////////////

std::string PathUtils::GetNativeStringFromPath(const boost::filesystem::path& path)
{
	return GetNativeStringFromPathInternal(path.native());
}

boost::filesystem::path PathUtils::GetPathFromNativeString(const std::string& str)
{
	auto cvtStr = GetPathFromNativeStringInternal<boost::filesystem::path::string_type>(str);
	return boost::filesystem::path(cvtStr);
}
