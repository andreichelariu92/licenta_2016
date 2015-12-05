#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <boost/filesystem/path.hpp>
#include <vector>

std::vector<char> readData(const boost::filesystem::path& path, unsigned int position, unsigned int size);
void overWriteData(const boost::filesystem::path& path, unsigned int position, const std::vector<char>& buffer);
void removeData(const boost::filesystem::path& path, unsigned int position, unsigned int length);
void addData(const boost::filesystem::path& path, unsigned int position, const std::vector<char>& buffer);

enum FileOperationOptions { FOO_file=0, FOO_directory=1};
void  createFileOrDirectory(FileOperationOptions option, const boost::filesystem::path& path);
void deleteFileOrDirectory(const boost::filesystem::path& path);
#endif // FILEOPERATIONS_H
