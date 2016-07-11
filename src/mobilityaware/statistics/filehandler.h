#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <vector>
#include <string>


class FileHandler
{
public:
    FileHandler();

    std::vector<std::string> readLines(const std::string &_path);

    void appendToFile(const std::string &_data, const std::string &_path);
    void clearFile(const std::string &_path);
    void writeToFile(const std::string &_data, const std::string &_path);
};

#endif // FILEHANDLER_H
