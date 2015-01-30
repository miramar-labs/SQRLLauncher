#include "stdafx.h"
#include "SqrlPoster.h"

using namespace ATL;

#include <winhttp.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <thread>
#include <cpprest/json.h>

using namespace concurrency::streams;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace utility;

void stripQuotes(utility::string_t& s){
	s.erase(remove(s.begin(), s.end(), '\"'), s.end());
}

SqrlPoster::SqrlPoster(const std::wstring& path)
{
	FullPath = path;

	Drive.resize(FullPath.size());
	Directory.resize(FullPath.size());
	FileName.resize(FullPath.size());
	Extension.resize(FullPath.size());

	errno_t Error = _wsplitpath_s(FullPath.c_str(),
		&Drive[0],
		Drive.size(),
		&Directory[0],
		Directory.size(),
		&FileName[0],
		FileName.size(),
		&Extension[0],
		Extension.size());

	wchar_t buff[MAX_PATH+1];
	memset(buff, 0, sizeof(buff));
	wcsncpy_s(buff, FileName.c_str(), sizeof(buff));
	wcsncat_s(buff, Extension.c_str(), sizeof(buff));

	FileNamePlusExt = buff;

}

SqrlPoster::~SqrlPoster()
{
}

std::wstring SqrlPoster::doPOST(){
	this->RequestJSONValueAsync(FullPath).wait();
	bool err = false;
	std::wstring url = UploadPDF(&err);
	//TODO error checking...
	return url;
}

//private:

std::ifstream::pos_type filesize(const std::wstring& filename)
{
	int stringSize = WideCharToMultiByte(CP_ACP, 0, filename.c_str(), -1, nullptr, 0, nullptr, nullptr);
	char* fname = new char[stringSize];
	WideCharToMultiByte(CP_ACP, 0, filename.c_str(), -1, fname, stringSize, nullptr, nullptr);
	std::ifstream in(fname, std::ifstream::ate | std::ifstream::binary);
	delete[] fname;
	return in.tellg();
}

pplx::task<void> SqrlPoster::RequestJSONValueAsync(utility::string_t sFile)
{
	http_client client(L"https://sqrl-development.herokuapp.com");

	json::value obj;
	obj[L"name"] = json::value::string(FileNamePlusExt);
	obj[L"file_type"] = json::value::string(U("application/pdf"));
	obj[L"size"] = json::value::number((int32_t)filesize(FullPath));

	const utility::string_t q(U("/api/v1/couriers"));

	return client.request(methods::POST, q, obj).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			const json::value& v = previousTask.get();

			for (auto iter = v.as_object().cbegin(); iter != v.as_object().cend(); ++iter) {

				const utility::string_t &str = iter->first;
				const json::value &v = iter->second;

				if (str == U("deliver_to")){
					deliver_to = v.serialize(); stripQuotes(deliver_to);
				}
				else if (str == U("delivered")){
					delivered = v.serialize(); stripQuotes(delivered);
				}
				else if (str == U("file_type")){
					file_type = v.serialize(); stripQuotes(file_type);
				}
				else if (str == U("name")){
					name = v.serialize(); stripQuotes(name);
				}
				else if (str == U("package_type")){
					package_type = v.serialize(); stripQuotes(package_type);
				}
				else if (str == U("size")){
					size = v.serialize(); stripQuotes(size);
				}
				else if (str == U("tracking_number")){
					tracking_number = v.serialize(); stripQuotes(tracking_number);
				}
				else if (str == U("form_data")){
					
					for (auto iter = v.as_object().cbegin(); iter != v.as_object().cend(); ++iter) {

						const utility::string_t &str = iter->first;
						const json::value &v = iter->second;

						if (str == U("AWSAccessKeyId")){ 
							AWSAccessKeyId = v.serialize(); stripQuotes(AWSAccessKeyId);
						}
						else if (str == U("Content-Type")){
							Content_Type = v.serialize(); stripQuotes(Content_Type);
						}
						else if (str == U("acl")){
							acl = v.serialize(); stripQuotes(acl);
						}
						else if (str == U("key")){
							key = v.serialize(); stripQuotes(key);
							std::cout << "key=" << key.c_str() << std::endl;
						}
						else if (str == U("policy")){
							policy = v.serialize(); stripQuotes(policy);
						}
						else if (str == U("signature")){
							signature = v.serialize(); stripQuotes(signature);
						}
						else if (str == U("success_action_status")){
							success_action_status = v.serialize(); stripQuotes(success_action_status);
						}
					}
				}
				//std::wcout << L"String: " << str.c_str() << L", Value: " << v.serialize() << std::endl
			}

		}
		catch (const http_exception& e)
		{
			// TODO - Print error.
			//wostringstream ss;
			//ss << e.what() << endl;
			//wcout << ss.str();
		}
	});

	/* Output:
	Content-Type must be application/json to extract (is: text/html)
	*/
}

