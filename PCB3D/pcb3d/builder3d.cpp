#include<pcb3d/builder3d.hpp>

#include <stdexcept>
namespace hwpcb {

    //析构函数
    Builder3D::~Builder3D() { vertices2d = nullptr; }
	
    //输入一个Board，输出一个Board3D
    PCBError Builder3D::create_Board3D(Board board, Board3D& board3d) {
        
      
        try {
          // 遍历层
          for (int i = 0, NL = board.layerNum(); i < NL; i++) {
            // 逐层进行三维建模
            Layer3D* layer3d = create_Layer3D(board.layer(i));

            // 如果Layer不是空的，完成建模
            if (layer3d)
              board3d.push_back(layer3d);
            else
              throw Build3DException(i);
          }
        } catch (const PCBException& e) {
          std::cout << "Error: " << e.what() << std::endl;
          return PCBError::Build3DError; 
        }

        
       //返回三维建模完成的板子
        return PCBError::Success;
    }
   
    //对输入的层进行三维建模
    Layer3D* Builder3D::create_Layer3D(Layer& layer2d) {

        //获取层中的点数组
        vertices2d = layer2d.vertexArray();

        //根据层的类型进行不同的处理
        if (layer2d.type() == Layer::ROUTE_LAYER) {
            const RouteLayer& routelayer = dynamic_cast<RouteLayer&>(layer2d);
            return deal_RouteLayer(routelayer);
        } else if (layer2d.type() == Layer::PLANE_LAYER) {
            const PlaneLayer& planelayer = dynamic_cast<PlaneLayer&>(layer2d);
            return deal_PlaneLayer(planelayer);
        }else if (layer2d.type() == Layer::MULTI_LAYER) {
            const MultiLayer& multilayer = dynamic_cast<MultiLayer&>(layer2d);
            return deal_MultiLayer(multilayer);
        }
        
        return nullptr;
    }

    //处理布线层
      Layer3D* Builder3D::deal_RouteLayer(const RouteLayer& routelayer) {

        
        Layer3D* layer3d = new Layer3D(Layer::ROUTE_LAYER);
        vector<Trail3D*>& trail3dlist = layer3d->get_TrailList();
        vector<PCBShell*>& shelllist = layer3d->get_ShellList();

        real baseheight = routelayer.upward()?routelayer.baseHeight():routelayer.baseHeight()-routelayer.thickness();
        real topHeight=  routelayer.upward()?routelayer.baseHeight()+routelayer.thickness():routelayer.baseHeight();
        
        for (int i = 0; i < routelayer.lineNum(); i++) {
            Line line = routelayer.line(i);
            Trail3D* trail3d = new Trail3D(line.id());

            vector<PCBShell*>& shelllist_in_trail3d = trail3d->get_ShellList();
            Trail& trail = line.trail();
            for (int j = 0; j < trail.size(); j++) {
              if (trail.seg(j).isCircle()) {
                shelllist_in_trail3d.push_back(create_Circle_Shell(trail.seg(j), baseheight,topHeight));
              } else if (trail.seg(j).isArc()) {
                shelllist_in_trail3d.push_back(create_Arc_Shell(trail.seg(j), baseheight,topHeight));
              } else {
                if (vertices2d[trail.seg(j).headVid].x ==vertices2d[trail.seg(j).tailVid].x &&
                    vertices2d[trail.seg(j).headVid].y ==vertices2d[trail.seg(j).tailVid].y) {

                  break;
                }
                 shelllist_in_trail3d .push_back(create_Line_Shell(trail.seg(j), baseheight,topHeight));
              }
            }
            if (shelllist_in_trail3d.size() > 0)
              trail3dlist.push_back(trail3d);
        }
      
        for (int i = 0; i < routelayer.shapeNum(); i++) {
            shelllist.push_back(create_Shape_Shell(routelayer.shape(i), baseheight,topHeight));
        }
        return layer3d;
    }


    //处理平面层
    Layer3D* Builder3D::deal_PlaneLayer(const PlaneLayer& planelayer) {
        Layer3D* layer3d = new Layer3D(Layer::PLANE_LAYER);

        vector<PCBShell*>& shelllist = layer3d->get_ShellList();
        for (int i = 0, NS = planelayer.shapeNum(); i < NS; i++) {
            shelllist.push_back(create_Shape_Shell(
                planelayer.shape(i), planelayer.baseHeight(i),
                planelayer.baseHeight(i) + planelayer.thickness(i)));
        }
        return layer3d;
    }
    
