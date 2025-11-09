---@meta
-- LampyEngine Lua scripting stubs.
-- Generated to mirror bindings exposed through ScriptModule registers.

-- ---------------------------------------------------------------------------
-- Logging helpers
-- ---------------------------------------------------------------------------

---@param message string
function LogInfo(message) end -- Write informational message to engine log.

---@param message string
function LogDebug(message) end -- Write debug-level message to engine log.

---@param message string
function LogVerbose(message) end -- Write verbose-level message to engine log.

---@param message string
function LogWarning(message) end -- Write warning-level message to engine log.

---@param message string
function LogError(message) end -- Write error-level message to engine log.

---@param message string
function LogFatal(message) end -- Write fatal-level message to engine log.

-- ---------------------------------------------------------------------------
-- Asset identifiers
-- ---------------------------------------------------------------------------

---@class AssetID
local AssetID = {}

---@param value string
---@return AssetID
function AssetID.new(value) return AssetID end -- Construct AssetID from string.

---@return string
function AssetID:str() return "" end

---@return boolean
function AssetID:empty() return true end

---@return string
function AssetID:__tostring() return "" end

-- ---------------------------------------------------------------------------
-- Events
-- ---------------------------------------------------------------------------

---@class SDL_KeyboardEvent
---@field scancode integer
---@field key integer
---@field repeat boolean
---@field modifiers integer
local SDL_KeyboardEvent = {}

---@class EventKeyboard
local EventKeyboard = {}

---@param handler fun(event: SDL_KeyboardEvent)
---@return integer subscriptionId -- Use with :unsubscribe to remove listener.
function EventKeyboard:subscribe(handler) return 0 end

---@param subscriptionId integer
function EventKeyboard:unsubscribe(subscriptionId) end

---@param event SDL_KeyboardEvent
function EventKeyboard:invoke(event) end

---@type EventKeyboard
OnKeyboardEvent = EventKeyboard -- Fired for every SDL keyboard event.

-- ---------------------------------------------------------------------------
-- Math helpers (glm wrappers)
-- ---------------------------------------------------------------------------

---@class Vec2
---@field x number
---@field y number
local Vec2 = {}

---@return number
function Vec2:length() return 0 end

---@return Vec2
function Vec2:getNormalized() return Vec2 end

---@param scalar number
---@return Vec2
function Vec2:__mul(scalar) return Vec2 end

---@param other Vec2
---@return Vec2
function Vec2:__add(other) return Vec2 end

---@param other Vec2
---@return Vec2
function Vec2:__sub(other) return Vec2 end

---@param scalar number
---@return Vec2
function Vec2:__div(scalar) return Vec2 end

function Vec2:normalize() end -- Normalizes the vector in place.

---@return string
function Vec2:__tostring() return "" end

---@param basis Vec2
---@return Vec2
function Vec2:projectOnto(basis) return Vec2 end

---@return Vec2
function Vec2:perpendicular() return Vec2 end

---@class Vec3
---@field x number
---@field y number
---@field z number
local Vec3 = {}

---@return number
function Vec3:length() return 0 end

function Vec3:normalize() end -- Normalizes the vector in place.

---@return Vec3
function Vec3:getNormalized() return Vec3 end

---@param other Vec3
---@return Vec3
function Vec3:__add(other) return Vec3 end

---@param other Vec3
---@return Vec3
function Vec3:__sub(other) return Vec3 end

---@param scalar number
---@return Vec3
function Vec3:__mul(scalar) return Vec3 end

---@param scalar number
---@return Vec3
function Vec3:__div(scalar) return Vec3 end

---@param basis Vec3
---@return Vec3
function Vec3:projectOnto(basis) return Vec3 end

---@param normal Vec3
---@return Vec3
function Vec3:reflect(normal) return Vec3 end

---@param other Vec3
---@return number
function Vec3:dot(other) return 0 end

---@param other Vec3
---@return Vec3
function Vec3:cross(other) return Vec3 end

---@return string
function Vec3:__tostring() return "" end

---@param v1 Vec2
---@param v2 Vec2
---@return number
function DotProduct2(v1, v2) return 0 end

---@param v1 Vec2
---@param v2 Vec2
---@return number
function Distance2(v1, v2) return 0 end

---@param v1 Vec2
---@param v2 Vec2
---@param t number
---@return Vec2
function Lerp2(v1, v2, t) return Vec2 end

---@param v Vec2
---@param normal Vec2
---@return Vec2
function Reflect2(v, normal) return Vec2 end

---@param v Vec2
---@param angle number
---@return Vec2
function Rotate2(v, angle) return Vec2 end

---@param v1 Vec3
---@param v2 Vec3
---@return number
function DotProduct3(v1, v2) return 0 end

---@param v1 Vec3
---@param v2 Vec3
---@return Vec3
function CrossProduct3(v1, v2) return Vec3 end

---@param v1 Vec3
---@param v2 Vec3
---@return number
function Distance3(v1, v2) return 0 end

