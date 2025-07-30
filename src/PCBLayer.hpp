#pragma once

#include<Graffiti.h>
#include"PCBLoader.hpp"
#include"BuildGPU.hpp"
namespace Graffiti {
    struct PCBGuiSet {
        bool IsWireFrame = 0;
        bool LetColor = 0;

        //0��StandardPipeLine��1��HybridPipeLine
        //��ʼ�Ǳ�׼���ߣ������Ӿ����ǲ�֧��meshshader�ܹ��ĵ���Ҳ������
        //Ҫ�ǲ�֧��meshshader����ô�ڽ�������ѡ��hybrid��ʱ��ͻ����
        int pipeline = 0;

        //0~7�˸�ģ�ͣ�0�ǲ���ģ��
        //��ʼ��testmodel
        int model = 0;

        float loadingtime_standard = 0;
        float loadingtime_hybrid = 0;

        //��ʾ��Щ�㣬Ĭ��ȫ����ʾ
        bool displaylayer[18] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
        const char* pipeline_name[2] = { "Standard", "Hybrid" };
        const char* models_name[8] = { "pcb:test","pcb:t1","pcb:t2","pcb:t3","pcb:t4","pcb:t5","pcb:t6","pcb:t7" };

        //��Ӧtestmodel��laysize
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