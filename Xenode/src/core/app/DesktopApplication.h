#pragma once
#include <Core.h>

#ifdef XEN_DEVICE_DESKTOP

#include "EventDispatcher.h"
#include "LayerStack.h"

#include "Window.h"
#include "core/renderer/GraphicsContext.h"
#include "imgui/ImGuiLayer.h" 

namespace Xen {

	class XEN_API DesktopApplication
	{
	private:
		EventDispatcher dispatcher;
		bool is_Running;
		Scope<LayerStack> stack;
		inline static Ref<Window> window;
		GraphicsContext* m_Context;
		inline static GraphicsAPI m_Api = GraphicsAPI::XEN_OPENGL_API;

		Ref<ImGuiLayer> m_ImGuiLayer;


	public:
		uint32_t window_width, window_height;
		std::string window_title;
		bool vsync, resizable, imgui_render;
		uint8_t fullscreen_monitor;
		bool imgui_always_render = 0;

	public:
		DesktopApplication();
		virtual ~DesktopApplication();

		void Run();

		void PushLayer(const Ref<Layer>& layer);
		void PushLayer(const Ref<Layer>& layer, uint8_t loc);

		void PopLayer();
		void PopLayer(uint8_t loc);

		virtual void OnCreate();
		virtual void OnStart();
		virtual void OnUpdate(double timestep);
		virtual void OnFixedUpdate();
		virtual void OnRender();

		void* GetNativeWindow();
		static Ref<Window> GetWindow() { return window; }

		void ImGuiRender();

		// Events:
		void OnWindowMoveEvent(Event& event);
		void OnWindowResizeEvent(Event& event);
		void OnWindowCloseEvent(Event& event);
		void OnWindowFocusEvent(Event& event);
		void OnWindowMinimizeEvent(Event& event);
		void OnWindowMaximizeEvent(Event& event);

		void OnKeyPressEvent(Event& event);
		void OnKeyReleaseEvent(Event& event);
		void OnCharEnterEvent(Event& event);

		void OnMouseEnterEvent(Event& event);
		void OnMouseMoveEvent(Event& event);
		void OnMouseButtonPressEvent(Event& event);
		void OnMouseButtonReleaseEvent(Event& event);
		void OnMouseScrollEvent(Event& event);

		static inline void SetGraphicsAPI(GraphicsAPI api) { m_Api = api; }
		static inline GraphicsAPI GetGraphicsAPI() { return m_Api; }
	};

	XEN_API DesktopApplication* CreateDesktopApplication();
}
#endif