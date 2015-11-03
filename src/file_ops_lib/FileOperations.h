#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <boost/filesystem/path.hpp>
#include <vector>

std::vector<char> readData(const boost::filesystem::path& path, unsigned int position, unsigned int size);
void overWriteData(const boost::filesystem::path& path, unsigned int position, const std::vector<char>& buffer);

#endif // FILEOPERATIONS_H