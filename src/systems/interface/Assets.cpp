#include "Assets.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace GWIN
{
    AssetsWindow::AssetsWindow(GWTextureHandler& imageLoader) : imageLoader(imageLoader)
    {
        createDefaultImages();
    }

    AssetsWindow::~AssetsWindow()
    {
        for (auto &image : images)
        {
            ImGui_ImplVulkan_RemoveTexture(image.second);
        }
    }

    void AssetsWindow::actions(AssetType type)
    {

    }

    void AssetsWindow::draw()
    {
        if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoResize);
            for (const auto &image : images)
            {

            }
            ImGui::EndChild();

        } ImGui::End();
    }

    VkDescriptorSet AssetsWindow::createImage(const std::string &pathToFile)
    {
        Image NewImage = imageLoader.getImageLoader().loadImage(pathToFile, false);

        VkSampler sampler;

        imageLoader.createSampler(1, sampler);

        return ImGui_ImplVulkan_AddTexture(sampler, NewImage.imageView, NewImage.layout);
    }

    void AssetsWindow::createDefaultImages()
    {
        std::string relativePath = "src/systems/interface/images";
        fs::path imageDirectory = fs::current_path() / relativePath;

        if (fs::exists(imageDirectory) && fs::is_directory(imageDirectory))
        {
            for (const auto &entry : fs::directory_iterator(imageDirectory))
            {
                if (entry.is_regular_file())
                {
                    std::string path = entry.path().string();

                    images.emplace(entry.path().filename().string(), createImage(path));
                }
            }
        }
    }

    void AssetsWindow::materialEditor()
    {

    }
}