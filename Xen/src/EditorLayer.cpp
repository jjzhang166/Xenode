#include "EditorLayer.h"
#include "core/scene/ScriptableEntity.h"

#include <core/app/Timer.h>
#include <core/app/Utils.h>

#include <ImGuizmo.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "math/Math.h"
#include <glm/gtx/quaternion.hpp>

Xen::Ref<Xen::Input> input;

EditorLayer::EditorLayer()
{
	m_Timestep = 0.0f;
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::OnAttach()
{
	Xen::FrameBufferSpec specs;
	specs.width = Xen::DesktopApplication::GetWindow()->GetWidth();
	specs.height = Xen::DesktopApplication::GetWindow()->GetHeight();

	input = Xen::Input::GetInputInterface();
	input->SetWindow(Xen::DesktopApplication::GetWindow());

	m_EditorCamera = std::make_shared<Xen::Camera>(Xen::CameraType::Perspective, viewport_framebuffer_width, viewport_framebuffer_height);
	m_ViewportFrameBuffer = Xen::FrameBuffer::CreateFrameBuffer(specs);

	Xen::Renderer2D::Init();

	m_ActiveScene = std::make_shared<Xen::Scene>();
	m_EditorCamera->Update();

	// To add native scripts to the entities
	//class CameraControlScript : public Xen::ScriptableEntity
	//{
	//private:
	//	Xen::Ref<Xen::Input> input;
	//public:
	//	void OnCreate() override{
	//		input = GetInput();
	//	}
	//
	//	void OnUpdate(double timestep) override{
	//		
	//	}
	//
	//	void OnDestroy() override{
	//
	//	}
	//};
	//quad_entity_1.AddComponent<Xen::Component::NativeScript>().Bind<CameraControlScript>(quad_entity_1);

	hier_panel = SceneHierarchyPanel(m_ActiveScene);
	prop_panel = PropertiesPanel(hier_panel.GetSelectedEntity());
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnUpdate(double timestep)
{
	hier_panel.SetActiveScene(m_ActiveScene);

	Xen::RenderCommand::Clear();
	m_Timestep = timestep;

	m_ViewportFrameBuffer->Bind();

	Xen::RenderCommand::Clear();
	Xen::RenderCommand::SetClearColor(Xen::Color(0.0f, 0.0f, 0.0f, 1.0f));

	if (zoom_iterations <= 0)
		m_IsMouseScrolled = 0;

	if (m_IsMouseHoveredOnViewport && m_IsMouseScrolled)
	{
		m_DistanceToFocalPoint -= m_ScrollDir * 0.04f * timestep;
		zoom_iterations--;
	}

	if (m_IsMouseHoveredOnViewport && m_IsOrbitKeyPressed)
	{
		Xen::Vec3 delta_angle = Xen::Vec3((m_NormalizedViewportMouseCoordinates.x - m_NormalizedViewportMouseCoordinatesWhenClicked.x) * 90.0f,
			(m_NormalizedViewportMouseCoordinates.y - m_NormalizedViewportMouseCoordinatesWhenClicked.y) * 90.0f,
			0.0f); 

		m_CameraRotationAlongFocalPoint.x = m_CameraRotationAlongFocalPointCurrent.x + delta_angle.x;
		m_CameraRotationAlongFocalPoint.y = m_CameraRotationAlongFocalPointCurrent.y + delta_angle.y;
	}

	if (m_IsMouseHoveredOnViewport && m_IsPanKeyPressed)
	{
		glm::vec3 delta_pan = glm::vec3((m_NormalizedViewportMouseCoordinates.x - m_NormalizedViewportMouseCoordinatesWhenClicked.x) * 3.0f,
			(m_NormalizedViewportMouseCoordinates.y - m_NormalizedViewportMouseCoordinatesWhenClicked.y) * 3.0f,
			1.0f);

		glm::vec3 focal_point = m_FocalPoint.GetVec();

		m_FocalPoint.x = m_FocalPointCurrent.x + (-delta_pan.x * m_CameraRightPosition.x);
		m_FocalPoint.y = m_FocalPointCurrent.y + (delta_pan.y * m_CameraUpPosition.y);
		m_FocalPoint.z = m_FocalPointCurrent.z + (-delta_pan.x * m_CameraRightPosition.z);
	}
	
	if (!m_IsOrbitKeyPressed)
		m_CameraRotationAlongFocalPointCurrent = m_CameraRotationAlongFocalPoint;
	if (!m_IsPanKeyPressed)
		m_FocalPointCurrent = m_FocalPoint;

	m_CameraPosition.x = m_FocalPoint.x + m_DistanceToFocalPoint * glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y)) * glm::cos(glm::radians(-m_CameraRotationAlongFocalPoint.x));
	m_CameraPosition.y = m_FocalPoint.y + m_DistanceToFocalPoint * glm::sin(glm::radians(m_CameraRotationAlongFocalPoint.y));
	m_CameraPosition.z = m_FocalPoint.z + m_DistanceToFocalPoint * glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y)) * glm::sin(glm::radians(-m_CameraRotationAlongFocalPoint.x));

	m_CameraRightPosition.x = -1.0f * glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y)) * glm::cos(glm::radians(-(m_CameraRotationAlongFocalPoint.x + 90.0f)));
	m_CameraRightPosition.y = glm::sin(glm::radians(m_CameraRotationAlongFocalPoint.y));
	m_CameraRightPosition.z = -1.0f * glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y)) * glm::sin(glm::radians(-(m_CameraRotationAlongFocalPoint.x + 90.0f)));

	m_CameraUpPosition.x = glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y + 90.0f)) * glm::cos(glm::radians(-m_CameraRotationAlongFocalPoint.x));
	m_CameraUpPosition.y = glm::sin(glm::radians(m_CameraRotationAlongFocalPoint.y + 90.0f));
	m_CameraUpPosition.z = glm::cos(glm::radians(m_CameraRotationAlongFocalPoint.y + 90.0f)) * glm::sin(glm::radians(-m_CameraRotationAlongFocalPoint.x));

	m_EditorCamera->SetPosition(m_CameraPosition);
	
	m_EditorCamera->LookAtPoint(m_FocalPoint);
	m_EditorCamera->Update(1);
	m_ActiveScene->OnUpdate(timestep, m_EditorCamera);


	// Line Rendering Test

	for (int i = -5; i < 6; i++)
	{
		Xen::Renderer2D::DrawLine(Xen::Vec3(-5.0f, i, 0.0f), Xen::Vec3(5.0f, i, 0.0f), Xen::Color(0.9f, 0.7f, 0.3f, 1.0f));
		Xen::Renderer2D::DrawLine(Xen::Vec3(i, -5.0f, 0.0f), Xen::Vec3(i, 5.0f, 0.0f), Xen::Color(0.9f, 0.7f, 0.3f, 1.0f));
	}

	Xen::Renderer2D::EndScene();
	Xen::Renderer2D::RenderFrame();

	m_ViewportFrameBuffer->Unbind();

	if (!input->IsKeyPressed(pan_key))
		m_IsPanKeyPressed = 0;
} 

