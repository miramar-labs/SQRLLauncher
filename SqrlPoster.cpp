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

	memset(buff, 0, sizeof(buff));

	::GetCurrentDirectoryW(sizeof(buff), buff);

	iniFile = buff; iniFile += L"\\SqrlLauncher.ini";
	
	memset(buff, 0, sizeof(buff));

	GetPrivateProfileString(
		L"settings",
		L"sqrlhttps",
		L"sqrl-development.herokuapp.com",
		buff,
		sizeof(buff),
		iniFile.c_str()
		);

	sqrlhttps = buff;

	memset(buff, 0, sizeof(buff));

	GetPrivateProfileString(
		L"settings",
		L"sqrlendpoint",
		L"/api/v1/couriers/",
		buff,
		sizeof(buff),
		iniFile.c_str()
		);

	sqrlendpoint = buff;

	memset(buff, 0, sizeof(buff));

	GetPrivateProfileString(
		L"settings",
		L"sqrlendpoint2",
		L"/#/couriers/",
		buff,
		sizeof(buff),
		iniFile.c_str()
		);

	sqrlendpoint2 = buff;

	memset(buff, 0, sizeof(buff));

	GetPrivateProfileString(
		L"settings",
		L"bucketurl",
		L"sqrl-dev-bucket.s3.amazonaws.com",
		buff,
		sizeof(buff),
		iniFile.c_str()
		);

	bucketurl = buff;
}

SqrlPoster::~SqrlPoster()
{
}

std::wstring SqrlPoster::doUpload(bool* errorFound){

	std::wstring url;

	GetUploadInfo(errorFound).wait();

	if (*errorFound == FALSE)
		url = UploadPDF(errorFound);

	if (*errorFound == TRUE){
		bool err = false;	
		DeletePDF(&err).wait();
	}

	return url;
}

pplx::task<void> SqrlPoster::GetUploadInfo(bool* errorFound)
{
	/*
		Send a POST request to https://SQRL_SERVER/api/v1/couriers with the following in the body (size is in bytes):
		  {
		   "name": "someFilename.pdf",
		   "file_type": "application/pdf",
		   "size": 103774
		 }
	*/
	*errorFound = FALSE;

	std::wstring url(L"https://"); url += sqrlhttps;
	
	http_client client(url);

	json::value obj;

	obj[L"name"] = json::value::string(FileNamePlusExt);
	obj[L"file_type"] = json::value::string(U("application/pdf"));

	std::ifstream in(FullPath, std::ifstream::ate | std::ifstream::binary);
	std::ifstream::pos_type nbytes = in.tellg();

	obj[L"size"] = json::value::number((int32_t)nbytes);

	return client.request(methods::POST, sqrlendpoint, obj).then([errorFound](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		std::wostringstream ss; ss << "Sqrl:GetUploadInfo failed - status code : " << response.status_code();
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
		return pplx::task_from_result(json::value());
	})
		.then([this, errorFound](pplx::task<json::value> previousTask)
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
							std::ostringstream ss;ss << "key=" << key.c_str() << std::endl;
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
			}

		}
		catch (const http_exception& e)
		{
			std::wostringstream ss;ss << "Sqrl:GetUploadInfo failed - status code : " << e.what();
			ATLTRACE(ss.str().c_str());
			*errorFound = TRUE;
		}
	});
}

pplx::task<void> SqrlPoster::DeletePDF(bool* errorFound)
{
	return pplx::create_task([this, errorFound]
	{
		*errorFound = FALSE;

		/* construct the DELETE endpoint:
			send a DELETE request to https ://SQRL_SERVER/api/v1/couriers/TRACKING_NUMBER 
			(using the value of the "tracking_number" property from the response received from the request in step #1)
		*/

		std::wstring url(L"https://"); 
		url += sqrlhttps;
		url += sqrlendpoint;
		url += tracking_number;

		http_client client(url);

		return client.request(methods::DEL);

	}).then([errorFound](http_response response)
	{
		if (response.status_code() == status_codes::OK)
		{
			auto body = response.extract_string();

			std::wostringstream ss;ss << L"Sqrl:deleted file: " << body.get().c_str() << std::endl;
			ATLTRACE(ss.str().c_str());
		}
		else{
			std::wostringstream ss;ss << "Sqrl:WARNING - DELETE failed - status code: " << response.status_code() << std::endl;
			ATLTRACE(ss.str().c_str());
			*errorFound = TRUE;
		}
	});
}

void dbgDump(std::string& input){
#ifdef _DEBUG
	std::ofstream out("SqrlLauncher-dbg.txt");
	out << input;
	out.close();
#endif
}

