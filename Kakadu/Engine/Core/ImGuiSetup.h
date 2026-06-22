#pragma once

namespace Kakadu::ImGuiSetup
{
    void Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void AddFonts();
    void SetStyle( const bool enable_gamma_correction );
}
