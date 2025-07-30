#include"BuildGPU.hpp"

#include<Graffiti/Render/RenderCommand.h>
namespace PCBBuilder {
    int GPUBuilder::modelname = 0;

    void GPUBuilder::Create_GPUModel(hwpcb::Board& board2d, GPUModel* model)
    {
    


        model->m_LayerCount = board2d.layerNum();
        model->m_Layer.resize(model->m_LayerCount);  
        for (int i = 0; i < board2d.layerNum(); i++) {
            verticesMutex.push_back(std::make_unique<std::mutex>());
            vertices2d = board2d.layer(i).vertexArray();
            create_GPUModelLayer(board2d.layer(i), model->m_Layer[i],i);   
        }
        for (auto& thread : threads) {
            thread.join();
        }
        for (int i = 0; i < board2d.layerNum(); i++) {
            GPULayer& layer = model->m_Layer[i];
  
            if (layer.Arcmodel.m_Indices.size() > 0) {
                layer.Arcmodel.m_VertexArray = Graffiti::VertexArray::Create();
                layer.Arcmodel.m_VertexArray->AddVertexBuffer(Graffiti::VertexBuffer::Create(layer.Arcmodel.m_Vertices, layer.Arcmodel.m_Vertices.size()));
                layer.Arcmodel.m_VertexArray->AddIndexBuffer(Graffiti::IndexBuffer::Create(layer.Arcmodel.m_Indices, layer.Arcmodel.m_Indices.size()));

            }
            if (layer.Areamodel.m_Indices.size() > 0) {
                layer.Areamodel.m_VertexArray = Graffiti::VertexArray::Create();
                layer.Areamodel.m_VertexArray->AddVertexBuffer(Graffiti::VertexBuffer::Create(layer.Areamodel.m_Vertices, layer.Areamodel.m_Vertices.size()));
                layer.Areamodel.m_VertexArray->AddIndexBuffer(Graffiti::IndexBuffer::Create(layer.Areamodel.m_Indices, layer.Areamodel.m_Indices.size()));

            }
        }
        threads.clear();
        verticesMutex.clear();
    }

    void GPUBuilder::create_GPUModelLayer(hwpcb::Layer& layer2d, GPULayer& layer, int layerNumber)
    {
        //根据层的类型进行不同的处理
        if (layer2d.type() == hwpcb::Layer::ROUTE_LAYER) {
            layer.Arcmodel.m_ShellType = ShellType::Line;
            layer.Areamodel.m_ShellType = ShellType::Area;

            hwpcb::RouteLayer& routelayer = dynamic_cast<hwpcb::RouteLayer&>(layer2d);  
            deal_RouteLayer(routelayer,layer, layerNumber);
        }
        else if (layer2d.type() == hwpcb::Layer::PLANE_LAYER) {
            layer.Areamodel.m_ShellType = ShellType::Plane;
            layer.Arcmodel.m_ShellType = ShellType::Plane;
            hwpcb::PlaneLayer& planelayer = dynamic_cast<hwpcb::PlaneLayer&>(layer2d);
            deal_PlaneLayer(planelayer, layer, layerNumber);
        }
        else if (layer2d.type() == hwpcb::Layer::MULTI_LAYER) {
            layer.Areamodel.m_ShellType = ShellType::Mutil;
            layer.CircleLetmodels.m_ShellType = ShellType::Mutil;
            hwpcb::MultiLayer& multilayer = dynamic_cast<hwpcb::MultiLayer&>(layer2d);
            deal_MultiLayer(multilayer,layer, layerNumber);
        }

    }

    void GPUBuilder::deal_PlaneLayer(hwpcb::PlaneLayer& planelayer, GPULayer& layer, int layerNumber)
    {
        hwpcb::Builder3D build3d;
        build3d.vertices2d = vertices2d;

      
        layer.CircleLetmodels.m_ID = planelayer.name() + "Plane" + std::to_string(modelname);
        modelname++;
        std::vector<double> baseheight;
        std::vector<double> topeight;
        for (int i = 0; i < planelayer.shapeNum(); i++) {
            baseheight.push_back(planelayer.baseHeight(i));
            topeight.push_back(planelayer.baseHeight(i) + planelayer.thickness(i));
        }
        hwpcb::Shape s = planelayer.shape(0);

        hwpcb::PCBShell* shell = build3d.create_Shape_Shell(s, baseheight[0], topeight[0]);
    //    tri_Planeshell(shell,layer,baseheight,topeight,layerNumber);
        threads.emplace_back(&GPUBuilder::tri_Planeshell, this, shell, std::ref(layer),baseheight, topeight, layerNumber);
        // delete shell;
    }

