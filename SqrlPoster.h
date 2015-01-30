#pragma once

#include <winhttp.h>
#include <string>
using namespace std;

#include <cpprest/json.h>
#include <cpprest/http_client.h>
using namespace web::http::client;
using namespace utility;

#define USE_WININET

class SqrlPoster
{
public:
	SqrlPoster(const std::wstring& FullPath);
	~SqrlPoster();

	std::wstring doPOST();	// does the upload and return URL to redirect to

private:
	string_t deliver_to;
	string_t delivered;
	string_t file_type;
	string_t name;
	string_t package_type;
	string_t size;
	string_t tracking_number;

	string_t AWSAccessKeyId;
	string_t Content_Type;
	string_t acl;
	string_t key;
	string_t policy;
	string_t signature;
	string_t success_action_status;

private:
	pplx::task<void> RequestJSONValueAsync(utility::string_t sFile);

#ifndef USE_WININET
	pplx::task<void> HTTPPostAsync(utility::string_t file);
#else
	std::wstring UploadPDF(bool* errorFound);
	int doPost(const HINTERNET *request);
#endif

private:
	std::wstring FullPath, Drive, Directory, FileName, Extension, FileNamePlusExt;
};

