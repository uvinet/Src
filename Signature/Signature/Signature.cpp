// Signature.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "SignatureMaker.h"

#include <chrono>
using namespace std::chrono;


#define press_any_key_return_0 { printf("\nPress any key..."); _getch(); return 0; }

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	if (argc < 3 || argc > 4)
	{
		printf(
			"Invalid parameters.\n"
			"Use parameters: input_file_name output_file_name [block size]\n"
		);
		press_any_key_return_0;
	}

	const std::string sourceFileName = argv[1];
	const std::string outputFileName = argv[2];

	unsigned long blockSize = CSignatureMaker::DefaultBlockSize();
	if (argc == 4) {
		try {
			const std::string strBuffer = argv[3];
			if (!std::all_of(strBuffer.begin(), strBuffer.end(), ::isdigit))
				throw std::invalid_argument("");
			blockSize = std::stoul(strBuffer);
		}
		catch (...) {
			printf("Block size argument is invalid\n");
			press_any_key_return_0;
		}
	}

	try
	{
		const steady_clock::time_point time_start(steady_clock::now());

		CSignatureMaker signatureMaker(blockSize);
		std::string outFileName;
		signatureMaker.Make(sourceFileName, outputFileName);

		const DWORD dwElapedMs = static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now() - time_start).count());

		printf("Processing time: %d milliseconds\n", dwElapedMs);
	}
	catch (const std::exception& e)
	{
		printf("\nError\n%s", e.what());
	}


	press_any_key_return_0;
}