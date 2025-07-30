#pragma once

#include<pcb/board.hpp>
#include<base.hpp>
#include<Graffiti.h>
#include<pcb3d/builder3d.hpp>
#include<pcb3d/triangulation.hpp>
#include<math.h>
namespace PCBBuilder {

    struct LineLet {
        glm::vec2 point[2];
        float height[2];
        float angle[2];  //0��ͷ�ڵ㸽��һȦ�ĵ���Ҫ���ɵĽǶ�ֵ��1��tail����һȦ��
        double width;
    };
    class LineLets {
    public:
        std::vector< LineLet > m_LineLets;
        std::string m_ID;
        std::shared_ptr<Graffiti::StorageBuffer> buffer;
    };
    struct CircleLet {
 
        glm::vec2 center;
        float height[2];
        float r[2];
    };

    struct AreaShell {
        ShellType m_ShellType;
        std::vector<Graffiti::Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;

        std::shared_ptr <Graffiti::VertexArray> m_VertexArray;
    };

    class CircleLets {
    public:
        ShellType m_ShellType = ShellType::Circle;
        std::vector< CircleLet > m_CircleLets;
        std::string m_ID;
        std::shared_ptr<Graffiti::StorageBuffer> buffer;
    };
    class GPULayer {
    public:

        //Բ��
        CircleLets CircleLetmodels;

        //��
        LineLets   LineLetmodels;

        //����
       AreaShell    Areamodel;
       AreaShell    Arcmodel;
    };

    class GPUModel {
   
    public:
        GPUModel();
 
        int m_ID = -1;
        int m_LayerCount;
        std::vector<GPULayer> m_Layer;
        Graffiti::BoundingBox boundingbox;
   
    };
    class GPUBuilder {
    public:
        void Create_GPUModel(hwpcb::Board& board2d, GPUModel* model);
       
    private:

        static int modelname ;
        void create_GPUModelLayer(hwpcb::Layer& layer2d, GPULayer& layer, int layerNumber);

        void deal_PlaneLayer(hwpcb::PlaneLayer& planelayer, GPULayer& layer, int layerNumber);
        void deal_RouteLayer(hwpcb::RouteLayer& routelayer, GPULayer& layer, int layerNumber);
        void deal_MultiLayer(hwpcb::MultiLayer& layer2d, GPULayer& layer, int layerNumber);
        std::vector < std::thread> threads;
        std::vector < std::unique_ptr< std::mutex> > verticesMutex;
        const hwpcb::V2d* vertices2d;


        void tri_Areashell(hwpcb::PCBShell* shell, GPULayer& layerGPU, int layer=0);
        void tri_Arcshell(hwpcb::PCBShell* shell, GPULayer& layerGPU, int layer=0);
        void tri_Planeshell(hwpcb::PCBShell* shell, GPULayer& layerGPU,std::vector<double> TopHeight, std::vector<double> BaseHeight, int layer=0);

        LineLet create_LineLet(hwpcb::Seg s, double baseheight, double topheight) {
           
            hwpcb::V2d h = vertices2d[s.headVid];
            hwpcb::V2d t = vertices2d[s.tailVid];
            
            float theangle = angle(h, t);   
            LineLet let{
                {glm::vec2(h.x,h.y),glm::vec2(t.x,t.y) },
                {baseheight,topheight},
                {theangle + M_PI_2,theangle - M_PI_2},
                s.width / 2
            };
         
            return let;
        }
        CircleLet create_CircleLet(hwpcb::Shape s, double baseheight, double topheight) {


            hwpcb::V2d c = vertices2d[s.trail(0).seg(0).centerVid];
            CircleLet let{
                glm::vec2(c.x,c.y),
                {baseheight,topheight},
                {s.trail(0).seg(0).radius,s.trail(1).seg(0).radius}
            };

            return let;
        }
        float angle(hwpcb::V2d h, hwpcb::V2d t) {

            double r = sqrt((h.x - t.x) * (h.x - t.x) + (h.y - t.y) * (h.y - t.y));
            //��������ֵ
            double sin = (t.y - h.y) / r;

            //�����ȣ���Ϊ�������� sin��ֵ����Ϊ1.000001���³������
            if (sin > 1) sin = 1;
            else if (sin < -1) sin = -1;

            float theangle = asin(sin);

            //��������ֵ�����ǶȻ��ֵ���ͬ��������
            //��һ���޲��ô���
            if ((t.x - h.x) >= 0) {
                // �������޴���
                if (theangle < 0)
                    theangle = theangle + M_PI * 2;

                //�ڶ�,�������޴���ʽ��ͬ
            }
            else {
                theangle = M_PI - theangle;
            }

            return theangle;
        }
    };

}