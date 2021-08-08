#include <iostream>
#include <sstream>
#include <string>
#include <iterator>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "HexFileReader.h"

HexFileReader::HexFileReader(const char* path) : file(path), nextIdx(0)
{
  if (!HexFileReader::isHexFile(path))
    throw HexFileException("Only accept HEX file.");
  if (!file.is_open())
    throw HexFileException(
        std::string("Error: ").append(path).append(std::string(" could not be open.")));
  ReadFile();
}

void HexFileReader::ReadFile()
{
  std::string line;
  while (!file.eof())
  {
    std::getline(file, line);
    hexArr.push_back(line);
  }
}

std::string HexFileReader::nextLine()
{
  if (nextIdx >= numLines()) return NULL;
  return hexArr[nextIdx++];
}