void EditorLayer::OnImGuiUpdate()
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		static auto first_time = true;
		if (first_time)
		{
			first_time = false;

			ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			// split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
			// window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id 
			// we want (which ever one we DON'T set as NULL, will be returned by the function)
			// out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction

			auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, nullptr, &dockspace_id);
			auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);

			auto dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);

			// we now dock our windows into the docking node we made above
			ImGui::DockBuilderDockWindow(hier_panel.GetPanelTitle().c_str(), dock_id_left);
			ImGui::DockBuilderDockWindow("Window Two", dock_id_down);
			ImGui::DockBuilderDockWindow("Renderer Stats", dock_id_left_down);
			ImGui::DockBuilderDockWindow(prop_panel.GetPanelTitle().c_str(), dock_id_left_down);
			//ImGui::DockBuilderDockWindow("Window Three", dock_id_left_down);
			ImGui::DockBuilderDockWindow((std::string(ICON_FA_MOUNTAIN_SUN) + std::string("  2D Viewport")).c_str(), dockspace_id); // IMP: To Dock In Centre!! use directly 'dockspace_id'
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Xenode", NULL, false, false);
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open", "Ctrl+O")) 
			{
				const std::string& filePath = Xen::Utils::OpenFileDialogOpen("Xenode 2D Scene (*.xen)\0*.*\0");
				
				if (!filePath.empty())
				{
					m_ActiveScene = std::make_shared<Xen::Scene>();
					Xen::SceneSerializer serialiser = Xen::SceneSerializer(m_ActiveScene);
					//serialiser.Deserialize("assets/scene.xen");
					serialiser.Deserialize(filePath);
				}
			}

			if (ImGui::MenuItem("Save", "Ctrl+S")) 
			{
				const std::string& filePath = Xen::Utils::OpenFileDialogSave("Xenode 2D Scene (*.xen)\0*.*\0");

				if (!filePath.empty())
				{
					Xen::SceneSerializer serialiser = Xen::SceneSerializer(m_ActiveScene);
					serialiser.Serialize(filePath);
				}

			}
			if (ImGui::MenuItem("Save As..")) {}

			ImGui::Separator();

			if (ImGui::BeginMenu("Options"))
			{
				static bool b = true;
				ImGui::Checkbox("Auto Save", &b);
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Quit", "Alt+F4")) {}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))		{ ImGui::EndMenu(); }
		if (ImGui::BeginMenu("Project"))	{ ImGui::EndMenu(); }
		if (ImGui::BeginMenu("View"))		{ ImGui::EndMenu(); }
		if (ImGui::BeginMenu("Build"))		{ ImGui::EndMenu(); }
		if (ImGui::BeginMenu("Tools"))		{ ImGui::EndMenu(); }
		if (ImGui::BeginMenu("About"))		{ ImGui::EndMenu(); }
		ImGui::EndMenuBar();
	}

	ImGui::End();

	hier_panel.OnImGuiRender();
	
	ImGui::Begin("Window Two");
	ImGui::Text("Hello");
	ImGui::Text("Framerate: %f", m_Timestep);
	ImGui::Text("This is some content");

	//ImGui::Checkbox("Render From Second Camera", &render_from_entity_cam);

	ImGui::End();

	prop_panel.OnImGuiRender();
	prop_panel.SetActiveEntity(hier_panel.GetSelectedEntity());

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin((std::string(ICON_FA_MOUNTAIN_SUN) + std::string("  2D Viewport")).c_str());

	m_IsMouseHoveredOnViewport = ImGui::IsWindowHovered();

	float y_offset = ImGui::GetWindowHeight() - viewport_framebuffer_height;

	if (m_IsMouseHoveredOnViewport) 
	{
		m_NormalizedViewportMouseCoordinates.x = (ImGui::GetMousePos().x - ImGui::GetWindowPos().x) / viewport_framebuffer_height;
		m_NormalizedViewportMouseCoordinates.y = (ImGui::GetMousePos().y - ImGui::GetWindowPos().y) / viewport_framebuffer_height;
	}
	else {
		m_NormalizedViewportMouseCoordinates.x = -1.0f;
		m_NormalizedViewportMouseCoordinates.y = -1.0f;
	}

	if (viewport_framebuffer_width != ImGui::GetContentRegionAvail().x || viewport_framebuffer_height != ImGui::GetContentRegionAvail().y)
	{
		if (ImGui::GetContentRegionAvail().x <= 0 || ImGui::GetContentRegionAvail().y <= 0) {}
		else {
			viewport_framebuffer_width = ImGui::GetContentRegionAvail().x;
			viewport_framebuffer_height = ImGui::GetContentRegionAvail().y;

			m_ViewportFrameBuffer->Resize(viewport_framebuffer_width, viewport_framebuffer_height);
			m_EditorCamera->OnViewportResize(viewport_framebuffer_width, viewport_framebuffer_height);
		}
	}
	m_ActiveScene->OnViewportResize(viewport_framebuffer_width, viewport_framebuffer_height);

	ImGui::Image((void*)m_ViewportFrameBuffer->GetColorAttachmentRendererID(), ImVec2(viewport_framebuffer_width, viewport_framebuffer_height), ImVec2(0, 1), ImVec2(1, 0));

	// Gizmos: 
	Xen::Entity selectedEntity = hier_panel.GetSelectedEntity();

	if (!selectedEntity.IsNull() && selectedEntity.IsValid()) {
		
		Xen::Entity camera_entt = m_ActiveScene->GetPrimaryCameraEntity();
		if (!camera_entt.IsNull() && camera_entt.IsValid())
		{
			Xen::Component::CameraComp& camera_comp = camera_entt.GetComponent<Xen::Component::CameraComp>();
			if (camera_comp.camera->GetProjectionType() == Xen::CameraType::Orthographic)
				ImGuizmo::SetOrthographic(true);
			else
				ImGuizmo::SetOrthographic(false);
		}
		else {
			if (m_EditorCamera->GetProjectionType() == Xen::CameraType::Orthographic)
				ImGuizmo::SetOrthographic(true);
			else
				ImGuizmo::SetOrthographic(false);
		}

		ImGuizmo::SetDrawlist();

		float y_offset = ImGui::GetWindowHeight() - viewport_framebuffer_height; // To eliminate the offset caused by the window title

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + y_offset, viewport_framebuffer_width, viewport_framebuffer_height);

		glm::mat4 camera_view;
		glm::mat4 camera_projection;

		if (!camera_entt.IsNull() && camera_entt.IsValid())
		{
			camera_view = camera_entt.GetComponent<Xen::Component::CameraComp>().camera->GetViewMatrix();
			camera_projection = camera_entt.GetComponent<Xen::Component::CameraComp>().camera->GetProjectionMatrix();
		}
		else
		{
			camera_view = m_EditorCamera->GetViewMatrix();
			camera_projection = m_EditorCamera->GetProjectionMatrix();
		}

		Xen::Component::Transform& entity_transform_comp = selectedEntity.GetComponent<Xen::Component::Transform>();

		glm::vec3 degrees_vec = glm::vec3(glm::radians(entity_transform_comp.rotation.x),
			glm::radians(entity_transform_comp.rotation.y),
			glm::radians(entity_transform_comp.rotation.z));

		glm::mat4 entity_transform = glm::translate(glm::mat4(1.0f), entity_transform_comp.position.GetVec())
						* glm::toMat4(glm::quat(degrees_vec))
						* glm::scale(glm::mat4(1.0f), entity_transform_comp.scale.GetVec());

		switch (m_GizmoOperation)
		{
		case GizmoOperation::Translate:
			ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection), 
				ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(entity_transform));
			break;
		case GizmoOperation::Rotate2D:
			ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
				ImGuizmo::ROTATE_Z, ImGuizmo::LOCAL, glm::value_ptr(entity_transform));
			break;
		case GizmoOperation::Rotate3D:
			ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
				ImGuizmo::ROTATE, ImGuizmo::LOCAL, glm::value_ptr(entity_transform));
			break;
		case GizmoOperation::Scale:
			ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
				ImGuizmo::SCALE, ImGuizmo::LOCAL, glm::value_ptr(entity_transform));
			break;

		}

		//ImGuizmo::AllowAxisFlip(1);
		//TODO: Fix rotation gizmos in 3D perspective mode
		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale, skew;
			glm::quat orientation;
			glm::vec4 perspective;

			glm::decompose(entity_transform, scale, orientation, translation, skew, perspective);

			rotation = glm::eulerAngles(orientation);

			//Xen::DecomposeTransform(entity_transform, translation, rotation, scale);

			rotation.x = glm::degrees(rotation.x);
			rotation.y = glm::degrees(rotation.y);
			rotation.z = glm::degrees(rotation.z);

			glm::vec3 delta_rotation = rotation - entity_transform_comp.rotation.GetVec();

			entity_transform_comp.rotation.x += delta_rotation.x;
			entity_transform_comp.rotation.y += delta_rotation.y;
			entity_transform_comp.rotation.z += delta_rotation.z;
			

			entity_transform_comp.position = translation;
			//entity_transform_comp.rotation = Xen::Vec3(deltar_rotation.x + entity_transform_comp.rotation.x,
			//	deltar_rotation.y + entity_transform_comp.rotation.y,
			//	deltar_rotation.z + entity_transform_comp.rotation.z);
			//
			//
			entity_transform_comp.scale = scale;
			//XEN_ENGINE_LOG_INFO("Translation: {0}, {1}, {2}", translation.x, translation.y, translation.z);
			XEN_ENGINE_LOG_INFO("Rotation: {0}, {1}, {2}", entity_transform_comp.rotation.x, entity_transform_comp.rotation.y, entity_transform_comp.rotation.z);
			XEN_ENGINE_LOG_INFO("Delta Rotation: {0}, {1}, {2}", delta_rotation.x, delta_rotation.y, delta_rotation.z);
			//XEN_ENGINE_LOG_INFO("Scale: {0}, {1}, {2}", scale.delta_rotation.yx, scale.y, scale.z);

		}

		//ImGuizmo::DrawCubes(glm::value_ptr(camera_view), glm::value_ptr(camera_projection), glm::value_ptr(entity_transform), 1);
	}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Renderer Stats");
	
	Xen::Renderer2D::Renderer2DStatistics& renderer_stats = Xen::Renderer2D::GetStatistics();

	ImGui::Text("Draw Calls: %d", renderer_stats.draw_calls);
	ImGui::Text("Quads Rendered: %d", renderer_stats.quad_count);
	ImGui::Text("Circles Rendered: %d", renderer_stats.circle_count);
	ImGui::Text("Unique Texture Objects: %d", renderer_stats.texture_count);

	ImGui::Separator();

	ImGui::Text("No. of Batches: %d batches", renderer_stats.batch_count);

	ImGui::Separator();

	ImGui::Text("Quad Vertex Buffer Size: %dB", renderer_stats.quad_vertex_buffer_size);
	ImGui::Text("Circle Vertex Buffer Size: %dB", renderer_stats.circle_vertex_buffer_size);
	ImGui::Text("Quad Index Buffer Size: %dB", renderer_stats.quad_index_buffer_size);
	ImGui::Text("Circle Index Buffer Size: %dB", renderer_stats.circle_index_buffer_size);

	ImGui::Separator();

	ImGui::Text("Quad Indices Drawn: %d vertices", renderer_stats.quad_indices_drawn);
	ImGui::Text("Circle Indices Drawn: %d vertices", renderer_stats.circle_indices_drawn);

	ImGui::End();

	ImGui::Begin("Debug");

	ImGui::Text("Editor Camera Position: (%f, %f, %f)", m_EditorCamera->GetPosition().x, m_EditorCamera->GetRotation().y, m_EditorCamera->GetPosition().z);
	ImGui::Text("Editor Camera Rotation: (%f, %f, %f)", m_EditorCamera->GetRotation().x, m_EditorCamera->GetRotation().y, m_EditorCamera->GetRotation().z);
	ImGui::Text("Distance to Focal point: %f", m_DistanceToFocalPoint);
	ImGui::Text("Polar angles: (%f, %f)", m_CameraRotationAlongFocalPoint.x, m_CameraRotationAlongFocalPoint.y);

	ImGui::Text("UP: (%f, %f, %f)", m_CameraUpPosition.x, m_CameraUpPosition.y, m_CameraUpPosition.z);
	ImGui::Text("Right: (%f, %f, %f)", m_CameraRightPosition.x, m_CameraRightPosition.y, m_CameraRightPosition.z);

	ImGui::End();

}

