#include"PCBLoader.hpp"

namespace PCBBuilder {
    hwpcb::Board create_TestBoard2D() {
        std::string src = R"(

LAYERBEGIN

  LAYERTYPE,top
  BASEPLANEHEIGHT,10.0
  THICKNESS,100.0
  POINTCOUNT,16
  SEGCOUNT,6
  LINEBEGIN # 走线
    LINEID,10
    LINEDATA,4
    200.0,200.0 
    STRAIGHT,102,80.0,1000.0,200.0 
    STRAIGHT,103,80.0,600.0,892.80 
    STRAIGHT,104,80.0,200.0,200.0 
  LINEEND

  LINEBEGIN # 走线
    LINEID,11
    LINEDATA,2
    -1000.0,-400.0 
    ARC,0,120.000000, -400.0,-400.0 , -400.0,-1000.0 ,CCW
  LINEEND

LINEBEGIN # 走线
    LINEID,11
    LINEDATA,2
    -600.0,-400.0
    ARC,0,160.1600, -400.0,-400.0 ,-400.0,-600.0,CW
  LINEEND

    SHAPEBEGIN
    SHAPEID,104
    SHAPEDATA,5
    200.0,-200.0 
    STRAIGHT,114,0.0,200.0,-1000.0 
    STRAIGHT,113,0.0,1000.0,-1000.0 
    STRAIGHT,112,0.0,1000.0,-200.0 
    STRAIGHT,114,0.0,200.0,-200.0 
    VOIDDATA,1
    CIRCLE,0,0.000000,800.0,-800.0,200.0
    SHAPEEND

LAYEREND

LAYERBEGIN
    LAYERTYPE,Plane
    BASEPLANEHEIGHT,-80.0
    THICKNESS,80.0
    POINTCOUNT,6
    SEGCOUNT,0
    SHAPEBEGIN
    SHAPEID,18446744069414611799
    SHAPEDATA,1
    CIRCLE,0,0.000000,-600.00,600.00,447.2135954
    VOIDDATA,5
    -600.0,800.0 
    STRAIGHT,0,0.0,-400.0,600.0 
    STRAIGHT,0,0.0,-600.0,400.0 
    STRAIGHT,0,0.0,-800.0,600.0 
    STRAIGHT,0,0.0, -600.0,800.0 
    SHAPEEND
LAYEREND



)";
        // ARC,0,54.1600, -400.0,-400.0 ,-400.0,-600.0,CW

