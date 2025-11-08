--- Print verbose message to log
--- @param message string
function LogVerbose(message) end

--- Print info message to log
--- @param message string
function LogInfo(message) end

--- Print debug message to log
--- @param message string
function LogDebug(message) end

--- Print warning message to log
--- @param message string
function LogWarning(message) end

--- Print error message to log
--- @param message string
function LogError(message) end

--- Print fatal message to log
--- @param message string
function LogFatal(message) end

--- Represents an event system that allows subscribing, unsubscribing, and invoking handlers.
--- @class Event
local Event = {}

--- Subscribes a function to the event.
--- @param handler fun(...):void The function to be called when the event is invoked.
--- @return number A unique subscription ID.
function Event:subscribe(handler) end

--- Unsubscribes a function from the event by its ID.
--- @param id number The subscription ID to remove.
function Event:unsubscribe(id) end

--- Invokes all subscribed functions with the given arguments.
--- @vararg any Arguments to pass to the subscribed functions.
function Event:invoke(...) end

--- Creates a new Event object.
--- @return Event A new event instance.
function Event.new() end

--- Represents the global event for key actions.
--- @type Event
OnKeyAction = Event;