void EditorLayer::OnFixedUpdate()
{

}

void EditorLayer::OnWindowResizeEvent(Xen::WindowResizeEvent& event)
{
	m_EditorCamera->OnViewportResize(viewport_framebuffer_width, viewport_framebuffer_height);
	m_ActiveScene->OnViewportResize(viewport_framebuffer_width, viewport_framebuffer_height);
}

void EditorLayer::OnMouseScrollEvent(Xen::MouseScrollEvent& event)
{
	m_IsMouseScrolled = 1;
	m_ScrollDir = (int8_t)event.GetYOffset();

	zoom_iterations = 20;
}

void EditorLayer::OnMouseMoveEvent(Xen::MouseMoveEvent& event)
{
	//XEN_ENGINE_LOG_WARN("{0}, {1}", m_NormalizedViewportMouseCoordinates.x, m_NormalizedViewportMouseCoordinates.y);
}

void EditorLayer::OnMouseButtonPressEvent(Xen::MouseButtonPressEvent& event)
{
	if (event.GetMouseKeyCode() == orbit_key)
	{
		if (input->IsKeyPressed(pan_key)) {
			m_IsPanKeyPressed = 1;
			m_IsPanOver = 0;
		}
		else {
			m_IsOrbitKeyPressed = 1;
			m_IsRotateOver = 0;
		}
	}

	m_NormalizedViewportMouseCoordinatesWhenClicked = m_NormalizedViewportMouseCoordinates;

	m_CameraRotationAlongFocalPointWhenClicked.x = m_NormalizedViewportMouseCoordinatesWhenClicked.x * 90.0f;
	m_CameraRotationAlongFocalPointWhenClicked.y = m_NormalizedViewportMouseCoordinatesWhenClicked.y * 90.0f;

	m_CameraPositionWhenClicked = m_EditorCamera->GetPosition();
	//XEN_ENGINE_LOG_WARN("fdsafdsa: {0}", m_CameraRotationWhenClicked.x);
}