    //处理跨层
   Layer3D* Builder3D::deal_MultiLayer(const MultiLayer& multilayer) {
        Layer3D* layer3d = new Layer3D(Layer::MULTI_LAYER);
     
        vector<PCBShell*>& shelllist = layer3d->get_ShellList();
        for (int i = 0, NG = multilayer.groupNum(); i < NG; i++) {
            auto g = multilayer.group(i);
            for (int j = 0, NS = g.shapeNum(); j < NS;j++)
                shelllist.push_back(create_Shape_Shell(g.shape(j), multilayer.baseHeight(), multilayer.baseHeight() + g.thickness()));        
        }
        return layer3d;
    }

    //输入一个shape，拉升成一个shell，当然需要提供基线高度和最终高度
    PCBShell* Builder3D::create_Shape_Shell(Shape s, real baseheight,real topheight) {
    
        
        //先创建底面
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        //创建shell，但是shell的点数组是空的需要后面去填写
        PCBShell* shell;

        //判断是否是FaceShell，并处理
        if (s.trailNum() == 2) {
            //两条边都得是圆
            if (s.trail(0).isCircle() && s.trail(0).isCircle()) {

                //洞的半径需要不小于外轮廓半径
                if (s.trail(0).seg(0).radius <= s.trail(1).seg(0).radius) {
                    return create_FaceShell_in_Shape(s.id(), s.trail(0).seg(0), baseheight, topheight);
                }
                shell = new PCBShell(PCBShell::CircleShell, s.id(), baseheight, topheight, bottomface, vertexCount(s));

            }
            else {
                shell = new PCBShell(PCBShell::ShapeShell, s.id(), baseheight, topheight, bottomface, vertexCount(s));

            }
        }else
        {
            shell = new PCBShell(PCBShell::ShapeShell, s.id(), baseheight, topheight, bottomface, vertexCount(s));
        }

         //获取顶点数组，以便后续顶点填充
        vector<V2d>& vertices3d = shell->PointList();

        //获取圈数组以便后续填充
        vector<PCBLoop>& bottom_loops = bottomface->Face();

        //顶点位置索引
        //因为不能用push_back,所以的记一下索引
        uint index = 0;
        //遍历trail
        for (int i = 0, trailnum = s.trailNum(); i < trailnum; i++) {

            //获取二维闭合形状
            auto poly = s.trail(i);

            PCBLoop bottom_loop;

            //对不同情况进行处理
            //对圆形情况进行处理
            if (poly.isCircle()) {

              //获取圆心，并计算得到圆上的一个点
              Seg seg = poly.seg(0);
              V2d c = vertices2d[seg.centerVid];
              V2d p = V2d(c.x + seg.radius, c.y);

              //填充顶点
              vertices3d[index] = c;
              vertices3d[index + 1] = p;

              //创建边并填充圈
              PCBEdge* bottom_edge = new PCBEdge(index + 1, index, seg.radius);
              bottom_loop.push_back(bottom_edge);
              
              //往里面加了两个点，所以索引要加2
              index += 2;

            } else {
              //对不是圆的情况进行处理

              //获取头顶点
              V2d h = vertices2d[poly.seg(0).headVid];

              //放入头顶点
              vertices3d[index++] = h;

              //对每一个二维形状进行处理
              for (int j = 0 ,polysize=poly.size(); j < polysize; j++) {
                Seg seg = poly.seg(j);
                V2d t = vertices2d[seg.tailVid];

                if (seg.isArc()) {

                  //获取圆心
                  V2d c = vertices2d[seg.centerVid];

                  //放入圆弧圆心和尾顶点
                  vertices3d[index] = c;
                  vertices3d[index + 1] = t;

                  //计算半径
                  double r = sqrt((c.x - t.x) * (c.x - t.x) +(c.y - t.y) * (c.y - t.y));
                  //创建圆弧Edge，注意这里的索引
                  PCBEdge* bottom_edge = new PCBEdge(seg.clockwise, index - 1,index + 1, index, r);
                  //放入圈中
                  bottom_loop.push_back(bottom_edge);

                  //圆弧线段放入了两个顶点，索引加2
                  index += 2;

                } else if (seg.isLine()) {
                  vertices3d[index] = t;

                  //计算方向
                  V3d dir = V3d(t.x - h.x, t.y - h.y, 0);
                  PCBEdge* bottom_edge = new PCBEdge(index - 1, index, dir);
                  // 添加边
                  bottom_loop.push_back(bottom_edge);

                  //直线段只放入了一个顶点，索引加1
                  index++;
                }
                //下一个线段的头顶点，就是这个线段的尾节点
                h = t;
              }
            }

            //填充圈数组
            bottom_loops.push_back(bottom_loop);
        }

        return shell;
    }

