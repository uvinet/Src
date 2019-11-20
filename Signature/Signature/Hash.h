/***********************************************************************************************//**
*     \file Hash.h.
*
*     \brief Declares the hash functions
****************************************************************************************************/

#pragma once

#pragma once

namespace CRC
{
	// make static crc table
	static std::vector<unsigned long> st_crc32_table = [] {
		std::vector<unsigned long> table;
		unsigned long crc;
		for (int i = 0; i < 256; i++)
		{
			crc = i;
			for (int j = 0; j < 8; j++)
				crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
			table.push_back(crc);
		}
		return std::move(table);
	}();

	unsigned int toCRC32(std::vector<byte>& bytes)
	{
		_ASSERT(!st_crc32_table.empty());
		unsigned long crc = 0xFFFFFFFFUL;
		auto len = bytes.size();
		int pos = 0;
		while (len--)
			crc = st_crc32_table[(crc ^ bytes.at(pos++)) & 0xFF] ^ (crc >> 8);
		return crc ^ 0xFFFFFFFFUL;
	}

} // namespace CRC