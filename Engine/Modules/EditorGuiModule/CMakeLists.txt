file(GLOB_RECURSE EDITOR_GUI_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
format_source_group(${EDITOR_GUI_MODULE_SOURCE})
add_library(EditorGuiModule STATIC ${EDITOR_GUI_MODULE_SOURCE})

target_link_libraries(
  EditorGuiModule
  PRIVATE EventModule
          ProjectModule
          ObjectCoreModule
          ImGuiModule
          RenderModule
          FilesystemModule
          LuaScriptModule
          InputModule
          EventModule
          EngineContext
          imguizmo::imguizmo)
