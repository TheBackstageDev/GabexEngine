#pragma once

#include <imgui/imgui.h>
#include <string>
#include <vector>

#include "GWFrameInfo.hpp"

namespace GWIN
{
    class GWConsole
    {
    public:
        GWConsole();

        ~GWConsole();

        void draw(FrameInfo& frameInfo);

        void addLog(const std::string &log);
        void addError(const std::string &error);
        void addWarning(const std::string &warning);

        void writeHistoryToFile();
    private:
        enum class LogType
        {
            LOG,
            ERR,
            WARNING
        };

        struct LogEntry
        {
            float time;
            LogType type;
            std::string message;
        };

        std::vector<LogEntry> History;
        std::vector<std::string> Commands;

        GWGameObject *findObjectByName(std::string &name, FrameInfo &frameInfo);

        void cmdPrint(const std::string &command, FrameInfo &frameInfo);
        void clearLog();
        void checkCommands(const std::string& command, FrameInfo& frameInfo);
        void consoleMainMenu(FrameInfo &frameInfo);
        //Flags
        bool autoScroll{true};
        bool logOnExit{true};
        char inputBuffer[256] = "";

        float passedTime{0.f};
    };
}