    //创建shape中的刀劈面
    PCBShell* Builder3D::create_FaceShell_in_Shape(euid id, Seg s,real baseheight,real topheight) {
        
        //获取圆心，计算圆上的一点
        V2d c = vertices2d[s.centerVid];
        V2d p = V2d(c.x + s.radius, c.y);

        //创建悬挂面
        PCBFace* bottomface = new PCBFace(0, s.radius);

        //创建上下两条圆边
        PCBEdge* edge1 = new PCBEdge(1, 0, s.radius);
        PCBEdge* edge2 = new PCBEdge(3, 2, s.radius);

        //上下两条圆边放入圈中
        PCBLoop loop1;loop1.push_back(edge1);
        PCBLoop loop2;loop2.push_back(edge2);
        
        //圈放入圈数组
        bottomface->Face().push_back(loop1);
        bottomface->Face().push_back(loop2);

        PCBShell* shell = new PCBShell(id, baseheight, topheight, bottomface);
        
        //顶点填充
        vector<V2d>& vertices3d = shell->PointList();
        vertices3d.push_back(p);
        vertices3d.push_back(c);

        //创建完成
        return shell;
    }

    //创建由Line拉伸成的shell
    PCBShell* Builder3D::create_Line_Shell(Seg s, real baseheight,real topheight) {
/*

                 p0<---------------<p2
               /   |n                  \
              /    |                    \
             |   h>------------------>t  |
              \                         /
               \                       /
                 p1>--------------->p3

*/
        //获取头尾顶点
        V2d h = vertices2d[s.headVid];
        V2d t = vertices2d[s.tailVid];

        //计算方向
        V3d dir(t.x - h.x, t.y - h.y, 0);

        //处理线框为0的情况
        //创建FaceShell
        if (s.width == 0) {

            //创建底面
            PCBFace* bottomface = new PCBFace(0, V3d(0, 0, 1));

            
            //创建两条顶边和两条侧边，依次连接
            //这一过程和Shell中的获取侧面的过程类似
            PCBEdge* edge1 = new PCBEdge(0, 1, dir);
            PCBEdge* edge2 = new PCBEdge(1, 2, V3d(0, 0, 1));
            PCBEdge* edge3 = new PCBEdge(2, 3, dir);
            PCBEdge* edge4 = new PCBEdge(3, 4, V3d(0, 0, 1));

            PCBLoop loop;
            loop.push_back(edge1);
            loop.push_back(edge2);
            loop.push_back(edge3);
            loop.push_back(edge4);
            bottomface->Face().push_back(loop);

            PCBShell* shell =new PCBShell(s.id, baseheight, topheight, bottomface);

            //填充顶点
            vector<V2d>& vertices3d = shell->PointList();
            vertices3d.push_back(h);
            vertices3d.push_back(t);

            return shell;
        }
        

        //创建底面和LineShell
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        PCBShell* shell = new PCBShell(PCBShell::LineShell, s.id, baseheight,topheight, bottomface, 6);
    
        //获取顶点数组，以便后续顶点填充
        vector<V2d>& vertices3d = shell->PointList();

        //根据线宽得到半径
        real r = s.width / 2;
        
        //法向量
        //已经带上位移的法向量
        V2d normal = (t - h).normal() * r;

        //顶点平移操作i
        //法向量在向量的左侧侧
        V2d p0 = V2d(h.x + normal.x, h.y + normal.y);
        V2d p1 = V2d(h.x - normal.x, h.y - normal.y);
        V2d p2 = V2d(t.x + normal.x, t.y + normal.y);
        V2d p3 = V2d(t.x - normal.x, t.y - normal.y);

        //填充已经平移完成的顶点
        vertices3d[0] = p0;
        vertices3d[1] = p1;
        vertices3d[2] = p2;
        vertices3d[3] = p3;
        vertices3d[4] = h;
        vertices3d[5] = t;

        //创建边
        PCBEdge* bottomedge1 = new PCBEdge(0, 0, 1, 4, r);
        PCBEdge* bottomedge2 = new PCBEdge(1, 3, dir);
        PCBEdge* bottomedge3 = new PCBEdge(0, 3, 2, 5, r);
        PCBEdge* bottomedge4 = new PCBEdge(2, 0, dir);

        //填充圈和圈数组
        PCBLoop bottomloop;
        vector<PCBLoop>& bottomloops = bottomface->Face();
        bottomloop.push_back(bottomedge1);
        bottomloop.push_back(bottomedge2);
        bottomloop.push_back(bottomedge3);
        bottomloop.push_back(bottomedge4);

        bottomloops.push_back(bottomloop);

        //创建完成
        return shell;
    }
   
