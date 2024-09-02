#include "InterfaceSystem.hpp"

namespace GWIN
{
    GWInterface::GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat, std::unique_ptr<GWTextureHandler>& textureHandler, 
    std::unique_ptr<GWMaterialHandler>& materialHandler) : window(window), device(device), textureHandler(textureHandler), materialHandler(materialHandler)
    {
        initializeGUI(imageFormat);
        assets = std::make_unique<AssetsWindow>(textureHandler, materialHandler);
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

        ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

        ImGui::StyleColorsDark();

        guipool = GWDescriptorPool::Builder(device)
                      .setMaxSets(1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
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

    void GWInterface::setCreateTextureCallback(std::function<void(VkDescriptorSet& set, Texture& texture)> callback)
    {
        createTextureCallback = callback;
    }

    void GWInterface::setSaveSceneCallback(std::function<void(const std::string path)> callback)
    {
        SaveSceneCallback = callback;
    }

    void GWInterface::setLoadSceneCallback(std::function<void(const std::string path)> callback)
    {
        LoadSceneCallback = callback;
    }

    //Temporary to load Object
    char filePathBuffer[256];
    char texturePathBuffer[256] = "C:\\Users\\cleve\\OneDrive\\Documents\\GitHub\\GabexEngine\\src\\textures\\no_texture.png";
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

    void GWInterface::drawImGuizmo(FrameInfo &frameInfo, ImDrawList* drawList)
    {
        uint32_t selectedObject = objectList.getSelectedObject();
        if (selectedObject != -1 && !objectList.isAssetSelected())
        {
            ImGuizmo::BeginFrame();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist(drawList);

            GWGameObject &gameObject = frameInfo.currentInfo.gameObjects.at(selectedObject);

            glm::mat4 transformMatrix = gameObject.transform.mat4();

            glm::mat4 view = frameInfo.currentInfo.currentCamera.getView();
            glm::mat4 projection = frameInfo.currentInfo.currentCamera.getProjection();

            projection[1][1] *= -1;

            ImGuizmo::AllowAxisFlip(true);
            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                mCurrentGizmoOperation, ImGuizmo::MODE::WORLD,
                                glm::value_ptr(transformMatrix));

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformMatrix),
                                                    glm::value_ptr(translation),
                                                    glm::value_ptr(rotation),
                                                    glm::value_ptr(scale));

                if (glm::any(glm::epsilonNotEqual(gameObject.transform.getRotation(), rotation, glm::epsilon<float>())))
                {
                    gameObject.transform.rotateEuler(rotation);
                }

                if(glm::any(glm::epsilonNotEqual(gameObject.transform.translation, translation, glm::epsilon<float>())))
                {
                    gameObject.transform.translation = translation;
                }

                if (mCurrentGizmoOperation == ImGuizmo::SCALE)
                {
                    float uniformScale = (scale.x + scale.y + scale.z) / 3.0f;
                    gameObject.transform.scale = uniformScale;
                    scale = glm::vec3(uniformScale);
                }
            }
        }
    }

    void GWInterface::drawSceneSettings()
    {
        if (ImGui::Begin("Scene Settings", nullptr))
        {
            ImGui::DragFloat("LightIntensity", &DirectionalLightingIntensity, .1f, 0.f, 10.f);
            ImGui::Checkbox("Render Shadows", &showShadows);
        }

        ImGui::End();
    }

    void GWInterface::newFrame(FrameInfo &frameInfo)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowDockID(ImGui::GetID("##Dockspace"));
        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar(2);
        ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

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
                    ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDialog", "Choose a Scene to Load", ".json");
                }
                if (ImGui::MenuItem("Save Project", "Ctrl+S"))
                {
                }
                if (ImGui::MenuItem("Save Project As.."))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("SaveProjectDialog", "Choose a Save Folder", nullptr);
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

        ImGui::End();

        objectList.Draw(frameInfo);
        console.draw(frameInfo);
        assets->draw();

        ImDrawList* drawList;

        if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            drawList = ImGui::GetWindowDrawList();

            auto& images = assets->getImages();

            int16_t buttonSize = 32;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));           // Darker background
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f)); // Highlight on hover
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));     // Highlight on active

            if (ImGui::ImageButton((ImTextureID)images.at("move"), ImVec2(buttonSize, buttonSize)))
            {
                mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            }
            ImGui::SameLine();
            if (ImGui::ImageButton((ImTextureID)images.at("rotate"), ImVec2(buttonSize, buttonSize)))
            {
                mCurrentGizmoOperation = ImGuizmo::ROTATE;
            }
            ImGui::SameLine();
            if (ImGui::ImageButton((ImTextureID)images.at("scale"), ImVec2(buttonSize, buttonSize)))
            {
                mCurrentGizmoOperation = ImGuizmo::SCALE;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            if (frameInfo.currentFrameSet)
            {
                ImGui::Image((ImTextureID)frameInfo.currentFrameSet, windowSize);
                
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImGuizmo::SetRect(windowPos.x, windowPos.y + buttonSize, windowSize.x, windowSize.y + buttonSize);
            }

            ImGui::End();
        }

            drawSceneSettings();
            drawImGuizmo(frameInfo, drawList);

            if (ImGuiFileDialog::Instance()->Display("SaveProjectDialog", ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    SaveSceneCallback(ImGuiFileDialog::Instance()->GetCurrentPath());
                }

                ImGuiFileDialog::Instance()->Close();
            }

            if (ImGuiFileDialog::Instance()->Display("OpenProjectDialog", ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    LoadSceneCallback(ImGuiFileDialog::Instance()->GetCurrentFileName());
                }

                ImGuiFileDialog::Instance()->Close();
            }
        }

    void GWInterface::render(VkCommandBuffer commandBuffer)
    {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
}