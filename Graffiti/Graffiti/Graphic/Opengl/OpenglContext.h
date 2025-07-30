#pragma once

#include"Graffiti/Render/GraphicsContext.h"

//ǰ���������������ˣ�֮��������ָ��
struct GLFWwindow;
namespace Graffiti {
	

	class OpenGLContext:public GraphicsContext
	{
	public:
		OpenGLContext(std::unique_ptr<Window>  windowHandle);

		virtual void Init() override;
	
		virtual void BeginFrame() override;
		virtual void BeginSwapChainRenderPass() override;
		virtual void SwapBuffers() override;
		virtual void EndFrame() override;
	
	};

}