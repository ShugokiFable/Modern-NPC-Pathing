#pragma once

#include "SKSEMenuFramework.h"

namespace Menu
{
	void Register();
	void __stdcall Render();
	void __stdcall RenderNPCPerfOverlay();

	inline MENU_WINDOW npcPerfOverlayWindow{ nullptr };
}
