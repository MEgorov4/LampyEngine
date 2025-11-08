#pragma once

#include <EngineMinimal.h>
#include <SDL3/SDL_events.h>


namespace Logger
{
	class Logger;
}

namespace WindowModule
{
	class WindowModule;
}

namespace InputModule
{
	/// <summary>
	/// Manages user input events such as keyboard, mouse movement, and scrolling.
	/// Uses function callbacks to handle input from a given window.
	/// </summary>
	class InputModule : public IModule
	{
	public:
		Event<SDL_Event> OnEvent;
		
		Event<SDL_KeyboardEvent> OnKeyboardEvent;
		Event<SDL_MouseButtonEvent> OnMouseButtonEvent;
		Event<SDL_MouseMotionEvent> OnMouseMotionEvent;
		Event<SDL_MouseWheelEvent> OnMouseWheelEvent;

		/// <summary>
		/// Initializes the input system and registers callbacks for input events.
		/// </summary>
		/// <param name="window">Pointer to the window where input will be captured.</param>
		void startup() override;

		/// <summary>
		/// Shuts down the input system and clears registered callbacks.
		/// </summary>
		void shutdown() override;

		void pushEvent(const SDL_Event& event);
		void pushKeyboardEvent(const SDL_KeyboardEvent& event);
		void pushMouseButtonEvent(const SDL_MouseButtonEvent& event);
		void pushMouseMotionEvent(const SDL_MouseMotionEvent& event);
		void pushMouseWheelEvent(const SDL_MouseWheelEvent& event);

	private:
		InputModule& getInstance();
	};
}
