file(GLOB_RECURSE PHYSICS_MODULE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${PHYSICS_MODULE_SOURCE})
add_library(PhysicsModule STATIC ${PHYSICS_MODULE_SOURCE})

target_link_libraries(
  PhysicsModule
  PUBLIC ObjectCoreModule Bullet::Bullet
  PRIVATE RenderModule EngineContext)
