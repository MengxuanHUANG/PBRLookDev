#include <iostream>
#include <functional>

#include "Core/window.h"

using namespace MyCore;

int main()
{
	WindowProps props({ 600, 600 }, "PBRLookDev");
	uPtr<WindowsWindow> window = mkU<WindowsWindow>(props);

	while (window->m_Data.is_alive)
	{
		window->OnUpdate();
	}

	return 0;
}