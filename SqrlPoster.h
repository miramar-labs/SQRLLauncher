#pragma once

#include <winhttp.h>
#include <string>
using namespace std;

#include <cpprest/json.h>
using namespace utility;

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
	pplx::task<void> SqrlPoster::RequestJSONValueAsync(utility::string_t sFile);
	std::wstring UploadPDF(bool* errorFound);
	int doPost(const HINTERNET *request);

private:
	std::wstring FullPath, Drive, Directory, FileName, Extension, FileNamePlusExt;
};

