-- Dev скрипт для инициализации тестовой сцены с физикой
-- Автоматически выполняется при загрузке (onLoad)

local function log_info(message)
    if LogInfo then
        LogInfo("[init_physics_scene] " .. message)
    else
        print("[init_physics_scene] " .. message)
    end
end

local function log_error(message)
    if LogError then
        LogError("[init_physics_scene] " .. message)
    else
        print("[ERROR] [init_physics_scene] " .. message)
    end
end

-- Функция для создания тестовой сцены с физикой
local function createPhysicsTestScene()
    local world = GetCurrentWorld()
    if not world then
        log_error("Could not get current world")
        return false
    end

    log_info("Creating physics test scene...")

    -- Создаем статическую плоскость (ground) для физики
    local ground = world:create("PhysicsTestGround")
    if ground and ground:is_valid() then
        ground:set_position(Vec3(0, -1, 0))
        local rotation = RotationComponent()
        ground:set_rotation(rotation)
        ground:set_scale(Vec3(10, 0.1, 10)) -- Плоская поверхность
        
        -- Устанавливаем меш
        ground:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        -- Назначаем материал
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        ground:set_material(materialComponent)
        
        -- Добавляем физические компоненты: Static RigidBody + Box Collider
        local rigidBody = RigidBodyComponent.new()
        rigidBody.mass = 0.0
        rigidBody.isStatic = true
        rigidBody.isKinematic = false
        rigidBody.needsCreation = true
        ground:set_rigid_body(rigidBody)
        
        local collider = ColliderComponent.new()
        collider.shapeDesc = PhysicsShapeDesc.new()
        collider.shapeDesc.type = PhysicsShapeType.Box
        collider.shapeDesc.size = Vec3(10, 0.1, 10) -- Соответствует scale
        collider.isTrigger = false
        collider.needsCreation = true
        ground:set_collider(collider)
        
        log_info("Added physics components to ground (Static)")
        
        log_info("Created static ground plane for physics")
    else
        log_error("Could not create ground entity")
        return false
    end

    -- Создаем динамический куб, который будет падать
    local fallingCube1 = world:create("PhysicsTestCube1")
    if fallingCube1 and fallingCube1:is_valid() then
        fallingCube1:set_position(Vec3(-2, 5, 0)) -- Высоко над землей
        fallingCube1:set_rotation(RotationComponent())
        fallingCube1:set_scale(Vec3(1, 1, 1))
        
        fallingCube1:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        fallingCube1:set_material(materialComponent)
        
        -- Добавляем физические компоненты: Dynamic RigidBody + Box Collider
        local rigidBody1 = RigidBodyComponent.new()
        rigidBody1.mass = 1.0
        rigidBody1.isStatic = false
        rigidBody1.isKinematic = false
        rigidBody1.needsCreation = true
        fallingCube1:set_rigid_body(rigidBody1)
        
        local collider1 = ColliderComponent.new()
        collider1.shapeDesc = PhysicsShapeDesc.new()
        collider1.shapeDesc.type = PhysicsShapeType.Box
        collider1.shapeDesc.size = Vec3(1, 1, 1) -- Соответствует scale
        collider1.isTrigger = false
        collider1.needsCreation = true
        fallingCube1:set_collider(collider1)
        
        log_info("Added physics components to falling cube 1 (Dynamic)")
        
        log_info("Created falling cube 1 at (-2, 5, 0)")
    else
        log_error("Could not create falling cube 1")
    end

    -- Создаем второй динамический куб
    local fallingCube2 = world:create("PhysicsTestCube2")
    if fallingCube2 and fallingCube2:is_valid() then
        fallingCube2:set_position(Vec3(0, 8, 0)) -- Еще выше
        fallingCube2:set_rotation(RotationComponent())
        fallingCube2:set_scale(Vec3(1, 1, 1))
        
        fallingCube2:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        fallingCube2:set_material(materialComponent)
        
        local rigidBody2 = RigidBodyComponent.new()
        rigidBody2.mass = 1.0
        rigidBody2.isStatic = false
        rigidBody2.isKinematic = false
        rigidBody2.needsCreation = true
        fallingCube2:set_rigid_body(rigidBody2)
        
        local collider2 = ColliderComponent.new()
        collider2.shapeDesc = PhysicsShapeDesc.new()
        collider2.shapeDesc.type = PhysicsShapeType.Box
        collider2.shapeDesc.size = Vec3(1, 1, 1)
        collider2.isTrigger = false
        collider2.needsCreation = true
        fallingCube2:set_collider(collider2)
        
        log_info("Added physics components to falling cube 2 (Dynamic)")
        
        log_info("Created falling cube 2 at (0, 8, 0)")
    else
        log_error("Could not create falling cube 2")
    end

    -- Создаем динамическую сферу
    local fallingSphere = world:create("PhysicsTestSphere")
    if fallingSphere and fallingSphere:is_valid() then
        fallingSphere:set_position(Vec3(2, 6, 0))
        fallingSphere:set_rotation(RotationComponent())
        fallingSphere:set_scale(Vec3(1, 1, 1))
        
        fallingSphere:set_mesh({
            mesh_id = "Meshes/BaseGeometry/sphere.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        fallingSphere:set_material(materialComponent)
        
        local rigidBodySphere = RigidBodyComponent.new()
        rigidBodySphere.mass = 1.0
        rigidBodySphere.isStatic = false
        rigidBodySphere.isKinematic = false
        rigidBodySphere.needsCreation = true
        fallingSphere:set_rigid_body(rigidBodySphere)
        
        local colliderSphere = ColliderComponent.new()
        colliderSphere.shapeDesc = PhysicsShapeDesc.new()
        colliderSphere.shapeDesc.type = PhysicsShapeType.Sphere
        colliderSphere.shapeDesc.radius = 0.5 -- Радиус сферы
        colliderSphere.isTrigger = false
        colliderSphere.needsCreation = true
        fallingSphere:set_collider(colliderSphere)
        
        log_info("Added physics components to falling sphere (Dynamic, Sphere)")
        
        log_info("Created falling sphere at (2, 6, 0)")
    else
        log_error("Could not create falling sphere")
    end

    -- Создаем статический куб (платформа)
    local platform = world:create("PhysicsTestPlatform")
    if platform and platform:is_valid() then
        platform:set_position(Vec3(0, 2, -3))
        platform:set_rotation(RotationComponent())
        platform:set_scale(Vec3(2, 0.5, 2))
        
        platform:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        platform:set_material(materialComponent)
        
        local rigidBodyPlatform = RigidBodyComponent.new()
        rigidBodyPlatform.mass = 0.0
        rigidBodyPlatform.isStatic = true
        rigidBodyPlatform.isKinematic = false
        rigidBodyPlatform.needsCreation = true
        platform:set_rigid_body(rigidBodyPlatform)
        
        local colliderPlatform = ColliderComponent.new()
        colliderPlatform.shapeDesc = PhysicsShapeDesc.new()
        colliderPlatform.shapeDesc.type = PhysicsShapeType.Box
        colliderPlatform.shapeDesc.size = Vec3(2, 0.5, 2) -- Соответствует scale
        colliderPlatform.isTrigger = false
        colliderPlatform.needsCreation = true
        platform:set_collider(colliderPlatform)
        
        log_info("Added physics components to platform (Static)")
        
        log_info("Created static platform at (0, 2, -3)")
    else
        log_error("Could not create platform")
    end

    -- Создаем точечный источник света для освещения сцены
    local light = world:create("MaterialTestLight")
    if light and light:is_valid() then
        light:set_position(Vec3(2, 3, 2))
        
        local pointLight = PointLightComponent.new()
        pointLight.inner_radius = 0.0
        pointLight.outer_radius = 15.0
        pointLight.intensity = 30.0
        pointLight.color = Vec3(1.0, 1.0, 1.0) -- Белый свет
        light:set_point_light(pointLight)
        
        log_info("Created point light at (2, 3, 2)")
    else
        log_error("Could not create point light")
    end

    -- Создаем второй источник света с другой стороны
    local light2 = world:create("MaterialTestLight2")
    if light2 and light2:is_valid() then
        light2:set_position(Vec3(-2, 3, -2))
        
        local pointLight2 = PointLightComponent.new()
        pointLight2.inner_radius = 0.0
        pointLight2.outer_radius = 15.0
        pointLight2.intensity = 20.0
        pointLight2.color = Vec3(0.8, 0.9, 1.0) -- Слегка голубоватый свет
        light2:set_point_light(pointLight2)
        
        log_info("Created second point light at (-2, 3, -2)")
    else
        log_error("Could not create second point light")
    end

    log_info("Physics test scene created successfully!")
    log_info("  - Static ground plane (y = -1)")
    log_info("  - Falling cube 1 (dynamic, y = 5)")
    log_info("  - Falling cube 2 (dynamic, y = 8)")
    log_info("  - Falling sphere (dynamic, y = 6)")
    log_info("  - Static platform (y = 2)")
    log_info("")
    log_info("Note: Make sure to enable Physics Debug in the editor to see collision shapes!")
    log_info("Objects should fall due to gravity when simulation is running.")
    
    return true
end

-- Автоматически вызывается при загрузке скрипта
local function onLoad()
    log_info("Script loaded - initializing physics test scene...")
    
    -- Небольшая задержка, чтобы убедиться, что мир готов
    createPhysicsTestScene()
end

return {
    onLoad = onLoad,
    createPhysicsTestScene = createPhysicsTestScene,
}

