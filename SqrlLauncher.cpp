// SqrlLauncher.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "SqrlLauncher_i.h"
#include "xdlldata.h"

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

class CSqrlLauncherModule : public ATL::CAtlExeModuleT< CSqrlLauncherModule >
{
public :
	DECLARE_LIBID(LIBID_SqrlLauncherLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SQRLLAUNCHER, "{C5EDD644-8351-4F2F-A0EE-BB035E6944E0}")
	};

CSqrlLauncherModule _AtlModule;

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

int doPost(
	const HINTERNET *request, const std::wstring& fileName)
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
		sb << L"\r\n" << "private" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"key\"\r\n";
		sb << L"\r\n" << "uploads/test000/test.pdf" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"policy\"\r\n";
		sb << L"\r\n" << "eyJleHBpcmF0aW9uIjoiMjAxNS0wMS0yN1QwMDo1NjoyNC4wMDBaIiwiY29uZGl0aW9ucyI6W3siYnVja2V0Ijoic3FybC1sb2NhbC1kZXYtYnVja2V0In0seyJhY2wiOiJwcml2YXRlIn0sWyJzdGFydHMtd2l0aCIsIiRrZXkiLCJ1cGxvYWRzLyJdLFsic3RhcnRzLXdpdGgiLCIkQ29udGVudC1UeXBlIiwiIl0seyJzdWNjZXNzX2FjdGlvbl9zdGF0dXMiOiIyMDAifV19" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"signature\"\r\n";
		sb << L"\r\n" << "fNd6Z4GSNVlFsMbrTlcyLNuH9Dg=" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"AWSAccessKeyId\"\r\n";
		sb << L"\r\n" << "AKIAJSOOLKBEJ2WENZVA" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"Content-Type\"\r\n";
		sb << L"\r\n" << "application/pdf" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"success_action_status\"\r\n";
		sb << L"\r\n" << "200" << L"\r\n";

		sb << L"--" << mimeBoundary << L"\r\n";
		sb << L"Content-Disposition: form-data; name=\"file\"; filename=\"" << fileName << L"\"\r\n\r\n";
		sb << L"\r\n" << "Content-Type: application/pdf" << L"\r\n";

		// Convert wstring to string
		std::wstring wideString = sb.str();
		int stringSize = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
		char* temp = new char[stringSize];
		WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, temp, stringSize, nullptr, nullptr);
		std::string str = temp;
		delete[] temp;

		// Add the photo to the stream
		std::ifstream f(fileName, std::ios::binary);
		std::ostringstream sb_ascii;
		sb_ascii << str;
		sb_ascii << f.rdbuf();
		sb_ascii << "\r\n--" << mimeBoundary << "\r\n";
		str = sb_ascii.str();
		result = WinHttpSendRequest(
			*request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (void*)str.c_str(),
			static_cast<unsigned long>(str.length()), static_cast<unsigned long>(str.length()), 0);

		DWORD err = GetLastError();

		std::cout << err << std::endl;
	}
	return result;
}
std::wstring UploadPDF(const std::wstring& fileName, bool* errorFound)
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
	//DWORD err = GetLastError();
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

	doPost(&request, fileName);
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



	pplx::task<void> RequestJSONValueAsync()
	{
		// TODO: To successfully use this example, you must perform the request  
		// against a server that provides JSON data.  
		// This example fails because the returned Content-Type is text/html and not application/json.
		http_client client(L"https://sqrl-development.herokuapp.com");

		json::value obj;
		obj[L"name"] = json::value::string(U("test.pdf"));
		obj[L"file_type"] = json::value::string(U("application/pdf"));
		obj[L"size"] = json::value::number(1929755);

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
			.then([](pplx::task<json::value> previousTask)
		{
			try
			{
				const json::value& v = previousTask.get();

				for (auto iter = v.as_object().cbegin(); iter != v.as_object().cend(); ++iter) {

						const utility::string_t &str = iter->first;
						const json::value &v = iter->second;

						std::wcout << L"String: " << str.c_str() << L", Value: " << v.serialize() << std::endl;


					}

				
				std::cout << "" << std::endl;
			}
			catch (const http_exception& e)
			{
				// Print error.
				//wostringstream ss;
				//ss << e.what() << endl;
				//wcout << ss.str();
			}
		});

		/* Output:
		Content-Type must be application/json to extract (is: text/html)
		*/
	}

	pplx::task<void> doPOSTAsync()
	{
		pplx::task<void> requestTaskPost = file_stream<unsigned char>::open_istream(L"test.pdf").then([](basic_istream<unsigned char> fileStream){
			
			// Make HTTP request with the file stream as the body.
			http_client client(U("https://sqrl-local-dev-bucket.s3.amazonaws.com"));

			http_request req;
			req.set_method(methods::POST);
			req.set_body(fileStream);

			req.headers().add(U("Content-Disposition: form-data; name=\"acl\""), U("private"));

			req.headers().add(U("Content-Disposition: form-data; name=\"key\""), U("uploads/test000/test.pdf"));

			req.headers().add(U("Content-Disposition: form-data; name=\"policy\""), U("eyJleHBpcmF0aW9uIjoiMjAxNS0wMS0yN1QwMDo1NjoyNC4wMDBaIiwiY29uZGl0aW9ucyI6W3siYnVja2V0Ijoic3FybC1sb2NhbC1kZXYtYnVja2V0In0seyJhY2wiOiJwcml2YXRlIn0sWyJzdGFydHMtd2l0aCIsIiRrZXkiLCJ1cGxvYWRzLyJdLFsic3RhcnRzLXdpdGgiLCIkQ29udGVudC1UeXBlIiwiIl0seyJzdWNjZXNzX2FjdGlvbl9zdGF0dXMiOiIyMDAifV19"));

			req.headers().add(U("Content-Disposition: form-data; name=\"signature\""), U("fNd6Z4GSNVlFsMbrTlcyLNuH9Dg="));

			req.headers().add(U("Content-Disposition: form-data; name=\"AWSAccessKeyId\""), U("AKIAJSOOLKBEJ2WENZVA"));

			req.headers().add(U("Content-Disposition: form-data; name=\"Content-Type\""), U("application/pdf"));

			req.headers().add(U("Content-Disposition: form-data; name=\"success_action_status\""), U("200"));

			req.headers().add(U("Content-Disposition: form-data; name=\"file\"; filename=\"test.pdf\""), U("Content-Type: application/pdf"));

			req.headers().set_content_length(1929755);

			req.headers().set_content_type(U("multipart"));

			client.request(req).then([fileStream](http_response response){
				fileStream.close();
				// Perform actions here to inspect the HTTP response...
				if (response.status_code() == status_codes::OK){
					//TODO: get sqrl GUID here....
				}
			});
		}); 

		return requestTaskPost;
	}

