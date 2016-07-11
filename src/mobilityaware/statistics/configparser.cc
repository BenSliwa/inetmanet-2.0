#include "configparser.h"

ConfigParser::ConfigParser()
{
}

std::string ConfigParser::parse(const std::string &_data)
{
    std::string data = _data;

    std::map<std::string, std::string>::iterator it;
    for(it = m_configMapping.begin(); it != m_configMapping.end(); it++)
    {
        std::string key = it->first;
        std::string value = it->second;

        replace(data, key, value);
    }

    return data;
}

void ConfigParser::addMapping(const std::string &_key, const std::string &_value)
{
    if(containsKey(_key))
        m_configMapping.at(_key) = _value;
    else
        m_configMapping.insert(std::pair<std::string, std::string>(_key, _value));
}

bool ConfigParser::containsKey(const std::string &_key)
{
    return m_configMapping.count(_key);
}

std::map<std::string, std::string> ConfigParser::key2Map(const std::string &_key, char _firstLevelDelim, char _secondLevelDelim)
{
    std::map<std::string, std::string> data;

    std::vector<std::string> firstLevelItems = split(_key, _firstLevelDelim);
    for(int i=0; i<firstLevelItems.size(); i++)
    {
        std::string item = firstLevelItems.at(i);
        std::vector<std::string> items = split(item, _secondLevelDelim);

        if(items.size()==2)
        {
            std::string key = items.at(0);
            std::string value = items.at(1);
            data.insert(std::pair<std::string, std::string>(key, value));

            //std::cout << key << " -> " << value << std::endl;
        }
    }


    return data;
}

bool ConfigParser::replace(std::string& str, const std::string& from, const std::string& to)
{ // http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::vector<std::string> & ConfigParser::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> ConfigParser::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
