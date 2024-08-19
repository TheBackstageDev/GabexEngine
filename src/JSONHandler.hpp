#pragma once
#include "json.hpp"
#include <fstream>
#include <string>
#include <regex>
#include <string>

namespace GWIN
{
    class JSONHandler
    {
    public:
        JSONHandler() = default;
        ~JSONHandler() = default;

        bool loadFromFile(const std::string &filePath);
        bool saveToFile(const std::string &path, const std::string &filename) const;

        template <typename T>
        T getValue(const std::string &filename, const std::string &key) const;

        template <typename T>
        void setValue(const std::string &filename, const std::string &key, const T &value);
        void setValue(const std::string &filename, const nlohmann::json &json);

    private:
        std::string JSONHandler::extractFilenameWithoutExtension(const std::string &filePath) const;
        std::unordered_map<std::string, nlohmann::json> jsonDataMap;
    };
} // namespace GWIN