    void GPUBuilder::deal_RouteLayer(hwpcb::RouteLayer& routelayer, GPULayer& layer, int layerNumber)
    {
        double baseheight = routelayer.baseHeight();
        double topheight = routelayer.baseHeight() + routelayer.thickness();

        //处理Shell需要
        hwpcb::Builder3D build3d;
        build3d.vertices2d = vertices2d;

        layer.LineLetmodels.m_ID = routelayer.name() +std::to_string(modelname %10000) + "Line" ;
        layer.CircleLetmodels.m_ID = routelayer.name() + std::to_string(modelname %10000) + "Circle" ;
        modelname++;
    
        for (int i = 0; i < routelayer.lineNum(); i++) {
            hwpcb::Line line = routelayer.line(i);
            hwpcb::Trail& trail = line.trail();
            for (int j = 0; j < trail.size(); j++) {
                if (trail.seg(j).isLine()) {
                    
                    layer.LineLetmodels.m_LineLets.push_back(create_LineLet(trail.seg(j), baseheight, topheight));  
                }
                else if (trail.seg(j).isArc()) {

                   hwpcb::PCBShell* shell = build3d.create_Arc_Shell(trail.seg(j), baseheight, topheight);

                    threads.emplace_back(&GPUBuilder::tri_Arcshell, this, shell, std::ref(layer), layerNumber);
                 //   tri_Arcshell(shell, layer);
                }
            }

        }

        for (int i = 0; i < routelayer.shapeNum(); i++) {
            hwpcb::Shape s = routelayer.shape(i);
            if (s.trailNum() == 2 && s.trail(0).isCircle() && s.trail(1).isCircle()) {
                //两条边都得是圆
                layer.CircleLetmodels.m_CircleLets.push_back(create_CircleLet(s, baseheight, topheight));
            }
            else {
                hwpcb::PCBShell* shell = build3d.create_Shape_Shell(s, baseheight, topheight);
               // tri_Areashell(shell, layer);
               threads.emplace_back(&GPUBuilder::tri_Areashell, this, shell, std::ref(layer), layerNumber);
            }
        }
        if (layer.LineLetmodels.m_LineLets.size() > 0)
        Graffiti::RenderCommand::AddLineLetset(layer.LineLetmodels.m_ID, layer.LineLetmodels.m_LineLets.data(), layer.LineLetmodels.m_LineLets.size());
        if(layer.CircleLetmodels.m_CircleLets.size()>0)
        Graffiti::RenderCommand::AddCircleLetset(layer.CircleLetmodels.m_ID, layer.CircleLetmodels.m_CircleLets.data(), layer.CircleLetmodels.m_CircleLets.size());
       /* if (layer.Arcmodel.m_Indices.size() > 0) {
            layer.Arcmodel.m_VertexArray= Graffiti::VertexArray::Create();
            layer.Arcmodel.m_VertexArray->AddVertexBuffer(Graffiti::VertexBuffer::Create(layer.Arcmodel.m_Vertices, layer.Arcmodel.m_Vertices.size()));
            layer.Arcmodel.m_VertexArray->AddIndexBuffer(Graffiti::IndexBuffer::Create(layer.Arcmodel.m_Indices, layer.Arcmodel.m_Indices.size()));
        }
        if (layer.Areamodel.m_Indices.size() > 0) {
            layer.Areamodel.m_VertexArray = Graffiti::VertexArray::Create();
            layer.Areamodel.m_VertexArray->AddVertexBuffer(Graffiti::VertexBuffer::Create(layer.Areamodel.m_Vertices, layer.Areamodel.m_Vertices.size()));
            layer.Areamodel.m_VertexArray->AddIndexBuffer(Graffiti::IndexBuffer::Create(layer.Areamodel.m_Indices, layer.Areamodel.m_Indices.size()));
        }*/
 
    }

    void GPUBuilder::deal_MultiLayer(hwpcb::MultiLayer& multilayer, GPULayer& layer,int layerNumber)
    {
        

        hwpcb::Builder3D build3d;
        build3d.vertices2d = vertices2d;

        hwpcb::Triangulation tri;
        layer.CircleLetmodels.m_ID = multilayer.name() + "Circle" + std::to_string(modelname % 10000);
        modelname++;
        for (int i = 0; i < multilayer.groupNum(); i++) {
            auto g = multilayer.group(i);
            for (int j = 0; j < g.shapeNum(); j++) {
                hwpcb::Shape s = g.shape(j);

                if (s.trailNum() == 2 && s.trail(0).isCircle() && s.trail(1).isCircle()) {
                    layer.CircleLetmodels.m_CircleLets.push_back(create_CircleLet(s, multilayer.baseHeight(), multilayer.baseHeight() + g.thickness()));
                }
                else {
                    auto baseheight = multilayer.baseHeight();
                    auto topheight = multilayer.baseHeight() + g.thickness();
                    hwpcb::PCBShell* shell = build3d.create_Shape_Shell(s, baseheight, topheight);
                 //   tri_Areashell(shell, layer);
                    threads.emplace_back(&GPUBuilder::tri_Areashell, this, shell, std::ref(layer), layerNumber);
                    
                }
            }

        }
        Graffiti::RenderCommand::AddCircleLetset(layer.CircleLetmodels.m_ID, layer.CircleLetmodels.m_CircleLets.data(), layer.CircleLetmodels.m_CircleLets.size());


    }

