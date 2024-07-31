#include "InterfaceSystem.hpp"

namespace GWIN
{
    GWInterface::GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat, std::unique_ptr<GWTextureHandler>& textureHandler) : window(window), device(device), textureHandler(textureHandler)
    {
        initializeGUI(imageFormat);
    }

    GWInterface::~GWInterface()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void GWInterface::initializeGUI(VkFormat imageFormat)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        guipool = GWDescriptorPool::Builder(device)
                      .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                      .build();

        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.phyDevice();
        init_info.Device = device.device();
        init_info.Queue = device.graphicsQueue();
        init_info.DescriptorPool = guipool->getDescriptorPool();
        init_info.MinImageCount = 2;
        init_info.ImageCount = GWinSwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.UseDynamicRendering = true;
        init_info.Subpass = 0;

        init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imageFormat;

        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void GWInterface::setLoadGameObjectCallback(std::function<void(GameObjectInfo& objectInfo)> callback)
    {
        loadGameObjectCallback = callback;
    }

    void GWInterface::setCreateTextureCallback(std::function<void(VkDescriptorSet& set, Texture& texture)> callback)
    {
        createTextureCallback = callback;
    }

    //Temporary to load Object
    char filePathBuffer[256];
    char texturePathBuffer[256];
    float Objscale = 1,f;
    float Objposition[3] = {0.f, 0.f, 0.f};
    VkDescriptorSet texture = VK_NULL_HANDLE;

    void GWInterface::drawFileDialog()
    {
        if (ImGui::Button("Choose File"))
        {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj,.png,.jpg", config);
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                if (ImGuiFileDialog::Instance()->GetCurrentFilter() == ".obj")
                {
                    strncpy_s(filePathBuffer, filePathName.c_str(), sizeof(filePathBuffer) - 1);
                    filePathBuffer[sizeof(filePathBuffer) - 1] = '\0';
                } else {
                    strncpy_s(texturePathBuffer, filePathName.c_str(), sizeof(texturePathBuffer) - 1);
                    texturePathBuffer[sizeof(texturePathBuffer) - 1] = '\0';
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::Text("Selected file: %s", filePathBuffer);
        ImGui::Text("Selected Texture file: %s", texturePathBuffer);
    }

    void GWInterface::drawImGuizmo(FrameInfo &frameInfo)
    {
       
    }

    void GWInterface::newFrame(FrameInfo &frameInfo)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();

        //Top Menu
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project"))
                {
                }
                if (ImGui::MenuItem("Open Project", "Ctrl+O"))
                {
                }
                if (ImGui::MenuItem("Save Project", "Ctrl+S"))
                {
                }
                if (ImGui::MenuItem("Save Project As.."))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Load new Object"))
                {
                    showCreateObjectWindow = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z"))
                {
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y"))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
                {
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C"))
                {
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V"))
                {
                }
                if (ImGui::MenuItem("Delete", "Del"))
                {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Zoom In", "Ctrl++"))
                {
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-"))
                {
                }
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0"))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Fullscreen", "F11"))
                {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Documentation"))
                {
                }
                if (ImGui::MenuItem("About"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
        ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::DockSpace(ImGui::GetID("Window"), ImVec2(ImGui::GetWindowSize()));
        ImGui::End();

        //Object List and Properties
        objectList.Draw(frameInfo);

        //Draw's Console
        console.draw(frameInfo);

        //Draw's ImGuizmo Actions
        drawImGuizmo(frameInfo);

        //Temporary Object Loader
        if (showCreateObjectWindow)
        {
            if (ImGui::Begin("Create New Object", &showCreateObjectWindow, ImGuiWindowFlags_AlwaysAutoResize))
            {
                drawFileDialog();
                ImGui::SliderFloat("Scale", &Objscale, 0.1f, 10.f);
                ImGui::InputFloat3("Position", Objposition);
                if (ImGui::Button("Load Object"))
                {
                    if (loadGameObjectCallback)
                    {
                        Texture createTexture = textureHandler->createTexture(std::string(texturePathBuffer));
                        createTextureCallback(texture, createTexture);

                        GameObjectInfo objectInfo{"DefaultName", filePathBuffer, Objscale, {Objposition[0], Objposition[1] * -1, Objposition[2]}, texture};
                        loadGameObjectCallback(objectInfo);

                        texture = VK_NULL_HANDLE;
                    }
                }
                ImGui::End();
            }
        }
    }

    void GWInterface::render(VkCommandBuffer commandBuffer)
    {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
}