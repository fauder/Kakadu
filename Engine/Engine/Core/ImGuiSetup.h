#pragma once

namespace Engine::ImGuiSetup
{
    void Initialize( const bool enable_gamma_correction );
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void AddFonts();
    void SetStyle( const bool enable_gamma_correction );
}
