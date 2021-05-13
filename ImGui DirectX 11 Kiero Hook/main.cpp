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
	// io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

constexpr std::array<const char*, 5> characterNameStrings{
	"Young Nier",		// 0
	"Prologue Nier",    // 1
	"Old Nier",			// 2
	"Papa Nier",		// 3
	"Kaine"				// 4
};

bool imguiInit = false;
bool imguiDraw = false;

utils::Config cfg{ "replicant_hook.cfg" };

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void OpenedHook() // called when the user opens or closes imgui
{
	// open imgui and steal cursor
	imguiDraw = !imguiDraw;
	if (imguiDraw)
	{
		ReplicantHook::stealCursor(1);
		// update values // update() does this every 500ms
		// ReplicantHook::getGold();
		// ReplicantHook::getZone();
		// ReplicantHook::getZone();
	}
	else
	{
		ReplicantHook::stealCursor(0);
		ReplicantHook::onConfigSave(cfg); // save config on hook close
	}
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

	if (GetAsyncKeyState(VK_DELETE) & 1)
	{
		ReplicantHook::cursorForceHidden_toggle =! ReplicantHook::cursorForceHidden_toggle; // toggle
		ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
	}

	if (!imguiDraw) {
		goto imgui_finish;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0)), ImGuiCond_Always;

	ImGui::Begin("REPLICANT_HOOK_SIYAN_0.1", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	// check [SECTION] MAIN USER FACING STRUCTURES (ImGuiStyle, ImGuiIO) @ imgui.cpp

	if (ImGui::BeginTabBar("Trainer", ImGuiTabBarFlags_FittingPolicyScroll))
	{
		if (ImGui::BeginTabItem("General"))
		{
			ImGui::Spacing();
			ImGui::Text("General");
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushItemWidth(217);
			if (ImGui::InputInt("Gold Amount", &ReplicantHook::gold, 1, 100))
			{
				ReplicantHook::setGold(ReplicantHook::gold);
			}
			ImGui::PopItemWidth();

			if (ImGui::Checkbox("Disable cursor", &ReplicantHook::cursorForceHidden_toggle)) // toggle
			{
				ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			}
			ImGui::SameLine(0);
			HelpMarker("Disable the cursor display while using a gamepad. This can be toggled mid play with DELETE.");

			if (ImGui::Checkbox("Force 100% Model Visibility", &ReplicantHook::forceModelsVisible_toggle)) // toggle
			{
				ReplicantHook::forceModelsVisible(ReplicantHook::forceModelsVisible_toggle);
			}
			ImGui::SameLine(0);
			HelpMarker("Stop models becoming transparent when the camera gets too close");

			if (ImGui::Checkbox("Infinite Jumps", &ReplicantHook::infiniteJumps_toggle)) // toggle
			{
				ReplicantHook::infiniteJumps(ReplicantHook::infiniteJumps_toggle);
			}

			if (ImGui::Checkbox("Infinite Air Combos", &ReplicantHook::infiniteAirCombos_toggle)) // toggle
			{
				ReplicantHook::infiniteAirCombos(ReplicantHook::infiniteAirCombos_toggle);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Spoilers"))
		{
			ImGui::Checkbox("Spoiler Warning - do not tick this unless\nyou have finished every ending.", &ReplicantHook::spoiler_toggle);
			if (ReplicantHook::spoiler_toggle)
			{
				ImGui::Checkbox("Force Character Change", &ReplicantHook::forceCharSelect_toggle);
				if (ReplicantHook::forceCharSelect_toggle)
				{
					ImGui::ListBox("##CharSelectDropdown", &ReplicantHook::forceCharSelect_num, characterNameStrings.data(), characterNameStrings.size());
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Credits"))
		{
			ImGui::Spacing();
			ImGui::Text("Thanks!");
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("This hook is based off ReplicantHook by Asiern:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "https://github.com/Asiern/ReplicantHook");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/Asiern/ReplicantHook", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Separator();
			ImGui::Text("The GUI is Dear ImGui by Ocornut:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "https://github.com/ocornut/imgui");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Text("Which was built using Kiero by rdbo:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "https://github.com/rdbo/ImGui-DirectX-11-Kiero-Hook");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/rdbo/ImGui-DirectX-11-Kiero-Hook", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("Many thanks to anyone else who helped.");
			ImGui::Text("~Siyan.");
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

imgui_finish:
	return oPresent(pSwapChain, SyncInterval, Flags);
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
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //Look for memory leaks

	ReplicantHook hook = ReplicantHook();
	std::cout << "Replicant Hook\n";
	std::cout << "Hooking..." << std::endl;
	// hook to process
	while (!hook.isHooked()) {
		hook.start();
		Sleep(500);
	}
	// now hooked
	std::cout << "Hooked" << std::endl;
	ReplicantHook::onConfigLoad(cfg);

	// enable some cheats
	// hook.InfiniteHealth(true);
	// hook.InfiniteMagic(true);

	// change actor model
	// hook.setActorModel("kaineE");

	// create a thread to exit when the 'END' button is pressed
	// std::thread exitThread(ENDPressed, &hook);

	// print some values
	while (hook.isHooked()) {
		hook.update();

		if (&ReplicantHook::forceCharSelect_toggle)
		{
			ReplicantHook::forceCharSelect(ReplicantHook::forceCharSelect_num);
		}

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

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	wchar_t buffer[MAX_PATH]{ 0 };
	if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
		// Org dinput8.dll
		if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
			failed();
		}
	}
	Sleep(10000); // hook after game is loaded
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