    //由一个圆弧膨胀拉伸成shell
    //圆弧不多，这个函数没多大调用
    PCBShell* Builder3D::create_Arc_Shell(Seg s, real baseheight,real topheight) {
/*
             /<------\
            /        p2
           /    >\     \
          p1      \     \
             \     \     \
             |      |     |
 	          /      /     /
 	        p3      /     /
           \    </     /
            \         p4
             \------>/

*/
        //获取起始结束顶点和圆弧圆心
        V2d h = vertices2d[s.headVid];
        V2d t = vertices2d[s.tailVid];
        V2d c = vertices2d[s.centerVid];

        //处理线宽为0的情况
        if (s.width == 0) {

            //创建悬挂面
            PCBFace* bottomface = new PCBFace(2, sqrt((c.x - t.x) * (c.x - t.x) + (c.y - t.y) * (c.y - t.y)));


            //创建两条顶边和两条侧边，依次连接
            //这一过程和Shell中的获取侧面的过程类似
            PCBEdge* edge1 = new PCBEdge(s.clockwise, 0, 1, 2,sqrt((c.x - t.x) * (c.x - t.x) + (c.y - t.y) * (c.y - t.y)));
            PCBEdge* edge2 = new PCBEdge(1, 3, V3d(0, 0, 1));
            PCBEdge* edge3 = new PCBEdge(s.clockwise, 3, 4, 5,sqrt((c.x - t.x) * (c.x - t.x) + (c.y - t.y) * (c.y - t.y)));
            PCBEdge* edge4 = new PCBEdge(4, 0, V3d(0, 0, 1));

            PCBLoop loop;
            loop.push_back(edge1);
            loop.push_back(edge2);
            loop.push_back(edge3);
            loop.push_back(edge4);
            bottomface->Face().push_back(loop);

            //创建Shell
            PCBShell* shell =new PCBShell(s.id, baseheight, topheight, bottomface);
           
            //填充顶点
            vector<V2d>& vertices3d = shell->PointList();
            vertices3d.push_back(h);
            vertices3d.push_back(t);
            vertices3d.push_back(c);

            return shell;
        }

       
        
        //由线框得到半径
        //这里是因为给定的线框有可能比圆弧的半径还大
        //这是不允许的，对这种情况进行处理
        real r =s.width/2 ;

       if (r >= sqrt((c.x - t.x) * (c.x - t.x) + (c.y - t.y) * (c.y - t.y))) {

            return create_Arc_Shell2(s, baseheight, topheight);
       } else if (ARC_TYPE(s)==0&&s.width >=sqrt((h.x - t.x) * (h.x - t.x) + (h.y - t.y) * (h.y - t.y))) {
            return create_Arc_Shell3(s, baseheight, topheight);
       }

        //已经带上位移的向量
        //这个向量的方向和法向量一样，长度和上面半径一样
        V2d normal_h = (h - c).normalize() * r;
        V2d normal_t = (t - c).normalize() * r;

        //顶点平移操作
        V2d p1 = V2d(h.x - normal_h.x, h.y - normal_h.y);
        V2d p2 = V2d(h.x + normal_h.x, h.y + normal_h.y);
        V2d p3 = V2d(t.x - normal_t.x, t.y - normal_t.y);
        V2d p4 = V2d(t.x + normal_t.x, t.y + normal_t.y);

       
         //创建底面和Shell
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        PCBShell* shell = new PCBShell(PCBShell::ArcShell, s.id, baseheight,topheight, bottomface, 7);
       
        vector<V2d>& vertices3d = shell->PointList();
   
        //放入平移完成的顶点
        vertices3d[0] = c;
        vertices3d[1] = p1;
        vertices3d[2] = p2;
        vertices3d[3] = p3;
        vertices3d[4] = p4;
        vertices3d[5] = h;
        vertices3d[6] = t;

        real R1 =sqrt((p3.x - c.x) * (p3.x - c.x) + (p3.y - c.y) * (p3.y - c.y));
        real R2 =sqrt((p4.x - c.x) * (p4.x - c.x) + (p4.y - c.y) * (p4.y - c.y));

        //创建边
        //注意这里的圆弧顺逆时针连接情况
        PCBEdge* bottomedge1 = new PCBEdge(!s.clockwise, 2, 1, 5, r);
        PCBEdge* bottomedge2 = new PCBEdge(s.clockwise, 1, 3, 0, R1);
        PCBEdge* bottomedge3 = new PCBEdge(!s.clockwise, 3, 4, 6, r);
        PCBEdge* bottomedge4 = new PCBEdge(!s.clockwise, 4, 2, 0, R2);

        //填充圈和圈数组
        PCBLoop bottomloop;
        vector<PCBLoop>& bottomloops = bottomface->Face();
        bottomloop.push_back(bottomedge1);
        bottomloop.push_back(bottomedge2);
        bottomloop.push_back(bottomedge3);
        bottomloop.push_back(bottomedge4);

        bottomloops.push_back(bottomloop);

        //返回建模完成的Shell
        return shell;
    }

