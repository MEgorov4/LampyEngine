local config = {
    moveStep = 0.5,
    sprintMultiplier = 2.0,
    rotationSensitivity = 0.15,
}

---@type Entity|nil
local camera_entity = nil
---@type World|nil
local cached_world = nil

local function fetch_world(world)
    if world ~= nil then
        cached_world = world
        return world
    end

    if cached_world ~= nil then
        return cached_world
    end

    if GetCurrentWorld then
        local current = GetCurrentWorld()
        if current ~= nil then
            cached_world = current
            return current
        end
    end

    return nil
end

local function log_debug(message)
    if LogInfo then
        LogInfo("[viewport_camera] " .. message)
    end
end

log_debug("script loaded")

local function ensure_camera(world)
    if camera_entity and camera_entity:is_valid() then
        return camera_entity
    end

    local lookup = fetch_world(world)
    if not lookup then
        log_debug("cached_world is nil; cannot acquire camera")
        return nil
    end

    local entity = lookup:find("ViewportCamera")
    if entity and entity:is_valid() then
        camera_entity = entity
        log_debug("acquired entity 'ViewportCamera'")
        return entity
    end

    log_debug("entity 'ViewportCamera' not found")
    return nil
end

local function radians(deg)
    return deg * (math.pi / 180.0)
end

local function camera_basis(entity)
    local rotation = entity:get_rotation()
    if not rotation then
        return Vec3(0.0, 0.0, -1.0), Vec3(1.0, 0.0, 0.0)
    end

    local yaw = radians(rotation.y or 0.0)
    local pitch = radians(rotation.x or 0.0)

    local cosPitch = math.cos(pitch)
    local sinPitch = math.sin(pitch)
    local cosYaw = math.cos(yaw)
    local sinYaw = math.sin(yaw)

    local forward = Vec3(
        sinYaw * cosPitch,
        -sinPitch,
        cosYaw * cosPitch
    )

    local right = Vec3(
        cosYaw,
        0.0,
        -sinYaw
    )

    return forward:getNormalized(), right:getNormalized()
end

local function move_camera(entity, direction, sprinting)
    if direction:length() <= 0.0001 then
        return
    end

    local step = config.moveStep
    if sprinting then
        step = step * config.sprintMultiplier
    end

    direction = direction:getNormalized() * step
    local position = entity:get_position() or Vec3(0.0, 0.0, 0.0)
    entity:set_position(position + direction)

    local new_pos = entity:get_position()
    if new_pos then
        log_debug(string.format("moved to (%.2f, %.2f, %.2f)", new_pos.x or 0.0, new_pos.y or 0.0, new_pos.z or 0.0))
    else
        log_debug("moved camera but get_position returned nil")
    end
end

local function map_key(event)
    if type(event.key) == "string" then
        return event.key:lower()
    end
    if type(event.key) == "number" then
        return event.key
    end
    if type(event.scancode) == "string" then
        return event.scancode:lower()
    end
    if type(event.scancode) == "number" then
        return event.scancode
    end
    return nil
end

local function is_shift(event)
    if event.modifiers then
        if event.modifiers.shift or event.modifiers.lshift or event.modifiers.rshift then
            return true
        end
    end
    local key = map_key(event)
    return key == "shift" or key == 16 or key == 304 or key == 303
end

local function setup(world)
    if ensure_camera(world) then
        log_debug("setup complete")
    else
        log_debug("setup could not find camera entity")
    end
end

-- Автоматически вызывается при загрузке скрипта
local function onLoad()
    log_debug("onLoad() called - initializing viewport camera script")
    -- Пытаемся найти камеру при загрузке
    setup(fetch_world(nil))
end

local function key_action(event)
    if not event then
        log_debug("key_action received nil event")
        return
    end

    log_debug("key_action triggered")

    if not (event.state == 1 or event.state == true or event.state == "down") then
        return
    end

    local entity = ensure_camera(fetch_world(nil))
    if not entity then
        return
    end

    local key = map_key(event)
    if not key then
        log_debug("unknown key in event")
        return
    end

    local forward, right = camera_basis(entity)
    local up = Vec3(0.0, 1.0, 0.0)
    local sprinting = is_shift(event)
    local move = Vec3(0.0, 0.0, 0.0)

    if key == "w" or key == 119 or key == 87 then
        move = move + forward
    elseif key == "s" or key == 115 or key == 83 then
        move = move - forward
    elseif key == "d" or key == 100 or key == 68 then
        move = move + right
    elseif key == "a" or key == 97 or key == 65 then
        move = move - right
    elseif key == "space" or key == 32 or key == "e" or key == 101 or key == 69 then
        move = move + up
    elseif key == "q" or key == 113 or key == 81 or key == 308 or key == 400 then
        move = move - up
    else
        log_debug("key not handled: " .. tostring(key))
        return
    end

    log_debug(string.format(
        "move vector for key %s => (%.3f, %.3f, %.3f)%s",
        tostring(key),
        move.x or 0.0,
        move.y or 0.0,
        move.z or 0.0,
        sprinting and " [sprint]" or ""
    ))

    move_camera(entity, move, sprinting)
end

local function mouse_action(event)
    if not event then
        log_debug("mouse_action received nil event")
        return
    end

    local entity = ensure_camera(fetch_world(nil))
    if not entity then
        return
    end

    local dx = event.dx or event.xrel or 0.0
    local dy = event.dy or event.yrel or 0.0

    log_debug(string.format("mouse_action dx=%.2f dy=%.2f", dx, dy))

    if dx == 0 and dy == 0 then
        return
    end

    local rot = entity:get_rotation() or RotationComponent()
    rot.y = (rot.y or 0.0) - dx * config.rotationSensitivity
    rot.x = (rot.x or 0.0) - dy * config.rotationSensitivity
    rot.x = math.max(-89.9, math.min(89.9, rot.x))

    entity:set_rotation(rot)
end

return {
    setup = setup,
    key_action = key_action,
    mouse_action = mouse_action,
    onLoad = onLoad, -- Автоматически вызывается при загрузке скрипта
}