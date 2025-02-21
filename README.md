# Kakadu

<p align="center">
  <img src="Gallery/banner.png" width="400" />
</p>

## What is it?

It is a graphics framework I'm developing, to study graphics programming topics.

![](Gallery/screenshot-21-11-24.png)

(This project supersedes the https://github.com/fauder/OpenGL-Framework project, which is no longer maintained.)

## Features/Progress

Currently Kakadu has:
- Its own math library with support for template Vectors, Matrices, Quaternions, Polar & Spherical Coordinates, Angular types and so on, along with a Math namespace with other/extra functions to operate on them.
- A Renderer class along with other classes such as Shader, Material, Buffer, Texture, Framebuffer, Model, RenderPass, RenderQueue, RenderState, UniformBufferManagement, VertexArray & VertexLayout.
- A Mesh class along with primitive meshes such as circle, cube, full-screen cube, cylinder, quad, sphere & uv-sphere.
- Scene elements such as Camera, Transform and CameraController.
- MeshUtility to interleave vertex attributes.
- Support for hot-reloading shaders (while it worked initially, it is buggy at the moment, needs rewrite).
- Blinn-Phong lighting (soon to be PBR'd)
- Shadow mapping (for the dir. light).
- Normal & Parallax mapping.
- Editor, with support for editing/inspecting
  - Materials (textures and per-material uniforms),
  - Lights (directional, point and spot types).
  - Render pass & queue toggling as well as individual objects (Renderables in Kakadu terms).
  - Textures,
  - Framebuffers,
  - Shaders,
  - Shader Intrinsics (uniforms set internally by the Renderer) & Globals (reserved for the user's needs).
  - Camera & projection.
  - Console for both GL logs and custom application logging.
  - An ImGuiDrawer module with support for most used types throughout the Engine, powering the most of the editor.
- MSAA, Gamma Correction & HDR support.
- Ability to create custom Framebuffers, Render Queues and Render Passes.
- Offscreen rendering through internal/custom Framebuffers.
- GPU Instancing.
- Shader retrospection (allows management of memory blobs for Materials & uniform/uniform buffer editing on the editor, amongst other things).
- A minimal AssetDatabase for managing resources.
- glTF model loading.
- Texture loading via stb.
- Beginnings of a post-processing pipeline.
- And 2 client applications to showcase and test functionality (with hopefully more to follow soon!).

## WIP
- [x] Getting the Renderer ready for HDR.
- [ ] Getting the Renderer ready for a post-processing pipeline.
- [ ] Bloom post-processing effect (not the default learnopengl.com one, but a physically based one such as CoD AW's & [froyok's](https://github.com/Froyok/Bloom) implementation).

## Planned Features

- [ ] Deferred Rendering (it will either be an interesting challenge to work it into Kakadu or it will be its own repo with the heavily modified skeleton of Kakadu).
- [ ] SSAO.
- [ ] PBR.
- [ ] Frustum Culling.

## Possible Future Additions
- Client App "SDF-CSG": Constructive Solid Geometry (CSG) through Signed Distance Functions (SDF).
- Compute Shaders.
- Tessellation.
- CSM.
- Area lights.
- A proper scene graph and editor support.
- Switch to DSA for OpenGL.
- Switch to multi-draw indirect.
- GPU driven rendering.

## Credits
- To get up to speed with OpenGL, I use:
  - [Joey de Vries](https://learnopengl.com/About)'s awesome [OpenGL tutorial series](https://learnopengl.com).
  - [Cherno](https://github.com/TheCherno)'s awesome [OpenGL tutorials on YouTube](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2).
  - [Cem Yuksel](https://www.youtube.com/@cem_yuksel)'s awesome ["Introduction to Computer Graphics" playlist on YouTube](https://www.youtube.com/playlist?list=PLplnkTzzqsZTfYh4UbhLGpI5kGd5oW_Hh) on YouTube.
  - [Cem Yuksel](https://www.youtube.com/@cem_yuksel)'s awesome ["Interactive Computer Graphics" playlist on YouTube](https://www.youtube.com/playlist?list=PLplnkTzzqsZS3R5DjmCQsqupu43oS9CFN) on YouTube.
- The book [3D Math Primer for Graphics and Game Development](https://gamemath.com) (along with various other sources) was used as the basis for math algorithms & classes.
- [letheoblivion](https://www.artstation.com/letheoblivion)'s awesome Kakadu icon is used as the engine's main icon.
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) from the [stb](https://github.com/nothings/stb) library was used for texture i/o.
- [stb_include](https://github.com/nothings/stb/blob/master/stb_include.h) from the [stb](https://github.com/nothings/stb) library was used to integrate #include support for shaders.
- [Dear ImGui](https://github.com/ocornut/imgui) is used for the user interface.
- [GLFW](https://www.glfw.org/) is used for windowing & other platform-specific needs.
- [GLAD](https://github.com/Dav1dde/glad) is used as the loader for OpenGL functions.
- [fastgltf](https://github.com/spnda/fastgltf) is used to load .gltf models.
- [RenderDoc](https://renderdoc.org/) is used for analyzing/debugging graphics bugs.
- [kenney](https://kenney.nl/)'s awesome assets are used (currently for prototype textures)
- [Jasper Flick](https://catlikecoding.com/jasper-flick/)'s awesome textures are used (currently for prototype textures)
- [glslang](https://github.com/KhronosGroup/glslang) is used for offline validation of GLSL shaders as a Visual Studio post-build step.
- [JetBrains Mono](https://www.jetbrains.com/lp/mono/) is used as the font for ImGui.
- [Font Awesome](https://github.com/FortAwesome/Font-Awesome) is used as the icon font for ImGui.
- [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders) is used for accessing icons in icon fonts via simple C++ headers.
- [Google Fonts' Archivo Black](https://fonts.google.com/specimen/Archivo+Black) is used for the text in the banner image.

## License

The source code in this repository is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

The icon image [app_icon.ico](Engine/Engine/Asset/Icon/app_icon.ico) is proprietary and may not be used, copied, modified, or distributed without explicit permission. See the [Engine/Engine/Asset/Icon/LICENSE](Engine/Engine/Asset/Icon/LICENSE) file for more information.
