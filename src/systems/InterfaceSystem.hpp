#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWSwapChain.hpp"
#include "../EC/GWDescriptors.hpp"
#include "../EC/GWFrameInfo.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include <functional>

// pre-made interfaces
#include "./interface/Console.hpp"

// for fonts
#include <imgui/misc/freetype/imgui_freetype.h>

namespace GWIN
{
    class GWInterface
    {
    public:
        GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat);
        ~GWInterface();

        void newFrame(FrameInfo& frameInfo);
        void render(VkCommandBuffer commandBuffer);

        void setLoadGameObjectCallback(std::function<void(GameObjectInfo& objectInfo)> callback);

    private:
        GWindow& window;
        GWinDevice& device;

        std::function<void(GameObjectInfo &objectInfo)> loadGameObjectCallback;

        void initializeGUI(VkFormat imageFormat);
        void showConsole();

        GWConsole console{};
        bool show_console{false};
        bool showCreateObjectWindow{false};

        std::unique_ptr<GWDescriptorPool> guipool;
    };
}