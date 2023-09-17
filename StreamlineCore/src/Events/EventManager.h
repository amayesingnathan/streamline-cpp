#pragma once

#include <queue>

#include "Event.h"

namespace slc {

	class IEventListener;

	class EventManager
	{
	public:
		static void RegisterListener(IEventListener* listener) { sListeners.emplace_back(listener); }
		static void DeregisterListener(IEventListener* listener) { std::erase(sListeners, listener); }

		template<IsEvent TEvent, typename... TArgs>
		static void Post(TArgs&&... args)
		{
			Event& e = sEventQueue.emplace();
			e.init<TEvent>(std::forward<TArgs>(args)...);
		}

		static void Dispatch();

	private:
		inline static std::vector<IEventListener*> sListeners;
		inline static std::queue<Event> sEventQueue;
	};
}