---@param v1 Vec3
---@param v2 Vec3
---@param t number
---@return Vec3
function Lerp3(v1, v2, t) return Vec3 end

---@param v Vec3
---@param normal Vec3
---@return Vec3
function Reflect3(v, normal) return Vec3 end

-- ---------------------------------------------------------------------------
-- ECS helpers (flecs)
-- ---------------------------------------------------------------------------

---@class Entity
local Entity = {}

---@return boolean
function Entity:is_valid() return false end

---@return integer
function Entity:id() return 0 end

---@return string
function Entity:name() return "" end

---@param value string
function Entity:set_name(value) end

function Entity:destruct() end

-- Position ---------------------------------------------------------------

---@return boolean
function Entity:has_position() return false end

---@return Vec3|nil
function Entity:get_position() return nil end

---@param value Vec3
function Entity:set_position(value) end

---@param value Vec3
function Entity:add_position(value) end

function Entity:remove_position() end

-- Rotation ---------------------------------------------------------------

---@return boolean
function Entity:has_rotation() return false end

---@return RotationComponent|nil
function Entity:get_rotation() return nil end

---@param rotation RotationComponent
function Entity:set_rotation(rotation) end

function Entity:remove_rotation() end

-- Scale ------------------------------------------------------------------

---@return boolean
function Entity:has_scale() return false end

---@return Vec3|nil
function Entity:get_scale() return nil end

---@param value Vec3
function Entity:set_scale(value) end

function Entity:remove_scale() end

-- Camera -----------------------------------------------------------------

---@return boolean
function Entity:has_camera() return false end

---@return CameraComponent|nil
function Entity:get_camera() return nil end

---@param camera CameraComponent
function Entity:set_camera(camera) end

function Entity:remove_camera() end

-- Mesh -------------------------------------------------------------------

---@return boolean
function Entity:has_mesh() return false end

---@return MeshComponent|nil
function Entity:get_mesh() return nil end

---@param mesh MeshComponent
function Entity:set_mesh(mesh) end

function Entity:remove_mesh() end

-- Material ---------------------------------------------------------------

---@return boolean
function Entity:has_material() return false end

---@return MaterialComponent|nil
function Entity:get_material() return nil end

---@param material MaterialComponent
function Entity:set_material(material) end

function Entity:remove_material() end

-- Lights -----------------------------------------------------------------

---@return boolean
function Entity:has_point_light() return false end

---@return PointLightComponent|nil
function Entity:get_point_light() return nil end

---@param light PointLightComponent
function Entity:set_point_light(light) end

function Entity:remove_point_light() end

---@return boolean
function Entity:has_directional_light() return false end

---@return DirectionalLightComponent|nil
function Entity:get_directional_light() return nil end

---@param light DirectionalLightComponent
function Entity:set_directional_light(light) end

function Entity:remove_directional_light() end

-- Tags -------------------------------------------------------------------

---@return boolean
function Entity:has_editor_only_tag() return false end

function Entity:add_editor_only_tag() end

function Entity:remove_editor_only_tag() end

---@return boolean
function Entity:has_invisible_tag() return false end

function Entity:add_invisible_tag() end

function Entity:remove_invisible_tag() end

---@return World|nil
function GetCurrentWorld() return World end -- Returns the active ECS world or nil if unavailable.

---@class World
local World = {}

---@param name string
---@return Entity
function World:entity(name) return Entity end -- Creates or fetches an entity by name.

---@param name string|nil
---@return Entity
function World:create(name) return Entity end -- Create an entity with optional name.

---@param name string
---@return Entity|nil
function World:find(name) return Entity end -- Finds an existing entity by name.

---@param entity Entity
function World:destroy(entity) end -- Destroys the given entity instance if alive.

---@class PositionComponent
---@field x number
---@field y number
---@field z number
local PositionComponent = {}

---@class RotationComponent
---@field x number
---@field y number
---@field z number
---@field qx number
---@field qy number
---@field qz number
---@field qw number
local RotationComponent = {}

---@class ScaleComponent
---@field x number
---@field y number
---@field z number
local ScaleComponent = {}

---@class CameraComponent
---@field fov number
---@field aspect number
---@field near_clip number
---@field far_clip number
---@field is_viewport_camera boolean
local CameraComponent = {}

---@class MeshComponent
---@field mesh_id AssetID
---@field texture_id AssetID
---@field vert_shader_id AssetID
---@field frag_shader_id AssetID
local MeshComponent = {}

---@class MaterialComponent
---@field material_id AssetID
local MaterialComponent = {}

---@class PointLightComponent
---@field inner_radius number
---@field outer_radius number
---@field intensity number
---@field color Vec3
local PointLightComponent = {}

---@class DirectionalLightComponent
---@field intensity number
local DirectionalLightComponent = {}

---@class EditorOnlyTag
local EditorOnlyTag = {}

---@class InvisibleTag
local InvisibleTag = {}

-- ---------------------------------------------------------------------------
-- ImGui helpers (reserved)
-- ---------------------------------------------------------------------------

-- Placeholder for future Immediate Mode GUI bindings.


