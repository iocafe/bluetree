imgui_iface 
note 15.1.2021/pekka

- The imgui_iface.h here defines functions and types that egui library code uses to call Dear ImGui backend (for example openGL 3, GLFW). 
- The backend files which ship with are ImGui are in /coderoot/bluetree/egui/glfw_opengl3, etc directory.
- Backend specific implementation for the egui library, like eimgui_iface_glfw_opengl3.cpp is in /coderoot/bluetree/egui/glfw_opengl3/eimgui_iface, etc, directory.