#ifdef HIDEME
void doPOST(){
	pplx::task<void> requestTaskPost = file_stream<unsigned char>::open_istream(L"test.pdf").then([](basic_istream<unsigned char> fileStream)
	{
		// Make HTTP request with the file stream as the body.
		http_client client(L"https://sqrl-development.herokuapp.com");

		// multipart/form-data
		//file_stream<unsigned char>::open_istream inputStream;

		//name = test.pdf
		//size = 1,929,755

		http_request req;
		req.set_method(methods::POST);
		req.set_body(fileStream);
		req.headers().set_content_type(U("multipart"));

		return client.request(req).then([fileStream](http_response response)
		{
			fileStream.close();
			// Perform actions here to inspect the HTTP response...
			if (response.status_code() == status_codes::OK)
			{
				std::cout << "Response OK" << std::endl;
			}
		});
	});
}

void doRESTCall(const std::wstring& file){
	//SQRL_SERVER = sqrl - development.herokuapp.com		//TODO
	
	http_client c(U("https://development.herokuapp.com"));

	c.request(methods::POST, "/api/v1/couriers", "{\"name\": \"someFilename.pdf\",\"file_type\": \"application / pdf\",\"size\": 103774}").then([=](http_response response){
		printf("Received response status code:%u\n", response.status_code());
	});

	/* Open stream to file.
	file_stream<unsigned char>::open_istream(file).then([](basic_istream<unsigned char> fileStream){
		// Make HTTP request with the file stream as the body.
		http_client client(U("http://www.myhttpserver.com"));
		client.request(methods::PUT, L"myfile", fileStream).then([fileStream](http_response response){
			fileStream.close();
			// Perform actions here to inspect the HTTP response...
			if (response.status_code() == status_codes::OK){
				//TODO: get sqrl GUID here....
			}
		});
	});*/
	//std::this_thread::sleep_for(std::chrono::seconds(10));
}
#endif
//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
								LPTSTR lpCmdLine, int nShowCmd)
{
	std::wstring sFile(U("C:\\Users\\Aaron Cody\\Miramar Labs\\Sqrl Project\\Debug\\test.pdf")); 

	// REST call here....
	RequestJSONValueAsync().wait();

	bool err = false;

	std::wstring result = UploadPDF(sFile, &err);
	

	

	/* bring up browser here.....
	std::wstring sFileRemote(L"http://www.getsqrl.com");
	std::wstring sFileLocal(L"file:////"); sFileLocal += sFile;

	IWebBrowser2*	 m_spWebBrowser = NULL;//TODO
	BSTR url = SysAllocString(sFileLocal.c_str());
	m_spWebBrowser->Navigate(url, NULL, NULL, NULL, NULL);
	SysFreeString(url);*/

	//delete generated PDF ....
	//DeleteFileW(sFile.c_str());		//will fail if in use....

	return _AtlModule.WinMain(nShowCmd);
}

