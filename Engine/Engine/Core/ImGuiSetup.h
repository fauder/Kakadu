#pragma once

// Vendor/ImGui includes.
#include <ImGui/imgui.h>
#include <ImGui/backends/imgui_impl_glfw.h>
#include <ImGui/backends/imgui_impl_opengl3.h>

namespace Engine::ImGuiSetup
{
    void Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void AddFonts();
    void SetStyle();
}
