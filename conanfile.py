from conan import ConanFile

class EngineDeps(ConanFile):
    name = "engine_deps"      
    version = "0.0"           
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("glew/2.2.0")
        self.requires("sdl/3.2.20")
        self.requires("openal-soft/1.23.1")
        self.requires("glm/cci.20230113")
        self.requires("nlohmann_json/3.11.3")
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("portable-file-dialogs/0.1.0")
        self.requires("flecs/4.0.4")
        self.requires("boost/1.86.0")
        self.requires("sol2/3.3.1")
        self.requires("fltk/1.3.9")
        self.requires("stb/cci.20240531")
        self.requires("dacap-clip/1.9")
        self.requires("bullet3/3.25")
        self.requires("gtest/1.15.0")
        self.requires("efsw/1.4.1")
        self.requires("imgui/1.92.0-docking")
        self.requires("tracy/0.12.2")
        # UI: Nuklear (rendered via IUIRenderBackend, separate from ImGui)
        self.requires("nuklear/4.06.1")