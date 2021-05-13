#define DIRECTINPUT_VERSION 0x0800
#include "includes.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

HMODULE g_dinput = 0;

extern "C" {
	__declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
#pragma comment(linker, "/EXPORT:DirectInput8Create=direct_input8_create")
		return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
	}
}

void failed() {
	ExitProcess(0);
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool imguiInit = false;
bool imguiDraw = false;

void OpenedHook()
{
	imguiDraw = !imguiDraw;
	if (imguiDraw) ReplicantHook::stealCursor(1);
	else ReplicantHook::stealCursor(0);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!imguiInit)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			imguiInit = true;
		}
		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1) {
		OpenedHook();
	}

	if (GetAsyncKeyState(VK_DELETE) & 1) //END button pressed
	{
		ReplicantHook::cursorForceHidden = !ReplicantHook::cursorForceHidden;
		ReplicantHook::hideCursor(ReplicantHook::cursorForceHidden);
	}

	if (!imguiDraw) {
		goto imgui_finish;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("ImGui Window");

	ImGui::SliderInt("Gold Amount", &ReplicantHook::gold, 0, 10000);

	if (ImGui::Button("Set Gold"))
	{
		ReplicantHook::setGold(ReplicantHook::gold);
	}

	ImGui::End();

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

imgui_finish:
	return oPresent(pSwapChain, SyncInterval, Flags);
}

int main();

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	wchar_t buffer[MAX_PATH]{ 0 };
	if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
		// Org dinput8.dll
		if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
			failed();
		}
	}
	Sleep(10000); // to hook after game is loaded
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			main();
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}


//Function used to exit the program
void ENDPressed(ReplicantHook* hook) {
	while (true) {
		if (GetKeyState(VK_END) & 0x8000) //END button pressed
		{
			//Disable cheats before exiting
			hook->InfiniteHealth(false);
			hook->InfiniteMagic(false);
			//Stop hook
			hook->stop();
			return; //exit function
		}
	}
}

/*This is a showcase program of the hook
* As NieR Replicant ver.1.22474487139 is a x64 program, you must compile this solution in x64.
*/
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //Look for memory leaks

	ReplicantHook hook = ReplicantHook();
	std::cout << "Replicant Hook\n";
	std::cout << "Hooking..." << std::endl;
	//Hook to process
	while (!hook.isHooked()) {
		hook.start();
		Sleep(500);
	}
	std::cout << "Hooked" << std::endl;

	//Enable some cheats
	//hook.InfiniteHealth(true);
	//hook.InfiniteMagic(true);

	//Change actor model
	//hook.setActorModel("kaineE");

	//Create a thread to exit when the 'END' button is pressed
	std::thread exitThread(ENDPressed, &hook);

	/*
	//Print some values
	while (hook.isHooked()) {
		hook.update();
		std::cout << "Magic " << hook.getMagic() << std::endl;
		std::cout << "Health " << hook.getHealth() << std::endl;
		std::cout << "Gold " << hook.getGold() << std::endl;
		std::cout << "Zone " << hook.getZone() << std::endl;
		Sleep(500);
		system("cls");
	}
	*/
	//Join thread and exit
	exitThread.join();

	return 0;
}