// TestProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <locale.h>
#include "..\LogReader\LogReader.h"

#include <conio.h> // for getch()

int _tmain(int argc, _TCHAR* argv[])
{
	_tsetlocale(LC_ALL,_T("Russian"));

	printf_s("This is a test program for CLogReader class.\nIt searches for a string matching the pattern in specified file.\n\n");

	if (argc == 3)
	{
		const char* pFileName = argv[1];
		const char* pFilter = argv[2];

		CLogReader logReader;
		bool bNoError = logReader.Open( pFileName );
		if(bNoError) {
			bNoError = logReader.SetFilter( pFilter );
			if(bNoError) {

				printf_s("Processing...\n");

				const DWORD dwStartTime = GetTickCount();
				int nFound = 0;
				char szResultLine[2000] = { '\0' };
				bool bContinue = true;
				while (bContinue) {
					LONGLONG lineNum;
					bContinue = logReader.GetNextLine(szResultLine, sizeof(szResultLine), lineNum);
					if(bContinue) {
						++nFound;
						printf_s("Found string (line %llu): %s\n", lineNum, szResultLine);
					}
					else
						bNoError = logReader.GetLastErrorCode()==EC_NO_ERROR;
				}

				printf_s("Matches found: %d\n", nFound);
				printf_s("Processing time: %d ms\n", GetTickCount() - dwStartTime);
			}
		}

		if(!bNoError)
			PrintError(logReader.GetLastErrorCode());
	}
	else {
		printf_s("Incorrect parameters\nUsage: TestProject pattern\n");
	}

	printf_s("\nPress any key...");
	_getch();

	return 0;
}

