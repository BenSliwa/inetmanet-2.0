#include "filehandler.h"
#include <iostream>
#include <fstream>

FileHandler::FileHandler()
{
}

std::vector<std::string> FileHandler::readLines(const std::string &_path)
{
    std::vector<std::string> lines;

    std::ifstream file;
    std::string line;
    file.open(_path.c_str());
    if (file.is_open())
    {
        while (!file.eof())
        {
            getline(file, line);
            lines.push_back(line);
        }
    }
    else
        std::cout << "FileHandler::readLines ERROR " << _path << std::endl;

    file.close();

    return lines;
}

void FileHandler::appendToFile(const std::string &_data, const std::string &_path)
{
    std::ofstream file;
    file.open(_path.c_str(), std::ios::out | std::ios::app | std::ios::binary);

    if (file.is_open())
    {
        if(_data!="")
            file << _data << "\n";
    }
    else
        std::cout << "FileHandler::appendToFile ERROR " << _path << std::endl;

    file.close();
}

void FileHandler::clearFile(const std::string &_path)
{
    std::ofstream file;
    file.open(_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

    if (file.is_open())
        file << "";
    else
        std::cout << "FileHandler::clearFile ERROR " << _path << std::endl;

    file.close();
}

void FileHandler::writeToFile(const std::string &_data, const std::string &_path)
{
    clearFile(_path);
    appendToFile(_data, _path);
}