        hwpcb::Board board2d;
        std::istringstream iss(src);
        auto parser = hwpcb::LayerParser::make(iss);
        int k = 0;
        while (1) {

            auto layer = parser->parse();
            if (layer) {
                board2d.addLayer(layer);
                ++k;
            }
            else {
                break;
            }
        }
        return board2d;

    }
    void PCBLoader::Create_CPUModel(std::string file, CPUModel* model)
    {
        
        hwpcb::Builder3D builder3d;
        hwpcb::Board3D board3d;

        builder3d.create_Board3D(create_Board2D(file), board3d);
    
        model->m_LayerCount = board3d.size();
        model->m_Layer.resize(model->m_LayerCount);
        for (int i = 0; i < model->m_LayerCount; i++) {
            create_Layer(board3d[i], model->m_Layer[i]); 
        }
     
        
        for(int i=0;i< model->m_Layer[0][0].m_Vertices.size();i++)
        {
            model->boundingbox.minPos = glm::min(model->boundingbox.minPos, model->m_Layer[0][0].m_Vertices[i].position);
            model->boundingbox.maxPos = glm::max(model->boundingbox.maxPos, model->m_Layer[0][0].m_Vertices[i].position);
        }
    }

    void PCBLoader::Create_CPUTestModel(CPUModel* model)
    {
        hwpcb::Builder3D builder3d;
        hwpcb::Board3D board3d;

        builder3d.create_Board3D(create_TestBoard2D(), board3d);

        model->m_LayerCount = board3d.size();
        model->m_Layer.resize(model->m_LayerCount);
        for (int i = 0; i < model->m_LayerCount; i++) {
            create_Layer(board3d[i], model->m_Layer[i]);
        }


        model->boundingbox.minPos = glm::vec3(-1047.21, -1000, -80);
        model->boundingbox.maxPos = glm::vec3(1000, 1047.21, 110);
        
    }

    void PCBLoader::Create_GPUTestModel(PCBBuilder::GPUModel* model)
    {
        GPUBuilder builder;
        builder.Create_GPUModel(create_TestBoard2D(), model);
        model->m_ID = 0;
        model->boundingbox.minPos = glm::vec3(-1047.21, -1000, -80);
        model->boundingbox.maxPos = glm::vec3(1000, 1047.21, 110);
    }

    void PCBLoader::Create_GPUModel(std::string file, GPUModel* model)
    {
        GPUBuilder builder;
        builder.Create_GPUModel(create_Board2D(file), model);
        /*model->boundingbox.minPos = glm::vec3(145, 155.056, 8);
        model->boundingbox.maxPos = glm::vec3(4335.36, 3580, 9.2);*/

        for(int k=0;k< model->m_Layer.size();k++)
        {
            auto v = model->m_Layer[k].Areamodel.m_Vertices;
            if(v.size()>0)
            {
                for (int i = 0; i < v.size(); i++)
                {
                    model->boundingbox.minPos = glm::min(model->boundingbox.minPos, v[i].position);
                    model->boundingbox.maxPos = glm::max(model->boundingbox.maxPos, v[i].position);
                }
                return;
            }

        }
     
    }

    void PCBLoader::create_Layer(hwpcb::Layer3D* layer3d, CPULayer& layer)
    {
     
        hwpcb::Triangulation t;

        CPULayerModel areashells;
        areashells.m_ShellType = ShellType::Area;
        if (layer3d->layertype() == hwpcb::Layer::Type::PLANE_LAYER) {
            areashells.m_ShellType = ShellType::Plane;
        }

        CPULayerModel lineshells;
        lineshells.m_ShellType = ShellType::Line;
    
        CPULayerModel circleshells;
        circleshells.m_ShellType = ShellType::Circle;
        if (layer3d->layertype() == hwpcb::Layer::Type::MULTI_LAYER) {
            circleshells.m_ShellType = ShellType::Mutil;
        }
        std::vector<hwpcb::PCBShell*>& ShellList = layer3d->get_ShellList();
        for (int i = 0, shellcount = ShellList.size(); i < shellcount; i++) {
            
            //先调用Shell离散化函数
            t.Tri_shell(ShellList[i]);
            std::vector< hwpcb::V2d >& temp_vertices= t.get_Vertices();
          
            
            uint32_t verticesize = temp_vertices.size();

            const double baseheight = ShellList[i]->BaseHeight();
            const double topheight = ShellList[i]->TopHeight();
            //预分配内存
            std::vector<Graffiti::Vertex> points(2 * verticesize);

            //填充顶点数组
            for (int k = 0; k < verticesize; k++) {
                points[k].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, baseheight);
                points[k + verticesize].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, topheight);
            }
            if (ShellList[i]->ShellType() == hwpcb::PCBShell::PCBShellType::CircleShell) {
                std::vector< uint32_t> temp_Index = t.get_Tri_Index(circleshells.m_Vertices.size());
                circleshells.m_Vertices.insert(circleshells.m_Vertices.end(), points.begin(), points.end());
                circleshells.m_Indices.insert(circleshells.m_Indices.end(), temp_Index.begin(), temp_Index.end());
            }
            else {
                std::vector< uint32_t> temp_Index = t.get_Tri_Index(areashells.m_Vertices.size());
                areashells.m_Vertices.insert(areashells.m_Vertices.end(), points.begin(), points.end());
                areashells.m_Indices.insert(areashells.m_Indices.end(), temp_Index.begin(), temp_Index.end());
            }

           

        }
      
        //遍历Trail3D中的Shell，获取其网格信息
        for (int i = 0, trailcount = layer3d->get_TrailList().size(); i < trailcount; i++) {
            std::vector<hwpcb::PCBShell*>& SegShellList = layer3d->get_TrailList()[i]->get_ShellList();
            for (int j = 0, shellcount = SegShellList.size(); j < shellcount; j++) {

                //先调用Shell离散化函数
                t.Tri_shell(SegShellList[j]);
                std::vector< hwpcb::V2d >& temp_vertices = t.get_Vertices();
             
                uint32_t verticesize = temp_vertices.size();

                //预分配内存
                std::vector<Graffiti::Vertex> points(2 * verticesize);
             
                //填充顶点数组
                for (int k = 0; k < verticesize; k++) {
                    points[k].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, SegShellList[j]->BaseHeight());
                    points[k + verticesize].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, SegShellList[j]->TopHeight());
                }
                
                if(SegShellList[j]->ShellType() == hwpcb::PCBShell::PCBShellType::LineShell||
                    SegShellList[j]->ShellType() == hwpcb::PCBShell::PCBShellType::ArcShell)
                {
                    std::vector< uint32_t> temp_Index = t.get_Tri_Index(lineshells.m_Vertices.size());
                    lineshells.m_Vertices.insert(lineshells.m_Vertices.end(), points.begin(), points.end());    
                    lineshells.m_Indices.insert(lineshells.m_Indices.end(), temp_Index.begin(), temp_Index.end());
                }
                else if (ShellList[i]->ShellType() == hwpcb::PCBShell::PCBShellType::CircleShell) {
                    std::vector< uint32_t> temp_Index = t.get_Tri_Index(circleshells.m_Vertices.size());
                    circleshells.m_Vertices.insert(circleshells.m_Vertices.end(), points.begin(), points.end());
                    circleshells.m_Indices.insert(circleshells.m_Indices.end(), temp_Index.begin(), temp_Index.end());
                }
                else {
                    std::vector< uint32_t> temp_Index = t.get_Tri_Index(areashells.m_Vertices.size());
                    areashells.m_Vertices.insert(areashells.m_Vertices.end(), points.begin(), points.end());
                    areashells.m_Indices.insert(areashells.m_Indices.end(), temp_Index.begin(), temp_Index.end());
                }
            }

        }  
        if(areashells.m_Indices.size()>0) layer.push_back(areashells);
        if(lineshells.m_Indices.size()>0) layer.push_back(lineshells);
        if(circleshells.m_Indices.size()>0) layer.push_back(circleshells);

     
    }

}