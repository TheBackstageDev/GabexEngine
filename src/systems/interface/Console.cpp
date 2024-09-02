#include "Console.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm> //For transform
#include <sstream>

namespace GWIN
{
    std::vector<GWConsole::LogEntry> GWConsole::History;
    float GWConsole::passedTime = 0.0f;

    GWConsole::GWConsole()
    {
        Commands.push_back(std::move("help"));
        Commands.push_back(std::move("clear"));
        Commands.push_back(std::move("print"));
        Commands.push_back(std::move("register"));

        passedTime += 0.0f;
    };

    GWConsole::~GWConsole()
    {
        if (logOnExit)
        {
            writeHistoryToFile();
        }
        clearLog();
    };

    void GWConsole::clearLog()
    {
        History.clear();
    }

    void GWConsole::addLog(const std::string &log)
    {
        History.push_back({passedTime, LogType::LOG, log});
    }

    void GWConsole::addError(const std::string &error)
    {
        History.push_back({passedTime, LogType::ERR, error});
    }

    void GWConsole::addWarning(const std::string &warning)
    {
        History.push_back({passedTime, LogType::WARNING, warning});
    }

    GWGameObject* GWConsole::findObjectByName(std::string &name, FrameInfo& frameInfo)
    {
        for (auto &kv : frameInfo.currentInfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.getName() == name)
            {
                return &obj;
            }
        }

        return nullptr;
    }

    void GWConsole::cmdPrint(const std::string &command, FrameInfo &frameInfo)
    {
        std::istringstream iss(command);
        std::string cmd, objectName, property;

        iss >> cmd;

        char quoteChar = '\0';
        if (iss.peek() == '"' || iss.peek() == '\'')
        {
            quoteChar = iss.get(); 
            std::getline(iss, objectName, quoteChar); 
        }
        else
        {
            iss >> objectName; 
        }

        iss >> property;

        GWGameObject *obj = findObjectByName(objectName, frameInfo);

        if (obj == nullptr)
        {
            addError("Object with name " + objectName + " not found.");
            return;
        }
    }

    void GWConsole::checkCommands(const std::string &command, FrameInfo& frameInfo)
    {
        std::string lowerCommand = command;
        std::transform(lowerCommand.begin(), lowerCommand.end(), lowerCommand.begin(), ::tolower);

        if (lowerCommand == "clear")
        {
            clearLog();
        }
        else if (lowerCommand == "help")
        {
            addLog("Commands:");
            addLog("help -// Show's all commands.");
            addLog("clear -// Clears the history (will also clear logs until this point on the registered File.)");
            addLog("print -// Print's the rest of the input into the screen, cmd \" help print\" to see all.");
            addLog("register -// Register's all logs into a .txt File.");
        }
        else if (lowerCommand.substr(0, 5) == "print")
        {
            cmdPrint(command, frameInfo);
        }
        else if (lowerCommand == "help print")
        {
            addLog("Commands:");
            addLog("<text> -// print's out the text you sent.");
            addLog("<objectName> <property> -// Print's out a property of the object.");
        }
        else if (lowerCommand == "register")
        {
            writeHistoryToFile();
        }
        else
        {
            addWarning("Unknown command: " + command);
        }
    }

    // https://stackoverflow.com/questions/41304891/how-to-count-the-number-of-files-in-a-directory-using-standard
    std::size_t number_of_files_in_directory(std::filesystem::path path)
    {
        using std::filesystem::directory_iterator;

        try
        {
            return std::distance(directory_iterator(path), directory_iterator{});
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Error accessing directory: " << e.what() << '\n';
            return 0;
        }
    }

    void GWConsole::writeHistoryToFile()
    {
        const std::filesystem::path logDir = "./logs";

        try
        {
            if (!std::filesystem::exists(logDir))
            {
                std::filesystem::create_directories(logDir);
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            addError("Failed to create logs directory: " + std::string(e.what()));
            return;
        }

        std::string id = logDir.string() + "/log" + std::to_string(number_of_files_in_directory(logDir)) + ".txt";
        std::ofstream logFile(id.c_str());

        if (!logFile.is_open())
        {
            addError("Failed to open log file for writing: " + id);
            return;
        }

        for (const auto &entry : History)
        {
            logFile << std::fixed << std::setprecision(2) << entry.time << "s " << entry.message << "\n";
        }

        if (!logFile)
        {
            addError("Error occurred while writing to log file.");
        }

        logFile.close();
    }

    void GWConsole::consoleMainMenu(FrameInfo& frameInfo)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Logs", "Ctrl+S"))
                {
                    writeHistoryToFile();
                }
                if (ImGui::MenuItem("Clear Logs", "Ctrl+C"))
                {
                    clearLog();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("AutoScroll", NULL, &autoScroll);
                ImGui::MenuItem("LogOnExit", NULL, &logOnExit);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Show Help", "Ctrl+H"))
                {
                    checkCommands("help", frameInfo);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void GWConsole::draw(FrameInfo &frameInfo)
    {
        passedTime += frameInfo.deltaTime;
        ImGui::SetNextWindowSize(ImVec2(700, 200));
        if (ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar))
        {
            ImGui::SameLine();

            consoleMainMenu(frameInfo);
            ;
            ImGui::Separator();

            if (ImGui::BeginChild("ConsoleScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
            {
                for (auto &entry : History)
                {
                    ImVec4 color;
                    switch (entry.type)
                    {
                    case LogType::LOG:
                        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
                        break;
                    case LogType::ERR:
                        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                        break;
                    case LogType::WARNING:
                        color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                        break;
                    }

                    ImGui::TextColored(color, "%.2f %s", entry.time, entry.message.c_str());
                }
            }

            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();

            ImGui::Separator();

            ImGui::InputText("Input", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                std::string input(inputBuffer);
                if (!input.empty())
                {
                    addLog("> " + input);
                    checkCommands(input, frameInfo);
                    input.clear();
                    memset(inputBuffer, 0, sizeof(inputBuffer));
                }
                ImGui::SetKeyboardFocusHere(-1);
            }
        }
        ImGui::End();
    }
}