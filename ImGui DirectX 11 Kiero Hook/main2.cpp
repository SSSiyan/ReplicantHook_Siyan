#include <d3d11.h>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <dinput.h>
#include <shlobj_core.h>

#include <thread>
#include <iostream>
#include <array>
#include <vector>
#include "ReplicantHook/ReplicantHook.hpp"
#include "main2.hpp"

static utils::Config cfg{ "replicant_hook.cfg" };

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //Look for memory leaks

	ReplicantHook hook = ReplicantHook();
	// std::cout << "Replicant Hook\n";
	// std::cout << "Hooking..." << std::endl;

	// hook to process
	while (!hook.isHooked()) {
		hook.start();
		Sleep(500);
	}

	// now hooked
	// std::cout << "Hooked" << std::endl;
	ReplicantHook::onConfigLoad(cfg);

	// enable some cheats
	// hook.InfiniteHealth(true);
	// hook.InfiniteMagic(true);

	// change actor model
	// hook.setActorModel("kaineE");

	// create a thread to exit when the 'END' button is pressed
	// std::thread exitThread(ENDPressed, &hook);

	while (hook.isHooked()) {
		hook.update();
		// print some values
		// std::cout << "Magic " << hook.getMagic() << std::endl;
		// std::cout << "Health " << hook.getHealth() << std::endl;
		// std::cout << "Gold " << hook.getGold() << std::endl;
		// std::cout << "Zone " << hook.getZone() << std::endl;
		Sleep(500);
		// system("cls");
	}

	// join thread and exit
	// exitThread.join();

	return 0;
}

/*
// exit the program
void ENDPressed(ReplicantHook* hook) {
	while (true) {
		if (GetKeyState(VK_END) & 0x8000)
		{
			// disable cheats
			hook->InfiniteHealth(false);
			hook->InfiniteMagic(false);
			// stop hook
			hook->stop();
			return;
		}
	}
}
*/