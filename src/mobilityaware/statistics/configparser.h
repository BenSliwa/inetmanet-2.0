#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>

class ConfigParser
{
public:
    ConfigParser();



    std::string parse(const std::string &_data);
    void addMapping(const std::string &_key, const std::string &_value);
    bool containsKey(const std::string &_key);

    std::map<std::string, std::string> key2Map(const std::string &_key,  char _firstLevelDelim, char _secondLevelDelim);

    bool replace(std::string& str, const std::string& from, const std::string& to);
    std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) ;
    std::vector<std::string> split(const std::string &s, char delim);


private:
    std::map<std::string, std::string> m_configMapping;


};


#endif // CONFIGPARSER_H
