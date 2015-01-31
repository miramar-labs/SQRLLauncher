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
	//std::wstring sFilePath(U("C:\\Users\\Aaron Cody\\Desktop\\sqrl\\test.pdf"));

	//DEBUG: Set cmdline args in debug settings to: C:\Users\Aaron Cody\Desktop\sqrl\test.pdf
	std::wstring sFilePath(lpCmdLine);

	SqrlPoster poster(sFilePath);

	bool err = FALSE;

	utility::string_t url = poster.doPOST(&err);

	if (!err){
		/* 
			bring up browser here.....
		*/
			IWebBrowser2* pBrowser = NULL;
			HRESULT hr = CoCreateInstance(CLSID_InternetExplorer, NULL,
			CLSCTX_SERVER, IID_IWebBrowser2, (LPVOID*)&pBrowser);

			if (SUCCEEDED(hr) && (pBrowser != NULL))
			{
				VARIANT vEmpty;
				VariantInit(&vEmpty);

				VARIANT vFlags;
				V_VT(&vFlags) = VT_I4;
				V_I4(&vFlags) = navOpenInNewWindow;

				BSTR bstrURL = SysAllocString(url.c_str());

				pBrowser->Navigate(bstrURL, &vFlags, &vEmpty, &vEmpty, &vEmpty);

				SysFreeString(bstrURL);

				if (pBrowser)
					pBrowser->Release();
			}

	}

	//delete generated PDF locally .... (release only ...)
#ifndef _DEBUG
	::DeleteFileW(sFilePath.c_str());		
#endif

	return _AtlModule.WinMain(nShowCmd);
}




