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
#include "./interface/ObjectList.hpp"

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

        GWConsole getConsole() const { return console; }

    private:
        GWindow& window;
        GWinDevice& device;

        std::function<void(GameObjectInfo &objectInfo)> loadGameObjectCallback;

        void initializeGUI(VkFormat imageFormat);

        GWObjectList objectList{};
        GWConsole console{};

        bool showCreateObjectWindow{false};

        std::unique_ptr<GWDescriptorPool> guipool;
    };
}