    //处理膨胀线宽大于圆弧半径的情况
    PCBShell* Builder3D::create_Arc_Shell2(Seg s, real baseheight, real topheight) {
        V2d h = vertices2d[s.headVid];
        V2d t = vertices2d[s.tailVid];
        V2d c = vertices2d[s.centerVid];
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        PCBShell* shell = new PCBShell(PCBShell::ArcShell, s.id, baseheight,topheight, bottomface, 6);
       
        vector<V2d>& vertices3d = shell->PointList();

        double r = s.width / 2;

        double basex = (h.x + t.x) / 2;
        double basey = (h.y + t.y) / 2;
        double head_tail_distance=sqrt((basex - t.x) * (basex - t.x) + (basey - t.y) * (basey - t.y));
        double d = sqrt(r * r - head_tail_distance * head_tail_distance);

        // 已经带上位移的向量
        // 这个向量的方向和法向量一样，长度和上面半径一样
        V2d normal_h = (h - c).normalize() * r;
        V2d normal_t = (t - c).normalize() * r;

        // 顶点平移操作
        V2d p1 = V2d(h.x + normal_h.x, h.y + normal_h.y);
        V2d p2 = V2d(t.x + normal_t.x, t.y + normal_t.y);

        //根据圆弧的优劣性来决定位移法向量
        V2d normal;

        //圆弧的优劣性
        int type = ARC_TYPE(s);
        if (type == -1) {
            //如果半圆是顺时针连接，那就往右平移
           if(s.clockwise)
              normal = normal_h.normal().normalize() * d;
           //如果是逆时针连接，那么平移方向就向左
           else
              normal = -normal_h.normal().normalize() * d;

           //根据圆弧的优劣进行不同方向的平移
        } else if (type == 1) {
            normal = -(normal_h + normal_t).normalize() * d;
        } else {
            normal = (normal_h + normal_t).normalize() * d;
        }

        
        V2d p3 = V2d(basex + normal.x, basey + normal.y);
     
        //放入平移完成的顶点
        vertices3d[0] = c;
        vertices3d[1] = p1;
        vertices3d[2] = p3;
        vertices3d[3] = p2;
        vertices3d[4] = h;
        vertices3d[5] = t;

        //创建边
        //注意只需要三条圆弧就可以
        PCBEdge* bottomedge1 = new PCBEdge(s.clockwise, 1, 3, 0, sqrt((p2.x - c.x) * (p2.x - c.x) + (p2.y - c.y) * (p2.y - c.y)));
        PCBEdge* bottomedge2 = new PCBEdge(s.clockwise, 3, 2, 5, r);
        PCBEdge* bottomedge3 = new PCBEdge(s.clockwise, 2, 1, 4, r);
          

        PCBLoop bottomloop;
        vector<PCBLoop>& bottomloops = bottomface->Face();
        bottomloop.push_back(bottomedge1);
        bottomloop.push_back(bottomedge2);
        bottomloop.push_back(bottomedge3);

        bottomloops.push_back(bottomloop);


        return shell;
    }
    
