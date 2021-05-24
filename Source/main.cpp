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
typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
HMODULE g_dinput = 0;
ResizeBuffers oResizeBuffers;

extern "C" {
	__declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
#pragma comment(linker, "/EXPORT:DirectInput8Create=direct_input8_create")
		return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
	}
}

utils::Config cfg{ "replicant_hook.cfg" };
static bool imguiInit = false;
static bool imguiDraw = false;

constexpr std::array<const char*, 5> characterNameStrings {
	"Young Nier",		// 0
	"Prologue Nier",    // 1
	"Old Nier",			// 2
	"Papa Nier",		// 3
	"Kaine"				// 4
};

void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void OpenedHook() {
	imguiDraw = !imguiDraw;
	if (imguiDraw)
		ReplicantHook::stealCursor(1);
	else
		ReplicantHook::stealCursor(0);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void InitImGui() {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// font load examples
	// io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 15.0f);
	// io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\impact.ttf", 15.0f);
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGuiStyle& style = ImGui::GetStyle();
	// 4hook styling
	style.WindowRounding = 3.0f;
	style.FrameRounding = 2.5f;
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0026f, 0.0026f, 0.0026f, 0.90f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.65f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.25f, 0.29f, 0.95f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.4588f, 0.45880f, 0.4588f, 0.35f);
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

HRESULT hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    if (mainRenderTargetView) {
        pContext->OMSetRenderTargets(0, 0, 0);
        mainRenderTargetView->Release();
    }

    HRESULT hr = oResizeBuffers(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    ID3D11Texture2D* pBuffer;
    pThis->GetBuffer(0, __uuidof(ID3D11Texture2D),
        (void**)&pBuffer);
    // Perform error handling here!

    pDevice->CreateRenderTargetView(pBuffer, NULL,
        &mainRenderTargetView);
    // Perform error handling here!
    pBuffer->Release();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

    // Set up the viewport.
    D3D11_VIEWPORT vp;
    vp.Width = Width;
    vp.Height = Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pContext->RSSetViewports(1, &vp);
    return hr;
}

void InitHook()
{
	// get process ID and module base address
	ReplicantHook::_hook();

	// load settings, must happen after hook
	ReplicantHook::onConfigLoad(cfg);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!imguiInit) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice))) {
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
			InitHook();
			imguiInit = true;
		}
		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}
	// check process ID is valid
	if (ReplicantHook::_pID != ReplicantHook::_getProcessID()) {
		goto imgui_finish;
	}
	// update imgui values
	ReplicantHook::update();
	// open menu
	if (GetAsyncKeyState(VK_DELETE) & 1) {
		OpenedHook();
	}
	// toggle gamepad cursor display
	if (GetAsyncKeyState(VK_INSERT) & 1) {
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
	ImGui::SetNextWindowSize(ImVec2(400, 500)), ImGuiCond_Always;

	ImGui::Begin("REPLICANT_HOOK_SIYAN_1.2", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SameLine(280);
	if (ImGui::Button("Save config")) {
		ReplicantHook::onConfigSave(cfg);
	}

	if (ImGui::BeginTabBar("Trainer", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
		if (ImGui::BeginTabItem("General")) {
			ImGui::Text("System");
			ImGui::Spacing();

			if (ImGui::Checkbox("Disable cursor", &ReplicantHook::cursorForceHidden_toggle)) { // toggle 
				ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			}
			ImGui::SameLine();
			HelpMarker("Disable the cursor display while using a gamepad. This can be toggled mid play with INSERT");

			if (ImGui::Checkbox("Force 100% Model Visibility", &ReplicantHook::forceModelsVisible_toggle)) { // toggle
				ReplicantHook::forceModelsVisible(ReplicantHook::forceModelsVisible_toggle);
			}
			ImGui::SameLine();
			HelpMarker("Stop models becoming transparent when the camera gets too close");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Cheats");
			ImGui::Spacing();

			ImGui::PushItemWidth(180);
			if (ImGui::InputInt("Gold Amount", &ReplicantHook::gold, 1, 100)) {
				ReplicantHook::setGold(ReplicantHook::gold);
			}

			if (ImGui::InputInt("Character XP", &ReplicantHook::XP, 1, 100)) {
				ReplicantHook::setXP(ReplicantHook::XP);
			}
			ImGui::PopItemWidth();

			if (ImGui::Checkbox("Take No Damage", &ReplicantHook::takeNoDamage_toggle)) { // toggle
				ReplicantHook::takeNoDamage(ReplicantHook::takeNoDamage_toggle);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Gameplay");
			ImGui::Spacing();

			// using SameLine(170) for buttons and (180) for dropdowns etc
			if (ImGui::Checkbox("Deal No Damage", &ReplicantHook::dealNoDamage_toggle)) { // toggle
				ReplicantHook::dealNoDamage(ReplicantHook::dealNoDamage_toggle);
			}

			if (ImGui::Checkbox("Infinite Air Combos and Dashes", &ReplicantHook::infiniteAirCombos_toggle)) { // toggle
				ReplicantHook::infiniteAirCombos(ReplicantHook::infiniteAirCombos_toggle);
			}

			if (ImGui::Checkbox("Infinite Jumps", &ReplicantHook::infiniteJumps_toggle)) { // toggle
				ReplicantHook::infiniteJumps(ReplicantHook::infiniteJumps_toggle);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Reference");
			ImGui::Spacing();

			ImGui::Text("Player Position:");
			ImGui::InputFloat3("##Player Position", ReplicantHook::xyzpos);
			ImGui::Spacing();
			ImGui::Text("Current area: %s", ReplicantHook::zone);
			ImGui::Text("Save name: %s", ReplicantHook::name);
			/*if (ImGui::InputText("##ZoneInputFieldTest", (char*)ReplicantHook::zone, IM_ARRAYSIZE(ReplicantHook::zone))) {
				ReplicantHook::setZone(ReplicantHook::zone);
			}*/
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Spoilers")) {
			ImGui::Checkbox("Spoiler Warning - do not tick this unless\nyou have finished every ending.", &ReplicantHook::spoiler_toggle);
			if (ReplicantHook::spoiler_toggle) {
				ImGui::Checkbox("Force Character Change", &ReplicantHook::forceCharSelect_toggle);
				if (ReplicantHook::forceCharSelect_toggle) {
					ImGui::PushItemWidth(180);
					ImGui::Combo("##CharSelectDropdown", &ReplicantHook::forceCharSelect_num, characterNameStrings.data(), characterNameStrings.size());
					ImGui::PopItemWidth();
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Warning")) {
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
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Credits")) {
			ImGui::Text("This hook is based off ReplicantHook by Asiern:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "github.com/Asiern/ReplicantHook");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/Asiern/ReplicantHook", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Separator();
			ImGui::Text("The GUI is Dear ImGui by Ocornut:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "github.com/ocornut/imgui");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Text("Which was built using Kiero by rdbo:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "github.com/rdbo/ImGui-DirectX-11-Kiero-Hook");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/rdbo/ImGui-DirectX-11-Kiero-Hook", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Separator();
			ImGui::Text("Find updates for ReplicantHook_Siyan:\n");
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "github.com/SSSiyan/ReplicantHook_Siyan");
			if (ImGui::IsItemClicked()) {
				ShellExecuteA(NULL, "open", "https://github.com/SSSiyan/ReplicantHook_Siyan", NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Many thanks to anyone who helped!");
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

// Pass 0 as the targetProcessId to suspend threads in the current process
void DoSuspendThread(DWORD targetProcessId, DWORD targetThreadId) {
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
					// Suspend all threads EXCEPT the one we want to keep running
					if (te.th32ThreadID != targetThreadId && te.th32OwnerProcessID == targetProcessId) {
						HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != NULL) {
							SuspendThread(thread);
							CloseHandle(thread);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
}

// Pass 0 as the targetProcessId to suspend threads in the current process
void DoResumeThread(DWORD targetProcessId, DWORD targetThreadId) {
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
					// Suspend all threads EXCEPT the one we want to keep running
					if (te.th32ThreadID != targetThreadId && te.th32OwnerProcessID == targetProcessId) {
						HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != NULL) {
							ResumeThread(thread);
							CloseHandle(thread);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
}

DWORD WINAPI MainThread(LPVOID lpReserved) {
	wchar_t buffer[MAX_PATH]{ 0 };
	if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
		// Org dinput8.dll
		if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
			ExitProcess(0);
		}
	}

	// look for memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// wait for game to load
	// Sleep(10000);

#ifdef DEBUG
	// give user time to attach for debug
	DWORD pID = GetCurrentProcessId();
	DWORD tID = GetCurrentThreadId();
	DoSuspendThread(pID, tID);
	MessageBox(window, "Attach", NULL, MB_OK | MB_SYSTEMMODAL);
	DoResumeThread(pID, tID);
#endif

	// init imgui
	bool init_hook = false;
	do {
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
			kiero::bind(8, (void**)&oPresent, hkPresent);
			kiero::bind(13, (void**)&oResizeBuffers, hkResizeBuffers);
			init_hook = true;
		}
	} while (!init_hook);

	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
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