void SqrlPoster::GetResponse(const HINTERNET *request, bool* errorFound)
{
	std::wstring outputString;
	int result = ::WinHttpReceiveResponse(*request, nullptr);
	if (!result){
		std::wostringstream ss; ss << L"Sqrl:GetResponse:WinHttpReceiveResponse failed - status code: " << GetLastError() << std::endl;
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
	}
	unsigned long dwSize = sizeof(unsigned long);
	if (result)
	{
		wchar_t headers[1024];
		dwSize = ARRAYSIZE(headers) * sizeof(wchar_t);

		result = ::WinHttpQueryHeaders(*request, WINHTTP_QUERY_RAW_HEADERS, nullptr, headers, &dwSize, nullptr);

		if (!result){
			std::wostringstream ss; ss << L"Sqrl:GetResponse:WinHttpQueryHeaders failed - status code: " << GetLastError() << std::endl;
			ATLTRACE(ss.str().c_str());
			*errorFound = TRUE;
		}
		else{
			wstring hdr(headers);
			if (hdr.find(L"OK") == std::wstring::npos){
				std::wostringstream ss; ss << L"Sqrl:GetResponse:WinHttpQueryHeaders contained an error: " << hdr.c_str() << std::endl;
				ATLTRACE(ss.str().c_str());
				*errorFound = TRUE;
			}
		}
	}
#ifdef _DEBUG	// get additional debug info...
	if (result && (*errorFound == TRUE))
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
				std::wostringstream ss; ss << L"Sqrl:GetResponse:WinHttpReadData: " << wideString << std::endl;
				ATLTRACE(ss.str().c_str());
			}
			delete[] wideString;
		}
		else{
			std::wostringstream ss; ss << L"Sqrl:GetResponse:WinHttpReadData failed - status code: " << GetLastError() << std::endl;
			ATLTRACE(ss.str().c_str());
			*errorFound = TRUE;
		}
	}
#endif
}

std::wstring SqrlPoster::UploadPDF(bool* errorFound)
{
	*errorFound = FALSE;

	std::wstring outputString = L"";

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
	if (!session){
		std::wostringstream ss;ss << "Sqrl:WinHttpOpen failed - status code: " << GetLastError() << std::endl;
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
		return L"";
	}
	connect = ::WinHttpConnect(session, deliver_to.substr(8).c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!connect){
		std::wostringstream ss;ss << "Sqrl:WinHttpConnect failed - status code: " << GetLastError() << std::endl;
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
		return L"";
	}
	request = ::WinHttpOpenRequest(
		connect, L"POST", L"", L"HTTP/1.1",		
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!request){
		std::wostringstream ss;ss << "Sqrl:WinHttpOpenRequest failed - status code: " << GetLastError() << std::endl;
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
		return L"";
	}

	autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
	// Use DHCP and DNS-based auto-detection.
	autoProxyOptions.dwAutoDetectFlags =
		WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
	// If obtaining the PAC script requires NTLM/Negotiate authentication,
	// then automatically supply the client domain credentials.
	autoProxyOptions.fAutoLogonIfChallenged = true;

	wstring tmp = deliver_to; tmp += L"/";

	if (FALSE != ::WinHttpGetProxyForUrl(
		session, tmp.c_str(), &autoProxyOptions, &proxyInfo))
	{
		// A proxy configuration was found, set it on the request handle.
		::WinHttpSetOption(request, WINHTTP_OPTION_PROXY, &proxyInfo, proxyInfoSize);
	}

	/*
		Send a multipart/form-data POST request with the file contents to the URL contained in the "deliver_to" property in the response above.
		The properties in the "form_data" object above should be used as the form data's name/value pairs.
		The contents of the file should be sent using the form item name "file".
	*/

	static const char* mimeBoundary = "B14433C2-EF49-4DB1-938F-EFFE9B471609";

	static const wchar_t* contentType = L"Content-Type: multipart/form-data; boundary=B14433C2-EF49-4DB1-938F-EFFE9B471609\r\n";

	int result = ::WinHttpAddRequestHeaders(request, contentType, (unsigned long)-1, WINHTTP_ADDREQ_FLAG_ADD);

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

		std::wstring wideString = sb.str();
		int stringSize = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
		char* temp = new char[stringSize];
		WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, temp, stringSize, nullptr, nullptr);
		std::string str = temp;
		delete[] temp;

		std::ifstream f(FullPath, std::ios::binary);
		std::ostringstream sb_ascii;
		sb_ascii << str;
		sb_ascii << f.rdbuf();
		sb_ascii << "\r\n--" << mimeBoundary << "--\r\n";
		str = sb_ascii.str();

		dbgDump(str);

		result = WinHttpSendRequest(
			request,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, (void*)str.c_str(),
			static_cast<unsigned long>(str.length()),
			static_cast<unsigned long>(str.length()),
			0);

		if (result != TRUE){
			std::wostringstream ss;ss << "Sqrl:WinHttpSendRequest failed - status code: " << GetLastError() << std::endl;
			ATLTRACE(ss.str().c_str());
			*errorFound = TRUE;
		}
	}
	else{
		std::wostringstream ss;ss << "Sqrl:WinHttpAddRequestHeaders failed - status code: " << GetLastError() << std::endl;
		ATLTRACE(ss.str().c_str());
		*errorFound = TRUE;
	}

	GetResponse(&request, errorFound);

	if (*errorFound == FALSE){
		/* construct the browser redirect URL:
			Once request in step #2 is complete (i.e. the file has been transferred to S3),
			launch the default web browser and go to https://SQRL_SERVER/#/couriers/TRACKING_NUMBER
			(using the value of the "tracking_number" property from the response received from the request in step #1)
		*/
		outputString = L"https://";
		outputString+= sqrlhttps;
		outputString += sqrlendpoint2;
		outputString += tracking_number;

		std::wostringstream ss; ss << "Sqrl:SUCCESS: uploaded " << FileNamePlusExt.c_str() << " to AWS bucket: " << key.c_str() << std::endl;
		ATLTRACE(ss.str().c_str());
	}

	if (proxyInfo.lpszProxy)
		GlobalFree(proxyInfo.lpszProxy);
	if (proxyInfo.lpszProxyBypass)
		GlobalFree(proxyInfo.lpszProxyBypass);

	if (request)
		WinHttpCloseHandle(request);
	if (connect)
		WinHttpCloseHandle(connect);
	if (session)
		WinHttpCloseHandle(session);

	return outputString;
}

