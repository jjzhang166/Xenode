#include "pch"
#include "RenderCommand.h"

#include "core/app/DesktopApplication.h"

#include "gfxapi/OpenGL/OpenGLRendererAPI.h"

namespace Xen {

	Scope<RendererAPI> RenderCommand::s_Api = GetRendererAPI();
	Scope<RendererAPI> RenderCommand::GetRendererAPI()
	{
		switch (DesktopApplication::GetGraphicsAPI())
		{
		case GraphicsAPI::XEN_OPENGL_API:
			return std::make_unique<OpenGLRendererAPI>();

		case GraphicsAPI::XEN_VULKAN_API:
			//return new VulkanContext(window);

		case GraphicsAPI::XEN_DIRECT3D_API:
			return nullptr;

		case GraphicsAPI::XEN_OPENGLES_API:
			return nullptr;

		case GraphicsAPI::XEN_METAL_API:
			return nullptr;
		}
	}

	void RenderCommand::Clear()
	{
		s_Api->Clear();
	}

	void RenderCommand::SetClearColor(const Color& color)
	{
		s_Api->SetClearColor(color);
	}

	void RenderCommand::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_Api->OnWindowResize(width, height);
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indices)
	{
		s_Api->DrawIndexed(vertexArray, indices);
	}

	void RenderCommand::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t indices)
	{
		s_Api->DrawLines(vertexArray, indices);
	}

	void RenderCommand::SetLineWidth(float width)
	{
		s_Api->SetLineWidth(width);
	}
}