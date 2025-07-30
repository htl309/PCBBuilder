

#include"PCBLayer.hpp"
namespace Graffiti {
    
  


    class PCBApplication :public Application {
    public:
        PCBApplication() {

            PushLayer(new PCBLayer());
        }
        ~PCBApplication() {
        }
    };

    Application* CreateApplication() {
        return new PCBApplication();
    }


}
#define GF_PLATFORM_WINDOWS 

#define GF_RENDERAPI_VULKAN
//#define GF_RENDERAPI_OPENGL

#include"Graffiti/EntryPoint.h"

