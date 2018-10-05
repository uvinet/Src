/// Application installed from Microsoft Store 
BOOL IsAppXProgId(LPCTSTR progId)
{
	LPCTSTR appx = L"AppX";
	const size_t len = _tcslen(appx);
	return _tcslen(progId)>len && _tcsnicmp(progId, appx, len)==0;
}

/// Returns default browser file name
void GetDefaultBrowserFileName(CString& strres)
{
  CString strPath;
	HKEY hKey;

	// using URLAssociations method (Win8+)
	LONG res = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\URLAssociations\\http\\UserChoice"), 0, KEY_READ, &hKey);
	if (res == ERROR_SUCCESS)
	{
		TCHAR szChar[MAX_PATH] = {'\0'};
		DWORD dwType;
		ULONG nBytes = sizeof(szChar);

		res = RegQueryValueEx(hKey, _T("ProgId"), NULL, &dwType, (LPBYTE)szChar, &nBytes);
		if (res == ERROR_SUCCESS && (dwType == REG_SZ || dwType == REG_EXPAND_SZ) && nBytes != 0)
		{
			if( IsAppXProgId(szChar) )
			{
				RegCloseKey(hKey);

				strres = GetAppXFullName(szChar);
				return;
			}
			else
			{
				CString strKeyPath;
				strKeyPath.Format(_T("%s\\shell\\open\\command"), szChar);
				HKEY hKeyProgId;
				res = RegOpenKeyEx(HKEY_CLASSES_ROOT, strKeyPath, 0, KEY_READ, &hKeyProgId);
				if (res == ERROR_SUCCESS) 
				{
					szChar[0] = _T('\0');
					nBytes = sizeof(szChar);
					res = RegQueryValueEx(hKeyProgId, _T(""), NULL, &dwType, (LPBYTE)szChar, &nBytes);
					if (res == ERROR_SUCCESS) {

						strPath = szChar;
						if (dwType == REG_EXPAND_SZ)
						{
							TCHAR buf[MAX_PATH];
							ExpandEnvironmentStrings(strPath, buf, MAX_PATH);
							strPath = buf;
						}
					}

					RegCloseKey(hKeyProgId);
				}
			}
		}

		RegCloseKey(hKey);
	}

	// using classic method
	if (strPath.IsEmpty())
	{
		res = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("HTTP\\shell\\open\\command"), 0, KEY_READ, &hKey);
		if(res == ERROR_SUCCESS)
		{
			TCHAR szChar[MAX_PATH] = {'\0'};
			DWORD dwType;
			ULONG nBytes = sizeof(szChar);

			res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szChar,	&nBytes);
			if(res == ERROR_SUCCESS && (dwType==REG_SZ || dwType==REG_EXPAND_SZ) && nBytes != 0) 
			{
				strPath = szChar;
				if(dwType==REG_EXPAND_SZ)
				{
					TCHAR buf[MAX_PATH];
					ExpandEnvironmentStrings(strPath, buf, MAX_PATH);
					strPath = buf;
				}
			}

			RegCloseKey(hKey);
		}
	}

	if (!strPath.IsEmpty())
		strres = strPath;
}

/// default Mail client path
void GetDefaultMailClientPath(CXString& strres)
{
	strres.Empty();
	HKEY hKey;
	LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Clients\\Mail"), 0, KEY_READ, &hKey);
	if(res == ERROR_SUCCESS)
	{
		TCHAR szName[MAX_PATH];
		DWORD dwType;
		ULONG nBytes = sizeof(szName);
		res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szName,	&nBytes);
		if(res == ERROR_SUCCESS && dwType == REG_SZ && nBytes != 0) 
			strres += szName;
		RegCloseKey(hKey);

		if(!strres.IsEmpty())
		{
			CString strKey;
			strKey.Format(_T("SOFTWARE\\Clients\\Mail\\%s\\shell\\open\\command"), strres);
			res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, KEY_READ, &hKey);
			if(res == ERROR_SUCCESS)
			{
				szName[0] = _T('\0');
				nBytes = sizeof(szName);
				res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szName,	&nBytes);
				if(res == ERROR_SUCCESS && (dwType==REG_SZ || dwType==REG_EXPAND_SZ) && nBytes != 0) 
				{
					CString strFile(szName);
					int nQuotePos = strFile.Find(_T('"'), 1);
					if(nQuotePos>0)
					{
						strFile =  strFile.Mid(1, nQuotePos-1);
						if(!strFile.IsEmpty())
						{
							if(dwType==REG_EXPAND_SZ)
							{
								TCHAR buf[MAX_PATH];
								ExpandEnvironmentStrings(strFile, buf, MAX_PATH);
								strFile = buf;
							}
              
							if (!strPath.IsEmpty())
                strres = strPath;
						}
					}
				}

				RegCloseKey(hKey);
			}
		}
	}
}

/// Ms Word FileName
void GetMsWordFileName(CXString& strres)
{
	HKEY hKey;
	LONG res = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("Word.Application\\CLSID"), 0, KEY_READ, &hKey);
	if(res == ERROR_SUCCESS)
	{
		CString strCLSID;
		TCHAR szChar[MAX_PATH];
		DWORD dwType;
		ULONG nBytes = sizeof(szChar);
		res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szChar,	&nBytes);
		if(res == ERROR_SUCCESS && dwType == REG_SZ && nBytes != 0)
			strCLSID = szChar;			
		RegCloseKey(hKey);

		strCLSID.Trim();
		if(!strCLSID.IsEmpty())
		{
			CString strKey;
			strKey.Format(_T("Software\\Classes\\CLSID\\%s\\LocalServer32"), strCLSID);
			res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, KEY_READ, &hKey);
			if(res == ERROR_SUCCESS)
			{
				szChar[0] = _T('\0');
				nBytes = sizeof(szChar);
				res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szChar,	&nBytes);
				if(res == ERROR_SUCCESS && (dwType==REG_SZ || dwType==REG_EXPAND_SZ) && nBytes != 0) 
				{
					CString strPath(szChar);
					if(dwType==REG_EXPAND_SZ)
					{
						TCHAR buf[MAX_PATH];
						ExpandEnvironmentStrings(strPath, buf, MAX_PATH);
						strPath = buf;
					}

					int nSlash = strPath.Find(_T('/'));
					if(nSlash>0)
						strPath = strPath.Left(nSlash-1);
  			}

			RegCloseKey(hKey);
        
			if (!strPath.IsEmpty())
			   strres = strPath;        
        
			}
		}
	}
}