    //处理线宽大于端点距离的情况
    PCBShell* Builder3D::create_Arc_Shell3(Seg s, real baseheight, real topheight) {
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        PCBShell* shell = new PCBShell(PCBShell::ArcShell, s.id, baseheight,topheight, bottomface, 9);
        
        V2d h = vertices2d[s.headVid];
        V2d t = vertices2d[s.tailVid];
        V2d c = vertices2d[s.centerVid];
        
        real r = s.width / 2;
        // 已经带上位移的向量
        // 这个向量的方向和法向量一样，长度和上面半径一样
        V2d normal_h = (h - c).normalize() * r;
        V2d normal_t = (t - c).normalize() * r;

        
        double basex = (h.x + t.x) / 2;
        double basey = (h.y + t.y) / 2;
        double head_tail_distance=sqrt((basex - t.x) * (basex - t.x) + (basey - t.y) * (basey - t.y));
        double d = sqrt(r * r - head_tail_distance * head_tail_distance);
        V2d normal = (normal_t + normal_h).normalize() * d;
        //顶点平移操作
        //单数内圈，双数外圈
        V2d p1 = V2d(h.x - normal_h.x, h.y - normal_h.y);
        V2d p2 = V2d(h.x + normal_h.x, h.y + normal_h.y);
        V2d p3 = V2d(t.x - normal_t.x, t.y - normal_t.y);
        V2d p4 = V2d(t.x + normal_t.x, t.y + normal_t.y);
        V2d p5 = V2d(basex - normal.x, basey - normal.y);
        V2d p6 = V2d(basex + normal.x, basey + normal.y);
       
        // 填充顶点
        vector<V2d>& vertices3d = shell->PointList();
        //放入平移完成的顶点
        vertices3d[0] = c;
        vertices3d[1] = p1;
        vertices3d[2] = p2;
        vertices3d[3] = p3;
        vertices3d[4] = p4;
        vertices3d[5] = p5;
        vertices3d[6] = p6;
        vertices3d[7] = h;
        vertices3d[8] = t;

        real R1 =sqrt((p3.x - c.x) * (p3.x - c.x) + (p3.y - c.y) * (p3.y - c.y));
        real R2 =sqrt((p4.x - c.x) * (p4.x - c.x) + (p4.y - c.y) * (p4.y - c.y));
        //创建边
        //注意这里的圆弧顺逆时针连接情况
        //内圈
        PCBEdge* bottomedge1 = new PCBEdge(s.clockwise, 1, 3, 0, R1);
        PCBEdge* bottomedge2 = new PCBEdge(!s.clockwise, 3, 5, 8, r);
        PCBEdge* bottomedge3 = new PCBEdge(!s.clockwise, 5, 1, 7, r);
        
        //外圈
        PCBEdge* bottomedge4 = new PCBEdge(s.clockwise, 2, 4, 0, R2);
        PCBEdge* bottomedge5 = new PCBEdge(s.clockwise, 4, 6, 8, r);
        PCBEdge* bottomedge6 = new PCBEdge(s.clockwise, 6, 2, 7, r);

        PCBLoop bottomloop1;
        PCBLoop bottomloop2;
        vector<PCBLoop>& bottomloops = bottomface->Face();
        
        bottomloop1.push_back(bottomedge1);
        bottomloop1.push_back(bottomedge2);
        bottomloop1.push_back(bottomedge3);

        bottomloop2.push_back(bottomedge4);
        bottomloop2.push_back(bottomedge5);
        bottomloop2.push_back(bottomedge6);

        bottomloops.push_back(bottomloop2);
        bottomloops.push_back(bottomloop1);
        
        return shell;
    }

