#pragma once

#include<Graffiti.h>
#include"PCBLoader.hpp"
#include"BuildGPU.hpp"
namespace Graffiti {
    struct PCBGuiSet {
        bool IsWireFrame = 0;
        bool LetColor = 0;

        //0是StandardPipeLine，1是HybridPipeLine
        //初始是标准管线，这样子就算是不支持meshshader架构的电脑也能运行
        //要是不支持meshshader，那么在交互界面选择hybrid的时候就会崩溃
        int pipeline = 0;

        //0~7八个模型，0是测试模型
        //初始是testmodel
        int model = 0;

        float loadingtime_standard = 0;
        float loadingtime_hybrid = 0;

        //显示哪些层，默认全部显示
        bool displaylayer[18] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
        const char* pipeline_name[2] = { "Standard", "Hybrid" };
        const char* models_name[8] = { "pcb:test","pcb:t1","pcb:t2","pcb:t3","pcb:t4","pcb:t5","pcb:t6","pcb:t7" };

        //对应testmodel的laysize
        int layersize = 2;
    };

    class PCBLayer : public Layer {
    public:

        PCBLayer();

        void OnImGuiRender() override;
        void OnUpdate(TimeStep& ts) override;


    private:
        
        std::string m_StandardPipeLine;
        std::string m_Hybrid_Line;
        std::string m_Hybrid_Circle;


        CameraControl m_CameraControl;
        std::shared_ptr < ShaderLibrary> m_ShaderLibrary;


        SceneData m_Scene;
        PushconstData m_pushcontdata;

        PCBGuiSet m_Guisets;
      

        PCBBuilder::CPUModel* cpumodel;
        std::vector <std::vector< std::shared_ptr <VertexArray>>> m_PCBVertexArray;

        int lastGPUModel = -1;
        PCBBuilder::GPUModel* gpumodel = nullptr;;
        PCBBuilder::GPUModel* gpumodel2 = nullptr;;
        bool modeli = 0;


        void OnEvent(Event& event)
        {
            m_CameraControl.OnEvent(event);
        }
        void LoadCPUModel();

        void RenderCPUModel();
        void LoadCPUVertexArray();
        void LoadGPUModel();
        void RenderGPUModel();
      
        void MeshShaderSubmit(uint32_t taskcount, const std::shared_ptr<Shader>& shader, const PushconstData& pushconst);
    };

}