#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWSwapChain.hpp"
#include "../EC/GWDescriptors.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

// for fonts
#include <imgui/misc/freetype/imgui_freetype.h>

namespace GWIN
{
    class GWInterface
    {
    public:
        GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat);
        ~GWInterface();

        void newFrame();
        void render(VkCommandBuffer commandBuffer);
    private:
        GWindow& window;
        GWinDevice& device;

        void initializeGUI(VkFormat imageFormat);

        std::unique_ptr<GWDescriptorPool> guipool;
    };
}