    //由一个圆膨胀拉伸成shell
    PCBShell* Builder3D::create_Circle_Shell(Seg s, real baseheight,real topheight) {

        //获取圆心
        V2d c = vertices2d[s.centerVid];

        //对线宽为0的情况进行处理
        //生成FaceShell
        if (s.width == 0) {

            //获得在圆上的点
            V2d p = V2d(c.x + s.radius, c.y);

            //创建圆柱面
            PCBFace* bottomface = new PCBFace(0, s.radius);
           
            //创建上下两条圆边
            PCBEdge* edge1 = new PCBEdge(0, 1, s.radius);
            PCBEdge* edge2 = new PCBEdge(2, 3, s.radius);

            
            PCBLoop loop;
            loop.push_back(edge1);
            loop.push_back(edge2);
            bottomface->Face().push_back(loop);


            //新建Shell
            PCBShell* shell =new PCBShell(s.id, baseheight, topheight, bottomface);
            
            //放入顶点
            vector<V2d>& vertices3d = shell->PointList();
            vertices3d.push_back(p);
            vertices3d.push_back(c);

            //返回Shell
            return shell;
        }

        //创建平面
        PCBFace* bottomface = new PCBFace(0, V3d(0, 0, -1));
        
        PCBShell* shell = new PCBShell(PCBShell::CircleShell, s.id, baseheight,topheight, bottomface, 3);
        
        vector<V2d>& vertices3d = shell->PointList();
        
        //由线宽计算得到半径
        real r = s.width / 2;

        //由二维点，创建底面三维点
        V2d p1 = V2d(c.x - r + s.radius, c.y);
        V2d p2 = V2d(c.x + r + s.radius, c.y);

        vertices3d[0] = c;
        vertices3d[1] = p1;
        vertices3d[2] = p2;

        //创建边
        PCBEdge* bottomedge1 = new PCBEdge(1, 0, s.radius - r);
        PCBEdge* bottomedge2 = new PCBEdge(2, 0, s.radius + r);

        PCBLoop bottomloop1;
        PCBLoop bottomloop2;
        bottomloop1.push_back(bottomedge1);
        bottomloop2.push_back(bottomedge2);

        vector<PCBLoop>& bottomloops = bottomface->Face();
        // 2是外圈，1是内圈，这里就把1看成洞轮廓了
        bottomloops.push_back(bottomloop2);
        bottomloops.push_back(bottomloop1);

        return shell;
    }

    //统计Shape中顶点的数量
    uint Builder3D::vertexCount(const Shape& sp) {
        uint nv = 0;
        
        //统计点的数量，开始遍历
        for (uint it = 0, NT = sp.trailNum(); it < NT; ++it) {
            const Trail& t = sp.trail(it);
            if (t.isCircle()) {
              //如果是圆就要加两个点
              //一个是圆心，一个是圆上的一个点
              //虽然这里看起来只加了一个点，另一个点加在return中
              nv += 1;
            } else {
              for (uint i = 0, ns = t.size(); i < ns; ++i) {
                //直线段只需要加一个点
                //圆弧就需要加两个点
                switch (t.seg(i).type) {
                case Seg::LINE_SEG:
                  nv += 1;
                  break;
                case Seg::ARC_SEG:
                  nv += 2;
                  break;
                default:
                  verify(false);
                }
              }
            }
        }
        //返回数量
        //因为头顶点和尾顶点重合，存放了两次，这需要加上一次
        //例如：一个长方形储存了五个点而不是四个
        return nv + sp.trailNum();
    }
    //这个函数是用于判断圆弧是优弧还是劣弧的
    //优弧输出1，劣弧输出0,半圆输出-1
    int Builder3D::ARC_TYPE(Seg s) {
        V2d h = vertices2d[s.headVid];
        V2d t = vertices2d[s.tailVid];
        V2d c = vertices2d[s.centerVid];

        V2d a = t - h;
        V2d b = c - h;
        bool leftorright = 0;
        const double cross = a.x * b.y - a.y * b.x;
        if (std::fabs(cross) <= 1e-3) {
            return -1;
        } else if (cross > 0) {
            leftorright = 1;
        }
        return leftorright^s.clockwise;
    }

}
