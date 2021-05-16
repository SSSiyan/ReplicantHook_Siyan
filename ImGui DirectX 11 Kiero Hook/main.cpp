#define DIRECTINPUT_VERSION 0x0800
#include <d3d11.h>
#include <thread>
#include <iostream>
#include <array>
#include <vector>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <dinput.h>
#include <shlobj_core.h>

#include "ReplicantHook/ReplicantHook.hpp"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;
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

utils::Config cfg{ "replicant_hook.cfg" };
static bool imguiInit = false;
static bool imguiDraw = false;

constexpr std::array<const char*, 5> characterNameStrings{
	"Young Nier",		// 0
	"Prologue Nier",    // 1
	"Old Nier",			// 2
	"Papa Nier",		// 3
	"Kaine"				// 4
};

void HelpMarker(const char* desc)
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
		// update() gets values every 500ms
		ReplicantHook::stealCursor(1);
	}
	else
	{
		ReplicantHook::stealCursor(0);
	}
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
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

	// open menu
	if (GetAsyncKeyState(VK_DELETE) & 1) {
		OpenedHook();
	}

	// toggle gamepad cursor display
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		ReplicantHook::cursorForceHidden_toggle =! ReplicantHook::cursorForceHidden_toggle;
		ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
	}

	// if display is toggled off, don't display imgui menu
	if (!imguiDraw) {
		goto imgui_finish;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0)), ImGuiCond_Always;

	ImGui::Begin("REPLICANT_HOOK_SIYAN_1.0", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	// check [SECTION] MAIN USER FACING STRUCTURES (ImGuiStyle, ImGuiIO) @ imgui.cpp

	ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SameLine(300);
	if (ImGui::Button("Save config"))
	{
		ReplicantHook::onConfigSave(cfg);
	}

	if (ImGui::BeginTabBar("Trainer", ImGuiTabBarFlags_FittingPolicyScroll))
	{
		if (ImGui::BeginTabItem("General"))
		{
			ImGui::Spacing();
			ImGui::TextWrapped("WARNING: PLEASE BACK UP YOUR SAVEDATA BEFORE USING THIS HOOK.");
			ImGui::TextWrapped("I haven't had any save corruption issues, but this is a long game and "
				"I would hate for anyone to lose their saves because of me.");
			ImGui::TextWrapped("By default your save is found here:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "My Games");
			if (ImGui::IsItemClicked()) {
				TCHAR saveGameLocation[MAX_PATH];
				TCHAR myGames[MAX_PATH] = "My Games";
				HRESULT result = SHGetFolderPathAndSubDirA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myGames, saveGameLocation);
				ShellExecuteA(NULL, "open", saveGameLocation, NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing(); 
			ImGui::Text("System");
			ImGui::Spacing();

			if (ImGui::Checkbox("Disable cursor", &ReplicantHook::cursorForceHidden_toggle)) // toggle
			{
				ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			}
			ImGui::SameLine();
			HelpMarker("Disable the cursor display while using a gamepad. This can be toggled mid play with INSERT.");

			if (ImGui::Checkbox("Force 100% Model Visibility", &ReplicantHook::forceModelsVisible_toggle)) // toggle
			{
				ReplicantHook::forceModelsVisible(ReplicantHook::forceModelsVisible_toggle);
			}
			ImGui::SameLine();
			HelpMarker("Stop models becoming transparent when the camera gets too close");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("Cheats");
			ImGui::Spacing();

			ImGui::PushItemWidth(180);
			if (ImGui::InputInt("Gold Amount", &ReplicantHook::gold, 1, 100))
			{
				ReplicantHook::setGold(ReplicantHook::gold);
			}

			if (ImGui::InputInt("Character XP", &ReplicantHook::XP, 1, 100))
			{
				ReplicantHook::setXP(ReplicantHook::XP);
			}
			ImGui::PopItemWidth();

			if (ImGui::Checkbox("Take No Damage", &ReplicantHook::takeNoDamage_toggle)) // toggle
			{
				ReplicantHook::takeNoDamage(ReplicantHook::takeNoDamage_toggle);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("Gameplay");
			ImGui::Spacing();

			// using SameLine(170) for buttons and (180) for dropdowns etc
			if (ImGui::Checkbox("Deal No Damage", &ReplicantHook::dealNoDamage_toggle)) // toggle
			{
				ReplicantHook::dealNoDamage(ReplicantHook::dealNoDamage_toggle);
			}

			if (ImGui::Checkbox("Infinite Air Combos and Dashes", &ReplicantHook::infiniteAirCombos_toggle)) // toggle
			{
				ReplicantHook::infiniteAirCombos(ReplicantHook::infiniteAirCombos_toggle);
			}

			if (ImGui::Checkbox("Infinite Jumps", &ReplicantHook::infiniteJumps_toggle)) // toggle
			{
				ReplicantHook::infiniteJumps(ReplicantHook::infiniteJumps_toggle);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Text("Reference");
			ImGui::Spacing();

			ImGui::InputFloat3("Player Position", ReplicantHook::xyzpos);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Spoilers"))
		{
			ImGui::Spacing();
			ImGui::Checkbox("Spoiler Warning - do not tick this unless\nyou have finished every ending.", &ReplicantHook::spoiler_toggle);
			if (ReplicantHook::spoiler_toggle)
			{
				ImGui::Checkbox("Force Character Change", &ReplicantHook::forceCharSelect_toggle);
				if (ReplicantHook::forceCharSelect_toggle)
				{
					ImGui::PushItemWidth(180);
					ImGui::Combo("##CharSelectDropdown", &ReplicantHook::forceCharSelect_num, characterNameStrings.data(), characterNameStrings.size());
					ImGui::PopItemWidth();
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Credits"))

		{
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
			ImGui::Text("Many thanks to anyone else who helped!");
			ImGui::Text("~Siyan");
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

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	wchar_t buffer[MAX_PATH]{ 0 };
	if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
		// Org dinput8.dll
		if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
			ExitProcess(0);
		}
	}

	// wait for game to load
	Sleep(10000);

	// look for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// init dinput and hook
	bool init_hook = false;
	do
	{	// if dinput init succeeds
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			// hook
			ReplicantHook::start();
			init_hook = true;
		}
	} while (!init_hook);

	// now hooked, load config and loop
	ReplicantHook::onConfigLoad(cfg);
	while (ReplicantHook::isHooked()) {
		ReplicantHook::update();
		Sleep(500);
	}
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