    void GPUBuilder::tri_Areashell(hwpcb::PCBShell* shell, GPULayer& layerGPU, int layer)
    {
        hwpcb::Triangulation tri;
        tri.Tri_shell(shell); 
        auto temp_vertices = tri.get_Vertices();
        auto verticesize = temp_vertices.size();
        //预分配内存
        std::vector<Graffiti::Vertex> points(2 * verticesize);
       
        auto baseheight = shell->BaseHeight();
        auto topheight = shell->TopHeight();
        //填充顶点数组
        for (int k = 0; k < verticesize; k++) {
            points[k].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, baseheight);
            points[k + verticesize].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, topheight);
        }

       
       
        std::lock_guard<std::mutex> lock(*verticesMutex[layer]);
        {
            uint32_t size = layerGPU.Areamodel.m_Vertices.size();

            std::vector<uint32_t> indexes = tri.get_Tri_Index(size);
            layerGPU.Areamodel.m_Vertices.insert(layerGPU.Areamodel.m_Vertices.end(), points.begin(), points.end());
            layerGPU.Areamodel.m_Indices.insert(layerGPU.Areamodel.m_Indices.end(), indexes.begin(), indexes.end());
         
        }
        
    }

    void GPUBuilder::tri_Arcshell(hwpcb::PCBShell* shell, GPULayer& layerGPU, int layer)
    {

        hwpcb::Triangulation tri;
        tri.Tri_shell(shell);
        auto temp_vertices = tri.get_Vertices();
        auto verticesize = temp_vertices.size();
        //预分配内存
        std::vector<Graffiti::Vertex> points(2 * verticesize);

        auto baseheight = shell->BaseHeight();
        auto topheight = shell->TopHeight();
        //填充顶点数组
        for (int k = 0; k < verticesize; k++) {
            points[k].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, baseheight);
            points[k + verticesize].position = glm::vec3(temp_vertices[k].x, temp_vertices[k].y, topheight);
        }

        std::lock_guard<std::mutex> lock(*verticesMutex[layer]);
        {
            uint32_t size = layerGPU.Arcmodel.m_Vertices.size();
            std::vector<uint32_t> indexes = tri.get_Tri_Index(size);
            layerGPU.Arcmodel.m_Vertices.insert(layerGPU.Arcmodel.m_Vertices.end(), points.begin(), points.end());
            layerGPU.Arcmodel.m_Indices.insert(layerGPU.Arcmodel.m_Indices.end(), indexes.begin(), indexes.end());
        }
        
    }

    void GPUBuilder::tri_Planeshell(hwpcb::PCBShell* shell, GPULayer& layerGPU, std::vector<double> BaseHeight, std::vector<double> TopHeight , int layer )
    {
        hwpcb::Triangulation tri;
        tri.Tri_shell(shell);
        auto temp_vertices = tri.get_Vertices();
        auto verticesize = temp_vertices.size();

        std::vector<uint32_t> indexes = tri.get_Tri_Index(0);

        uint32_t layersize = TopHeight.size();
        layerGPU.Areamodel.m_Vertices.resize(verticesize * 2 * layersize);
        layerGPU.Areamodel.m_Indices.resize(indexes.size() * layersize);
        for (int i = 0; i < layersize; i++) {
            int basenumber = i * verticesize * 2;
            int Indexbasenumber = i * indexes.size();
            for (int j = 0; j < verticesize; j++) {
                layerGPU.Areamodel.m_Vertices[j+ basenumber].position = glm::vec3(temp_vertices[j].x, temp_vertices[j].y, BaseHeight[i]);
                layerGPU.Areamodel.m_Vertices[j+ basenumber+ verticesize].position = glm::vec3(temp_vertices[j].x, temp_vertices[j].y, TopHeight[i]);
            }
            for (int j = 0; j < indexes.size(); j++)
                layerGPU.Areamodel.m_Indices[j+ Indexbasenumber] = indexes[j]+ basenumber;
        }
        

    }



    GPUModel::GPUModel()
    {
 
    }

}