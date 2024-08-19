#include "JSONHandler.hpp"
#include <filesystem>

#include <iostream>

namespace GWIN
{
    std::string JSONHandler::extractFilenameWithoutExtension(const std::string &filePath) const
    {
        std::filesystem::path path(filePath);
        return path.stem().string();
    }

    bool JSONHandler::loadFromFile(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            nlohmann::json jsonData;
            file >> jsonData;

            std::string filename = extractFilenameWithoutExtension(filePath);
            jsonDataMap[filename] = std::move(jsonData);
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }

    bool JSONHandler::saveToFile(const std::string &path, const std::string &filename) const
    {
        auto it = jsonDataMap.find(filename);
        if (it == jsonDataMap.end())
        {
            return false;
        }

        std::string fullPath = path + "/" + filename + ".json"; 

        std::ofstream file(fullPath);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            file << it->second.dump(); 
        }
        catch (const nlohmann::json::type_error &e)
        {
            return false;
        }

        return true;
    }

    template <typename T>
    T JSONHandler::getValue(const std::string &filename, const std::string &key) const
    {
        auto it = jsonDataMap.find(filename);
        if (it != jsonDataMap.end() && it->second.contains(key))
        {
            return it->second.at(key).get<T>();
        }
        return T();
    }

    template <typename T>
    void JSONHandler::setValue(const std::string &filename, const std::string &key, const T &value)
    {
        jsonDataMap[filename][key] = value;
    }

    void JSONHandler::setValue(const std::string &filename, const nlohmann::json &json)
    {
        jsonDataMap[filename] = json;
    }
}