void EditorLayer::OnMouseButtonReleaseEvent(Xen::MouseButtonReleaseEvent& event)
{
	if (event.GetMouseKeyCode() == orbit_key)
	{
		m_IsOrbitKeyPressed = 0;
		m_IsPanKeyPressed = 0;
	}
}

void EditorLayer::OnKeyPressEvent(Xen::KeyPressEvent& event)
{
	if (m_IsMouseHoveredOnViewport)
	{
		switch (event.GetKey())
		{
		case Xen::KeyCode::KEY_Q:
		{
			Xen::Entity camera_entt = m_ActiveScene->GetPrimaryCameraEntity();
			m_GizmoOperation = GizmoOperation::Translate;
			break;
		}

		case Xen::KeyCode::KEY_W:
		{
			Xen::Entity camera_entt = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera_entt.IsNull() && camera_entt.IsValid())
			{
				Xen::Component::CameraComp& camera_comp = camera_entt.GetComponent<Xen::Component::CameraComp>();
				if (camera_comp.camera->GetProjectionType() == Xen::CameraType::Orthographic)
					m_GizmoOperation = GizmoOperation::Rotate2D;
				else
					m_GizmoOperation = GizmoOperation::Rotate3D;
			}
			else {
				if (m_EditorCamera->GetProjectionType() == Xen::CameraType::Orthographic)
					m_GizmoOperation = GizmoOperation::Rotate2D;
				else
					m_GizmoOperation = GizmoOperation::Rotate3D;
			}
			break;
		}
		case Xen::KeyCode::KEY_E:
		{
			Xen::Entity camera_entt = m_ActiveScene->GetPrimaryCameraEntity();
			m_GizmoOperation = GizmoOperation::Scale;
			break;
		}

		}
	}	
}