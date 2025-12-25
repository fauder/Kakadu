# Render vs Draw: Naming Convention

This codebase makes a strict semantic distinction between **Render** and **Draw**.

## Draw*
"Draw" functions are **mechanical submission/emission**.

They:
- Issue (or record) a *single* unit of GPU work
  (e.g. glDraw*, instanced draw, fullscreen primitive).
- Do NOT decide *what* should exist in the frame.
- Do NOT orchestrate passes, framebuffers, or features.
- May contain small local branching needed to select the draw variant.
- Are safe to call many times within the same render pass.

Examples:
- DrawMesh()
- Draw_Indexed()
- DrawInstanced()

Rule of thumb:
If the function ultimately maps to **one draw call (or equivalent)**,
it must be named **Draw***.

## Render*
"Render" functions are **orchestration/intent**.

They:
- Decide *what* is rendered and *how*.
- Own feature-, pass-, or frame-level structure.
- Select framebuffers, render states, materials, and execution order.
- Drive multiple Draw* calls.
- Are NOT safe to call repeatedly without semantic duplication.

Examples:
- RenderFrame()
- RenderViewport()
- RenderFullscreenEffect()
- RenderImGui()

Rule of thumb:
If removing **Draw*** calls still leaves meaningful logic behind,
the function is a **Render***.

## ImGui / UI Specific Note
- RenderImGui*() functions own **UI feature orchestration**.
- ImGuiDrawer::Draw*() functions **emit UI elements** into an
  already-active ImGui context and are intentionally named **Draw***.

# Summary
**Render***  => decides WHAT and HOW (orchestration/intent).  
**Draw***    => executes NOW (submission/emission).

This distinction is intentional and enforced to keep the renderer
scalable and unambiguous as complexity grows.
===============================================================================
