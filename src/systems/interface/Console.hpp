#pragma once

#include <imgui/imgui.h>
#include <string>
#include <vector>

namespace GWIN
{
    class GWConsole
    {
    public:
        GWConsole();

        ~GWConsole();

        void draw(bool *p_open);
    private:
         
        bool autoScroll{true};
    };
}