std::wstring GetPDFId(const HINTERNET *request, bool* errorFound)
{
	std::wstring outputString;
	int result = ::WinHttpReceiveResponse(*request, nullptr);
	unsigned long dwSize = sizeof(unsigned long);
	if (result)
	{
		wchar_t headers[1024];
		dwSize = ARRAYSIZE(headers) * sizeof(wchar_t);
		result = ::WinHttpQueryHeaders(
			*request, WINHTTP_QUERY_RAW_HEADERS, nullptr, headers, &dwSize, nullptr);
	}
	/*if (result)
	{
	char resultText[1024] = { 0 };
	unsigned long bytesRead;
	dwSize = ARRAYSIZE(resultText) * sizeof(char);
	result = ::WinHttpReadData(*request, resultText, dwSize, &bytesRead);
	if (result)
	{
	// Convert string to wstring
	int wideSize = MultiByteToWideChar(CP_UTF8, 0, resultText, -1, 0, 0);
	wchar_t* wideString = new wchar_t[wideSize];
	result = MultiByteToWideChar(CP_UTF8, 0, resultText, -1, wideString, wideSize);
	if (result)
	{
	std::wstring photoId = GetXmlElementValueByName(wideString, L"photoid", errorFound);
	if (!(*errorFound))
	{
	outputString = photoId;
	}
	}
	delete[] wideString;
	}
	}*/
	return outputString;
}

int SqrlPoster::doPost(const HINTERNET *request)
{
	static const char* mimeBoundary = "EBA799EB-D9A2-472B-AE86-568D4645707E";
	static const wchar_t* contentType =
		L"Content-Type: multipart/form-data; boundary=EBA799EB-D9A2-472B-AE86-568D4645707E\r\n";

	int result = ::WinHttpAddRequestHeaders(
		*request, contentType, (unsigned long)-1, WINHTTP_ADDREQ_FLAG_ADD);
	if (result)
	{
		std::wostringstream sb;

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"acl\"\r\n";
		sb << L"\r\n" << acl << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"key\"\r\n";
		sb << L"\r\n" << key << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"policy\"\r\n";
		sb << L"\r\n" << policy << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"signature\"\r\n";
		sb << L"\r\n" << signature << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"AWSAccessKeyId\"\r\n";
		sb << L"\r\n" << AWSAccessKeyId << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"Content-Type\"\r\n";
		sb << L"\r\n" << Content_Type << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"success_action_status\"\r\n";
		sb << L"\r\n" << success_action_status << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"file\"; filename=\"" << FileNamePlusExt << L"\"\r\n\r\n";
		sb << L"\r\n" << "Content-Type: application/pdf" << L"\r\n";

		// Convert wstring to string
		std::wstring wideString = sb.str();
		int stringSize = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
		char* temp = new char[stringSize];
		WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, temp, stringSize, nullptr, nullptr);
		std::string str = temp;
		delete[] temp;

		// Add the photo to the stream
		std::ifstream f(FullPath, std::ios::binary);
		std::ostringstream sb_ascii;
		sb_ascii << str;
		sb_ascii << f.rdbuf();
		sb_ascii << "\r\n--" << mimeBoundary << "\r\n";
		str = sb_ascii.str();
		result = WinHttpSendRequest(
			*request, 
			WINHTTP_NO_ADDITIONAL_HEADERS, 
			0, (void*)str.c_str(),
			static_cast<unsigned long>(str.length()), 
			static_cast<unsigned long>(str.length()), 
			0);
		if (result){

		}
		//DWORD err = GetLastError();

		//std::cout << err << std::endl;
	}
	return result;
}
std::wstring SqrlPoster::UploadPDF(bool* errorFound)
{
	std::wstring outputString;
	HINTERNET session = nullptr;
	HINTERNET connect = nullptr;
	HINTERNET request = nullptr;

	WINHTTP_AUTOPROXY_OPTIONS  autoProxyOptions;
	WINHTTP_PROXY_INFO proxyInfo;
	unsigned long proxyInfoSize = sizeof(proxyInfo);
	ZeroMemory(&autoProxyOptions, sizeof(autoProxyOptions));
	ZeroMemory(&proxyInfo, sizeof(proxyInfo));

	// Create the WinHTTP session.
	session = ::WinHttpOpen(
		L"Hilo/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	connect = ::WinHttpConnect(session, L"sqrl-dev-bucket.s3.amazonaws.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
	//TODO DWORD err = GetLastError();
	request = ::WinHttpOpenRequest(
		connect, L"POST", L"sqrl-dev-bucket.s3.amazonaws.com", L"HTTP/1.1",
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
	// Use DHCP and DNS-based auto-detection.
	autoProxyOptions.dwAutoDetectFlags =
		WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
	// If obtaining the PAC script requires NTLM/Negotiate authentication,
	// then automatically supply the client domain credentials.
	autoProxyOptions.fAutoLogonIfChallenged = true;

	if (FALSE != ::WinHttpGetProxyForUrl(
		session, L"sqrl-dev-bucket.s3.amazonaws.com", &autoProxyOptions, &proxyInfo))
	{
		// A proxy configuration was found, set it on the request handle.
		::WinHttpSetOption(request, WINHTTP_OPTION_PROXY, &proxyInfo, proxyInfoSize);
	}

	doPost(&request);
	outputString = GetPDFId(&request, errorFound);

	// Clean up
	if (proxyInfo.lpszProxy)
	{
		GlobalFree(proxyInfo.lpszProxy);
	}
	if (proxyInfo.lpszProxyBypass)
	{
		GlobalFree(proxyInfo.lpszProxyBypass);
	}

	WinHttpCloseHandle(request);
	WinHttpCloseHandle(connect);
	WinHttpCloseHandle(session);

	return outputString;
}

