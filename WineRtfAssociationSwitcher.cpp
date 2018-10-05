
/// changes rtf association to default ubuntu office app during its lifetime
class CWineRtfAssociationSwitcher
{
public:
	CWineRtfAssociationSwitcher()
	{
			HKEY hKey;
			LONG res = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("rtfFile\\shell\\open\\command"), 0, KEY_READ | KEY_WRITE, &hKey);
			if (res == ERROR_SUCCESS) {
				TCHAR szChar[MAX_PATH];
				DWORD dwType;
				ULONG nBytes = sizeof(szChar);
				res = RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE)szChar, &nBytes);
				if (res == ERROR_SUCCESS && dwType == REG_SZ && nBytes != 0) {
					const CString strWineBrowser = _T("winebrowser \"%1\"");
					res = RegSetValueEx(hKey, _T(""), 0, REG_SZ, (CONST BYTE*)(LPCTSTR)strWineBrowser, (strWineBrowser.GetLength() + 1) * sizeof(TCHAR));
					if (res == ERROR_SUCCESS)
						m_currentKeyValue = szChar;
				}

				RegCloseKey(hKey);
      }
	}

	~CWineRtfAssociationSwitcher()
	{
		if (!m_currentKeyValue.IsEmpty()) {
			HKEY hKey;
			LONG res = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("rtfFile\\shell\\open\\command"), 0, KEY_WRITE, &hKey);
			if (res == ERROR_SUCCESS) {
				RegSetValueEx(hKey, _T(""), 0, REG_SZ, (CONST BYTE*)(LPCTSTR)m_currentKeyValue, (m_currentKeyValue.GetLength() + 1) * sizeof(TCHAR));
				RegCloseKey(hKey);
			}

		}
	}

private:
	CString m_currentKeyValue;
};
