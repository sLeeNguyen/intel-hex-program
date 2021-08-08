// HexFileReader.h
#ifndef HEXFILEREADER_H
#define HEXFILEREADER_H

#include <fstream>
#include <string>
#include <vector>

#include "HexFileException.h"

class HexFileReader
{
private:
	std::ifstream file;
	std::vector<std::string> hexArr;
	int nextIdx;

	void ReadFile();

public:
	HexFileReader(const char* path);

	HexFileReader(std::string& path) : HexFileReader(path.c_str()) {}

	static bool isHexFile(const char* path)
	{
		std::string s(path);
		// filename doesn't contain `.hex` extension
		if (s.length() < 4 || s.compare(s.length() - 4, 4, ".hex") != 0)
			return false;

		return true;
	}

	std::string nextLine();

	int numLines() { return hexArr.size(); }
};

#endif