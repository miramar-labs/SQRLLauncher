#pragma once

#include <winhttp.h>
#include <string>
using namespace std;

#include <cpprest/json.h>
#include <cpprest/http_client.h>
using namespace web::http::client;
using namespace utility;

class SqrlPoster
{
public:
	SqrlPoster(const std::wstring& FullPath);
	~SqrlPoster();

	std::wstring doPOST(bool* errorFound);	

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
	pplx::task<void>	GetUploadInfo(utility::string_t sFile, bool* errorFound);
	std::wstring		UploadPDF(bool* errorFound);
	pplx::task<void>	DeletePDF(bool* errorFound);
	void				doHTTPPost(const HINTERNET *request, bool* errorFound);

private:
	std::wstring FullPath, Drive, Directory, FileName, Extension, FileNamePlusExt;
	std::wstring iniFile;

	std::wstring sqrlhttps;
	std::wstring sqrlendpoint;
	std::wstring sqrlendpoint2;
	std::wstring bucketurl;

};

