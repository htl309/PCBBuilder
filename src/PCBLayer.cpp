#include"PCBLayer.hpp"

namespace Graffiti {
    PCBLayer::PCBLayer()
        :Layer("PCBLayer"), m_StandardPipeLine("Standard"), m_Hybrid_Line("Hybrid_Line")
        , m_Hybrid_Circle("Hybrid_Circle") {

        m_ShaderLibrary = std::make_shared<ShaderLibrary>();
        //Bunny模型加载//////////////////////////////////////////////////////////////////////////////////

  
        auto shader = Shader::Create(m_StandardPipeLine, {
                {ShaderType::VertexShader, "Standard/Standard.vert"},
                {ShaderType::FragmentShader, "Standard/Standard.frag"}
            }
        );

        shader->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::DepthTestDisable));
        shader->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::WireFrame|PipelineState::DepthTestDisable));
        m_ShaderLibrary->Add(m_StandardPipeLine, shader);
        m_ShaderLibrary->Get(m_StandardPipeLine)->Link(); 

        auto shader2 = Shader::Create(m_Hybrid_Line, {
                {ShaderType::TaskShader, "Hybrid/LineLet.task"},
               {ShaderType::MeshShader, "Hybrid/LineLet.mesh"},
               {ShaderType::FragmentShader, "Hybrid/LineLet.frag"}
            }
        );
        shader2->UploadStorageBuffer("LineLets", sizeof(PCBBuilder::LineLet), 1, 1, 0);
        shader2->UploadStorageBuffer("Index1", sizeof(uint8_t), 1, 1, 1);
        shader2->UploadStorageBuffer("Index2", sizeof(uint8_t), 1, 1, 2);

        shader2->ClearPipeline();
       
        shader2->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::DepthTestDisable|PipelineState::MeshShaderPipeLine));
        shader2->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::WireFrame | PipelineState::DepthTestDisable | PipelineState::MeshShaderPipeLine));
        m_ShaderLibrary->Add(m_Hybrid_Line, shader2);
        m_ShaderLibrary->Get(m_Hybrid_Line)->Link();
  
        auto shader3 = Shader::Create(m_Hybrid_Circle, {
                {ShaderType::TaskShader, "Hybrid/CircleLet.task"},
               {ShaderType::MeshShader, "Hybrid/CircleLet.mesh"},
               {ShaderType::FragmentShader, "Hybrid/CircleLet.frag"}
            }
        );
        shader3->UploadStorageBuffer("CircleLets", sizeof(PCBBuilder::CircleLet), 1, 1, 0);
        shader3->UploadStorageBuffer("CircleIndex1", sizeof(uint8_t), 1, 1, 1);
        shader3->UploadStorageBuffer("CircleIndex2", sizeof(uint8_t), 1, 1, 2);
        shader3->UploadStorageBuffer("CircleIndex3", sizeof(uint8_t), 1, 1, 3);
     
        shader3->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::DepthTestDisable | PipelineState::MeshShaderPipeLine));
        shader3->AddPipeline(std::make_shared<PipelineConfigInfo>(PipelineState::WireFrame | PipelineState::DepthTestDisable | PipelineState::MeshShaderPipeLine));
        m_ShaderLibrary->Add(m_Hybrid_Circle, shader3);
        m_ShaderLibrary->Get(m_Hybrid_Circle)->Link();


        PCBBuilder::PCBLoader loader;
        cpumodel = new PCBBuilder::CPUModel();
        gpumodel = new PCBBuilder::GPUModel();
        loader.Create_CPUTestModel(cpumodel);
        loader.Create_GPUTestModel(gpumodel);
        m_CameraControl.SetTarget(cpumodel->boundingbox);
    }
    void PCBLayer::OnImGuiRender()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 10.0f;  // 设置窗口的圆角半径为 10 像素
        style.FrameRounding = 5.0f;    // 设置框架的圆角半径为 5 像素（比如按钮等UI元素）
        style.ChildRounding = 5.0f;    // 设置子窗口的圆角

        ImGui::Begin("PCBBuilder");
        ImGui::SetWindowPos(ImVec2(40, 190), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(420, 225), ImGuiCond_Always);


        ImGui::Checkbox("WireFrameMode", &m_Guisets.IsWireFrame);
        ImGui::SameLine();
        ImGui::Checkbox("LetColor", &m_Guisets.LetColor);

        ImGui::PushItemWidth(180);
        ImGui::Combo("Model", &m_Guisets.model, m_Guisets.models_name, 8);
        ImGui::PushItemWidth(180);
        ImGui::Combo("Pipeline", &m_Guisets.pipeline, m_Guisets.pipeline_name, 2);

        ImGui::Text("Loading time :");
        ImGui::Text("Standard:%0.2f", m_Guisets.loadingtime_standard);
        ImGui::SameLine();
        ImGui::Text("s|");
        ImGui::SameLine();
        ImGui::Text("Hybrid:%0.2f", m_Guisets.loadingtime_hybrid);
        ImGui::SameLine();
        ImGui::Text("s");



        ImGui::End();

        ImGui::Begin("Layers");
        ImGui::SetWindowPos(ImVec2(40, 425), ImGuiCond_Once);

        if (ImGui::Button("ALL")) { for (int i = 0; i < 18; i++) m_Guisets.displaylayer[i] = 1; }
        ImGui::SameLine();
        if (ImGui::Button("NONE")) { for (int i = 0; i < 18; i++) m_Guisets.displaylayer[i] = 0; }

        if (m_Guisets.layersize > 0)
            for (int i = 0; i < m_Guisets.layersize; i++) {
                std::string label =
                    "Layer" + std::to_string(i); // 生成一个标签，例如 "Layer 0"
                ImGui::Checkbox(label.c_str(), &m_Guisets.displaylayer[i]);

            }
        ImGui::SetWindowSize(ImVec2(155, 85 + 36 * m_Guisets.layersize), ImGuiCond_Always);

        ImGui::End();
    }
    void PCBLayer::OnUpdate(TimeStep& ts)
    {
    
        if (!ImGui::GetIO().WantCaptureMouse) {
            // 只有在 ImGui 没有占用鼠标时，才允许相机控制逻辑生效
            m_CameraControl.KeyMovement(ts);
            m_CameraControl.MouseMovement();
        }

        m_Scene.ViewProjectionMatrix = m_CameraControl.GetViewProjectionMatrix();

        Render::BeginScene(m_Scene);
        Render::SetMode(m_Guisets.IsWireFrame);
        RenderCommand::SetDepthtest(0);


        if (m_Guisets.pipeline == 0)
        {
          
            LoadCPUModel();          
            RenderCPUModel();
        }
        else if (m_Guisets.pipeline == 1) {
        
            LoadGPUModel();   
            RenderGPUModel();
        }
     
        Render::EndScene();   
    }
    void PCBLayer::LoadCPUModel()
    {

        if (m_Guisets.model != cpumodel->m_ID) {


            delete cpumodel;
            auto timestart = std::chrono::high_resolution_clock::now();

            cpumodel = new PCBBuilder::CPUModel();
            PCBBuilder::PCBLoader loader;
            if (m_Guisets.model == 0) { loader.Create_CPUTestModel(cpumodel); }
            else if (m_Guisets.model == 1) { loader.Create_CPUModel("Models/t1.txt", cpumodel); }
            else if (m_Guisets.model == 2) { loader.Create_CPUModel("Models/t2.txt", cpumodel); }
            else if (m_Guisets.model == 3) { loader.Create_CPUModel("Models/t3.txt", cpumodel); }
            else if (m_Guisets.model == 4) { loader.Create_CPUModel("Models/t4.txt", cpumodel); }
            else if (m_Guisets.model == 5) { loader.Create_CPUModel("Models/t5.txt", cpumodel); }
            else if (m_Guisets.model == 6) { loader.Create_CPUModel("Models/t6.txt", cpumodel); }
            else if (m_Guisets.model == 7) { loader.Create_CPUModel("Models/t7.txt", cpumodel); }


            LoadCPUVertexArray();
            cpumodel->m_ID = m_Guisets.model;
            m_Guisets.layersize = cpumodel->m_LayerCount;
            m_CameraControl.SetTarget(cpumodel->boundingbox);

            auto timeend = std::chrono::high_resolution_clock::now();
            m_Guisets.loadingtime_standard = std::chrono::duration<float, std::chrono::seconds::period>(timeend - timestart).count();

        }


    }
    void PCBLayer::RenderCPUModel()
    {
        for (int i = 0; i < m_PCBVertexArray.size(); i++) {
            if (m_Guisets.displaylayer[i] == 1)
                for (int j = 0; j < m_PCBVertexArray[i].size(); j++) {

                    if (m_Guisets.LetColor == 1) {
                        m_pushcontdata.type = 5;
                    }
                    else {
                        m_pushcontdata.type = cpumodel->m_Layer[i][j].m_ShellType;
                    }
                    Render::Submit(m_PCBVertexArray[i][j], m_ShaderLibrary->Get(m_StandardPipeLine), m_pushcontdata);

                }
        }
    }
    void PCBLayer::LoadCPUVertexArray()
    {
        for (int i = 0; i < m_PCBVertexArray.size(); i++) {
            m_PCBVertexArray[i].clear();
        }
        m_PCBVertexArray.clear();

        m_PCBVertexArray.resize(cpumodel->m_LayerCount);
        for (int i = 0; i < cpumodel->m_LayerCount; i++) {
            auto size = cpumodel->m_Layer[i].size();
            m_PCBVertexArray[i].resize(size);
            for (int j = 0; j < size; j++)
            {
                m_PCBVertexArray[i][j] = VertexArray::Create();
                m_PCBVertexArray[i][j]->AddVertexBuffer(VertexBuffer::Create(cpumodel->m_Layer[i][j].m_Vertices, cpumodel->m_Layer[i][j].m_Vertices.size()));
                m_PCBVertexArray[i][j]->AddIndexBuffer(IndexBuffer::Create(cpumodel->m_Layer[i][j].m_Indices, cpumodel->m_Layer[i][j].m_Indices.size()));

            }
        }
    }
    void PCBLayer::LoadGPUModel() {

        if(modeli)
        {
            if (m_Guisets.model != lastGPUModel) {
                modeli = !modeli;
                lastGPUModel = m_Guisets.model;
    
                auto timestart = std::chrono::high_resolution_clock::now();
                delete gpumodel;
                gpumodel = new PCBBuilder::GPUModel();
                PCBBuilder::PCBLoader loader;

                if (m_Guisets.model == 0) { loader.Create_GPUTestModel(gpumodel); }
                else if (m_Guisets.model == 1) { loader.Create_GPUModel("Models/t1.txt", gpumodel); }
                else if (m_Guisets.model == 2) { loader.Create_GPUModel("Models/t2.txt", gpumodel); }
                else if (m_Guisets.model == 3) { loader.Create_GPUModel("Models/t3.txt", gpumodel); }
                else if (m_Guisets.model == 4) { loader.Create_GPUModel("Models/t4.txt", gpumodel); }
                else if (m_Guisets.model == 5) { loader.Create_GPUModel("Models/t5.txt", gpumodel); }
                else if (m_Guisets.model == 6) { loader.Create_GPUModel("Models/t6.txt", gpumodel); }
                else if (m_Guisets.model == 7) { loader.Create_GPUModel("Models/t7.txt", gpumodel); }


                gpumodel->m_ID = m_Guisets.model;
                m_Guisets.layersize = gpumodel->m_LayerCount;
                m_CameraControl.SetTarget(gpumodel->boundingbox);


                auto timeend = std::chrono::high_resolution_clock::now();
                m_Guisets.loadingtime_hybrid = std::chrono::duration<float, std::chrono::seconds::period>(timeend - timestart).count();

            }
        }
        else {
         
            if (m_Guisets.model != lastGPUModel) {
                modeli = !modeli;
                lastGPUModel = m_Guisets.model;
             
                auto timestart = std::chrono::high_resolution_clock::now();
                delete gpumodel2;
                gpumodel2 = new PCBBuilder::GPUModel();
                PCBBuilder::PCBLoader loader;

                if (m_Guisets.model == 0) { loader.Create_GPUTestModel(gpumodel2); }
                else if (m_Guisets.model == 1) { loader.Create_GPUModel("Models/t1.txt", gpumodel2); }
                else if (m_Guisets.model == 2) { loader.Create_GPUModel("Models/t2.txt", gpumodel2); }
                else if (m_Guisets.model == 3) { loader.Create_GPUModel("Models/t3.txt", gpumodel2); }
                else if (m_Guisets.model == 4) { loader.Create_GPUModel("Models/t4.txt", gpumodel2); }
                else if (m_Guisets.model == 5) { loader.Create_GPUModel("Models/t5.txt", gpumodel2); }
                else if (m_Guisets.model == 6) { loader.Create_GPUModel("Models/t6.txt", gpumodel2); }
                else if (m_Guisets.model == 7) { loader.Create_GPUModel("Models/t7.txt", gpumodel2); }


                gpumodel2->m_ID = m_Guisets.model;
                m_Guisets.layersize = gpumodel2->m_LayerCount;
                m_CameraControl.SetTarget(gpumodel2->boundingbox);


                auto timeend = std::chrono::high_resolution_clock::now();
                m_Guisets.loadingtime_hybrid = std::chrono::duration<float, std::chrono::seconds::period>(timeend - timestart).count();

            }
        }
    }
    void PCBLayer::RenderGPUModel()
    {
      
        if(!modeli)
        {
            for (int i = 0; i < gpumodel->m_LayerCount; i++) {
                if (m_Guisets.displaylayer[i] == 1) {


                    RenderCommand::SetMeshShader(true);
                    if (gpumodel->m_Layer[i].LineLetmodels.m_LineLets.size() > 0) {
                        if (m_Guisets.LetColor == 1) m_pushcontdata.type = 0;
                        else  m_pushcontdata.type = 1;
                        m_pushcontdata.letsize = gpumodel->m_Layer[i].LineLetmodels.m_LineLets.size();



                        m_ShaderLibrary->Get(m_Hybrid_Line)->PCBBind(gpumodel->m_Layer[i].LineLetmodels.m_ID);
                        MeshShaderSubmit((gpumodel->m_Layer[i].LineLetmodels.m_LineLets.size() + 31) / 32,
                            m_ShaderLibrary->Get(m_Hybrid_Line),
                            m_pushcontdata);
                    }

                    uint32_t circleLetsize = gpumodel->m_Layer[i].CircleLetmodels.m_CircleLets.size();
                    if (circleLetsize > 0) {
                        if (m_Guisets.LetColor == 1) m_pushcontdata.type = 0;

                        else {
                            if (gpumodel->m_Layer[i].CircleLetmodels.m_ShellType == PCBBuilder::ShellType::Mutil)m_pushcontdata.type = 2;
                            else  m_pushcontdata.type = 1;
                        }
                        m_pushcontdata.letsize = circleLetsize;

                        m_ShaderLibrary->Get(m_Hybrid_Circle)->PCBBind(gpumodel->m_Layer[i].CircleLetmodels.m_ID);
                        MeshShaderSubmit((circleLetsize + 31) / 32,
                            m_ShaderLibrary->Get(m_Hybrid_Circle),
                            m_pushcontdata);
                    }
                    RenderCommand::SetMeshShader(false);


                    uint32_t arcsize = gpumodel->m_Layer[i].Arcmodel.m_Indices.size();
                    if (arcsize > 0) {
                        if (m_Guisets.LetColor == 1) {
                            m_pushcontdata.type = 5;
                        }
                        else {
                            m_pushcontdata.type = gpumodel->m_Layer[i].Arcmodel.m_ShellType;
                        }
                        Render::Submit(gpumodel->m_Layer[i].Arcmodel.m_VertexArray, m_ShaderLibrary->Get(m_StandardPipeLine), m_pushcontdata);
                    }
                    uint32_t areasize = gpumodel->m_Layer[i].Areamodel.m_Indices.size();
                    if (areasize > 0) {
                        if (m_Guisets.LetColor == 1) {
                            m_pushcontdata.type = 5;
                        }
                        else {
                            m_pushcontdata.type = gpumodel->m_Layer[i].Areamodel.m_ShellType;
                        }
                        Render::Submit(gpumodel->m_Layer[i].Areamodel.m_VertexArray, m_ShaderLibrary->Get(m_StandardPipeLine), m_pushcontdata);

                    }

                }
            }
        }
        else {
            for (int i = 0; i < gpumodel2->m_LayerCount; i++) {
                if (m_Guisets.displaylayer[i] == 1) {


                    RenderCommand::SetMeshShader(true);
                    if (gpumodel2->m_Layer[i].LineLetmodels.m_LineLets.size() > 0) {
                        if (m_Guisets.LetColor == 1) m_pushcontdata.type = 0;
                        else  m_pushcontdata.type = 1;
                        m_pushcontdata.letsize = gpumodel2->m_Layer[i].LineLetmodels.m_LineLets.size();

                        m_ShaderLibrary->Get(m_Hybrid_Line)->PCBBind(gpumodel2->m_Layer[i].LineLetmodels.m_ID);

                        MeshShaderSubmit((gpumodel2->m_Layer[i].LineLetmodels.m_LineLets.size() + 31) / 32,
                            m_ShaderLibrary->Get(m_Hybrid_Line),
                            m_pushcontdata); 
                    }

                    uint32_t circleLetsize = gpumodel2->m_Layer[i].CircleLetmodels.m_CircleLets.size();
                    if (circleLetsize > 0) {
                        if (m_Guisets.LetColor == 1) m_pushcontdata.type = 0;

                        else {
                            if (gpumodel2->m_Layer[i].CircleLetmodels.m_ShellType == PCBBuilder::ShellType::Mutil)m_pushcontdata.type = 2;
                            else  m_pushcontdata.type = 1;
                        }
                        m_pushcontdata.letsize = circleLetsize;

                        m_ShaderLibrary->Get(m_Hybrid_Circle)->PCBBind(gpumodel2->m_Layer[i].CircleLetmodels.m_ID);
                        MeshShaderSubmit((circleLetsize + 31) / 32,
                            m_ShaderLibrary->Get(m_Hybrid_Circle),
                            m_pushcontdata);
                    }
                    RenderCommand::SetMeshShader(false);


                    uint32_t arcsize = gpumodel2->m_Layer[i].Arcmodel.m_Indices.size();
                    if (arcsize > 0) {
                        if (m_Guisets.LetColor == 1) {
                            m_pushcontdata.type = 5;
                        }
                        else {
                            m_pushcontdata.type = gpumodel2->m_Layer[i].Arcmodel.m_ShellType;
                        }
                        Render::Submit(gpumodel2->m_Layer[i].Arcmodel.m_VertexArray, m_ShaderLibrary->Get(m_StandardPipeLine), m_pushcontdata);
                    }
                    uint32_t areasize = gpumodel2->m_Layer[i].Areamodel.m_Indices.size();
                    if (areasize > 0) {
                        if (m_Guisets.LetColor == 1) {
                            m_pushcontdata.type = 5;
                        }
                        else {
                            m_pushcontdata.type = gpumodel2->m_Layer[i].Areamodel.m_ShellType;
                        }
                        Render::Submit(gpumodel2->m_Layer[i].Areamodel.m_VertexArray, m_ShaderLibrary->Get(m_StandardPipeLine), m_pushcontdata);

                    }

                }
            }
        }
    }
    void PCBLayer::MeshShaderSubmit(uint32_t taskcount, const std::shared_ptr<Shader>& shader, const PushconstData& pushconst)
    {

        shader->SetSceneData(m_Scene);
        shader->SetTransform(pushconst);
        RenderCommand::MeshShaderDraw(taskcount);
    }
}