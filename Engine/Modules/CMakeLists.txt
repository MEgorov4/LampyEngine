include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(MemoryModule)
add_subdirectory(ResourceModule)
add_subdirectory(WindowModule)
add_subdirectory(AudioModule)
add_subdirectory(InputModule)
add_subdirectory(RenderModule)
add_subdirectory(PhysicsModule)

add_subdirectory(ProjectModule)

add_subdirectory(LuaScriptModule)
add_subdirectory(ObjectCoreModule)

add_subdirectory(ImGuiModule)
add_subdirectory(EditorGuiModule)

add_subdirectory(ShaderCompilerModule)
