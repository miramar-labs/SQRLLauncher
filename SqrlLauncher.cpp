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

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void stripFileQuotes(utility::string_t& s){
	s.erase(remove(s.begin(), s.end(), '\"'), s.end());
}

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
	LPTSTR lpCmdLine, int nShowCmd)
{
	//DEBUG: Set cmdline args in debug settings, eg: $(ProjDir)\test.pdf
	
	std::wstring sFilePath(lpCmdLine);
	
	stripFileQuotes(sFilePath);

	if (sFilePath.empty()){
		ATLTRACE("Sqrl:ERROR: missing pathname argument...");
		MessageBox(NULL, L"Sqrl:ERROR: missing pathname argument...", NULL, NULL);
		return 1;
	}
	if (!FileExists(sFilePath.c_str())){
		ATLTRACE("Sqrl:ERROR: path arg does not point to a valid file...");
		MessageBox(NULL, L"Sqrl:ERROR: path arg does not point to a valid file...", NULL, NULL);
		return 1;
	}

	SqrlPoster poster(sFilePath);

	bool err = FALSE;

	std::wstring url = poster.doUpload(&err);

	if ((!err) && (!url.empty())){		
		ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	} else {
		ATLTRACE("Sqrl:ERROR: unable to navigate to PDF...");
		std::wostringstream ss; ss << "Sqrl:ERROR: unable to navigate to PDF, err : " << err;
		MessageBox(NULL,ss.str().c_str(), NULL, NULL);
	}

	//delete locally generated PDF .... (release only ...)
#ifndef _DEBUG
	if (FileExists(sFilePath.c_str()))
		::DeleteFileW(sFilePath.c_str());		
#endif

	return _AtlModule.WinMain(nShowCmd);
}




