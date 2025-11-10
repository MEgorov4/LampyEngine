-- Dev скрипт для инициализации тестовой сцены с материалами
-- Автоматически выполняется при загрузке (onLoad)

local function log_info(message)
    if LogInfo then
        LogInfo("[init_material_scene] " .. message)
    else
        print("[init_material_scene] " .. message)
    end
end

local function log_error(message)
    if LogError then
        LogError("[init_material_scene] " .. message)
    else
        print("[ERROR] [init_material_scene] " .. message)
    end
end

-- Функция для создания тестовой сцены с материалами
local function createMaterialTestScene()
    local world = GetCurrentWorld()
    if not world then
        log_error("Could not get current world")
        return false
    end

    log_info("Creating material test scene...")

    -- Создаем плоскость (ground) для тестирования материалов
    local ground = world:create("MaterialTestGround")
    if ground and ground:is_valid() then
        ground:set_position(Vec3(0, -1, 0))
        -- Создаем RotationComponent (по умолчанию все углы = 0)
        local rotation = RotationComponent()
        ground:set_rotation(rotation)
        ground:set_scale(Vec3(10, 0.1, 10)) -- Плоская поверхность
        
        -- Устанавливаем меш (куб, который будет выглядеть как плоскость)
        ground:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        -- Назначаем материал по умолчанию
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        ground:set_material(materialComponent)
        
        log_info("Created ground plane with default material")
    else
        log_error("Could not create ground entity")
        return false
    end

    -- Создаем куб с золотым материалом
    local goldCube = world:create("MaterialTestGoldCube")
    if goldCube and goldCube:is_valid() then
        goldCube:set_position(Vec3(-2, 0, 0))
        goldCube:set_rotation(RotationComponent())
        goldCube:set_scale(Vec3(1, 1, 1))
        
        goldCube:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/metal_gold.lmat")
        goldCube:set_material(materialComponent)
        
        log_info("Created gold cube")
    else
        log_error("Could not create gold cube")
    end

    -- Создаем куб с красным пластиком
    local redCube = world:create("MaterialTestRedCube")
    if redCube and redCube:is_valid() then
        redCube:set_position(Vec3(0, 0, 0))
        redCube:set_rotation(RotationComponent())
        redCube:set_scale(Vec3(1, 1, 1))
        
        redCube:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/plastic_red.lmat")
        redCube:set_material(materialComponent)
        
        log_info("Created red plastic cube")
    else
        log_error("Could not create red cube")
    end

    -- Создаем сферу с материалом по умолчанию
    local sphere = world:create("MaterialTestSphere")
    if sphere and sphere:is_valid() then
        sphere:set_position(Vec3(2, 0, 0))
        sphere:set_rotation(RotationComponent())
        sphere:set_scale(Vec3(1, 1, 1))
        
        sphere:set_mesh({
            mesh_id = "Meshes/BaseGeometry/sphere.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/default_material.lmat")
        sphere:set_material(materialComponent)
        
        log_info("Created sphere with default material")
    else
        log_error("Could not create sphere")
    end

    -- Создаем куб с материалом viking_room (если есть текстура)
    local vikingCube = world:create("MaterialTestVikingCube")
    if vikingCube and vikingCube:is_valid() then
        vikingCube:set_position(Vec3(0, 0, -2))
        vikingCube:set_rotation(RotationComponent())
        vikingCube:set_scale(Vec3(1, 1, 1))
        
        vikingCube:set_mesh({
            mesh_id = "Meshes/BaseGeometry/cube.obj",
            texture_id = "",
            vert_shader_id = "",
            frag_shader_id = ""
        })
        
        local materialComponent = MaterialComponent.new()
        materialComponent.material_id = AssetID.new("Materials/viking_room_material.lmat")
        vikingCube:set_material(materialComponent)
        
        log_info("Created cube with viking_room material")
    else
        log_error("Could not create viking cube")
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

    log_info("Material test scene created successfully!")
    log_info("  - Ground plane with default material")
    log_info("  - Gold cube (metallic material)")
    log_info("  - Red plastic cube (dielectric material)")
    log_info("  - Sphere with default material")
    log_info("  - Cube with viking_room material (if texture available)")
    log_info("  - Two point lights for illumination")
    
    return true
end

-- Автоматически вызывается при загрузке скрипта
local function onLoad()
    log_info("Script loaded - initializing material test scene...")
    
    -- Небольшая задержка, чтобы убедиться, что мир готов
    -- В реальности это может быть не нужно, но на всякий случай
    createMaterialTestScene()
end

return {
    onLoad = onLoad,
    createMaterialTestScene = createMaterialTestScene,
}

