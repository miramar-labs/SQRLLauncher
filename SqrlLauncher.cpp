// SqrlLauncher.cpp : Implementation of WinMain
#include "stdafx.h"
#include "resource.h"
#include "SqrlLauncher_i.h"
#include "xdlldata.h"
#include "SqrlPoster.h"

class CSqrlLauncherModule : public ATL::CAtlExeModuleT< CSqrlLauncherModule >
{
public :
	DECLARE_LIBID(LIBID_SqrlLauncherLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SQRLLAUNCHER, "{C5EDD644-8351-4F2F-A0EE-BB035E6944E0}")
	};

CSqrlLauncherModule _AtlModule;

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
	LPTSTR lpCmdLine, int nShowCmd)
{
	std::wstring sFilePath(U("C:\\Users\\Aaron Cody\\Miramar Labs\\Sqrl Project\\Debug\\test.pdf"));

	SqrlPoster poster(sFilePath);

	utility::string_t url = poster.doPOST();

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
#ifdef HIDEME

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



