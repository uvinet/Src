isEqualNoCase for std::tstring (std::wstring or std::string) with lambda

namespace std
{
	typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > 
		tstring;

	typedef basic_stringstream<TCHAR, char_traits<TCHAR>,
		allocator<TCHAR> > tstringstream;
};

		auto isEqualNoCase = [](LPCTSTR lpstr1, LPCTSTR lpstr2) {
			std::tstring str1 = lpstr1;
			std::tstring str2 = lpstr2;
			return 
				((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), [](TCHAR & c1, TCHAR & c2) {
#ifdef  UNICODE
					return (c1 == c2 || std::towupper(c1) == std::towupper(c2));
#else
					return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
#endif
			}));
		};
