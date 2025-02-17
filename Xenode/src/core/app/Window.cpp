#include "pch"
#include "Window.h"
#include "DesktopApplication.h"

#include <gfxapi/window/glfw/GLFW_window.h>

namespace Xen {
	Ref<Window> Window::GetWindow(const WindowProps& props)
	{
		GraphicsAPI api = DesktopApplication::GetGraphicsAPI();

		switch (api)
		{
		case GraphicsAPI::XEN_OPENGL_API:
			return std::make_shared<GLFW_window>(props);

		case GraphicsAPI::XEN_VULKAN_API:
			return std::make_shared<GLFW_window>(props);

		case GraphicsAPI::XEN_DIRECT3D_API:
			return nullptr;

		case GraphicsAPI::XEN_OPENGLES_API:
			return nullptr;

		case GraphicsAPI::XEN_METAL_API:
			return nullptr;
		}
	}
}