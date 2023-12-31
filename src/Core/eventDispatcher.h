#pragma once

#include "event.h"
#include <functional>

namespace MyCore
{
	class EventDispatcher
	{
	public:
		template<typename T>
		using EventCallBackFn = std::function<bool(T&)>;

		EventDispatcher(Event& event)
			:m_Event(event)
		{}

		template<typename T>
		bool Dispatch(EventCallBackFn<T> fn)
		{
			if (T::GetStaticType() == m_Event.GetEventType())
			{
				m_Event.m_Handled = fn(*(T*)&m_Event);
				return m_Event.m_Handled;
			}
			else
			{
				return false;
			}
		}

	protected:
		Event& m_Event;
	};
}