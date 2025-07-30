#pragma once

#include<pcb3d/triangulation.hpp>
#include<pcb/board.hpp>
#include<pcb/layer_parser.hpp>
#include <pcb3d/builder3d.hpp>
#include<Graffiti.h>

#include"base.hpp"
#include"BuildGPU.hpp"
namespace PCBBuilder {


	struct CPULayerModel {
		ShellType m_ShellType;
		std::vector<Graffiti::Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
	
	using CPULayer = std::vector<CPULayerModel> ;
	class CPUModel {
		public:
         ~CPUModel() {};
		int m_ID=-1;
		int m_LayerCount;
		std::vector<CPULayer> m_Layer;
		Graffiti::BoundingBox boundingbox;
	private:

	};

	class PCBLoader {
	public:
		void Create_CPUModel(std::string file, CPUModel* model);
        void Create_CPUTestModel(CPUModel* model);

        void Create_GPUTestModel(GPUModel* model);
        void Create_GPUModel(std::string file, GPUModel* model);
        //void Create_GPUModel(std::string file, hwpcb::GPUModel* model);

    private:
        void create_Layer(hwpcb::Layer3D* layer3d, CPULayer& layer);
        hwpcb::Board create_Board2D(std::string path) {

            hwpcb::Board board2d;
            std::ifstream ifs(path.c_str(), std::ios::binary);
            auto parser = hwpcb::LayerParser::make(ifs);
            int k = 0;
            while (1) {
                try {
                    auto layer = parser->parse();
                    if (layer) {
                        board2d.addLayer(layer);
                        ++k;
                    }
                    else {
                        break;
                    }
                }
                catch (const std::runtime_error& ex) {
                    std::cerr << ex.what() << std::endl;
                }
            }




            return  board2d;
        }
	};

}