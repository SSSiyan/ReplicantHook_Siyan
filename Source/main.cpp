#define DIRECTINPUT_VERSION 0x0800
// #include <d3d11.h>
// #include <thread>
// #include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <dinput.h>
#include "ReplicantHook/ReplicantHook.hpp"
#include "Mods.hpp"

#define EXCLUDE_RHOOK_D3D9
#define EXCLUDE_RHOOK_D3D10
//#define EXCLUDE_RHOOK_D3D11
#define EXCLUDE_RHOOK_D3D12
#define EXCLUDE_RHOOK_OGL
#define EXCLUDE_RHOOK_VULKAN
#define EXCLUDE_RHOOK_PRESENTHOOK
#define EXCLUDE_RHOOK_UTILITY
#include <RenderHook.hpp>

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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

static bool imguiInit = false;
static bool imguiDraw = false;

void OpenedHook() {
	imguiDraw = !imguiDraw;
	if (imguiDraw)
		ReplicantHook::stealCursor(true);
	else
		ReplicantHook::stealCursor(false);
}

void __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (!imguiInit) {
		return;
	}

	if (uMsg == WM_KEYDOWN){
		// keybinds
		switch (wParam){
		case VK_DELETE: // open menu
			OpenedHook();
			break;
		case VK_INSERT: // toggle cursor
			ReplicantHook::cursorForceHidden_toggle = !ReplicantHook::cursorForceHidden_toggle;
			ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			break;
		default:
			break;
		}
	}

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return;
}

// 4hook styling
void ImGuiHookStyle(void) {
	ImGui::GetStyle().FrameRounding = 2.5f;
	ImGui::GetStyle().GrabRounding = 3.0f;
	auto& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(6, 4);
	style.WindowRounding = 6.0f;
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_Button] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.25f, 0.29f, 0.95f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.0078f, 0.53f, 0.8196f, 0.95f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.4588f, 0.45880f, 0.4588f, 0.35f);
}

void InitImGui() {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// font load examples
	// io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 15.0f);
	// io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\impact.ttf", 15.0f);
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiHookStyle();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

void hkResizeBuffers(RHook::D3D11Hook* pd3d11Hook) {
	imguiInit = false;

	ImGui_ImplDX11_Shutdown();

    if (mainRenderTargetView) {
        pContext->OMSetRenderTargets(0, 0, 0);
        mainRenderTargetView->Release();
    }
}

void InitHook() {
	// get process ID and module base address
	ReplicantHook::_hook();

	// get inventory addresses
	ReplicantHook::getInventoryAddresses();

	// load settings, must happen after hook
	ReplicantHook::onConfigLoad(ReplicantHook::cfg);
	Mods::GetInstance()->LoadConfig(ReplicantHook::cfg);

	// init mods if they are not initialized yet (can be done on another thread)
	if (!Mods::GetInstance()->IsInitialized()) {
		if (auto err = Mods::GetInstance()->Initialize(); !err.empty())
		{
			printf("%s\n", err.c_str());
			return; // We failed like a bitch, we don't continue
		}
	}
	
	if (const auto sampleMod = Mods::GetInstance()->GetMod("SampleMod")) {
		ReplicantHook::sampleMod1Init = sampleMod->IsInitialized();
	}
}

std::unique_ptr<WindowsMessageHook> g_WindowsMessageHook{};

void __stdcall hkPresent(RHook::D3D11Hook* pd3d11Hook) {
	IDXGISwapChain* pSwapChain = pd3d11Hook->GetPresentParams().swapChain.get();

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
			g_WindowsMessageHook.reset();
			g_WindowsMessageHook = std::make_unique<WindowsMessageHook>(window);

			g_WindowsMessageHook->OnPreMessage([&](WindowsMessageHook&) {
				auto& params = g_WindowsMessageHook->GetWNDPROCParams();

				WndProc(params.hWnd, params.Msg, params.wParam, params.lParam);
			});

			// create imgui context, apply style
			InitImGui();

			// get game addresses, onConfigLoad
			InitHook();

			imguiInit = true;
		}
		else
			return;
	}

	if (Mods::GetInstance()->IsInitialized()) {
		Mods::GetInstance()->OnFrame();
	}

	// check process ID is valid
	if (ReplicantHook::_pID != ReplicantHook::_getProcessID()) {
		goto imgui_finish;
	}

	// force character on tick
	if (ReplicantHook::forceCharSelect_toggle && ReplicantHook::spoiler_toggle) {
		ReplicantHook::forceCharSelect(ReplicantHook::forceCharSelect_num);
	
		// if character is 4, force old save stats
		if (ReplicantHook::forceCharSelect_num == 4) {
			ReplicantHook::forceEndgameStats(true);
		}
	}

	// if display is toggled off, don't display imgui menu
	if (!imguiDraw) {
		return;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0)), ImGuiCond_Always;
	ImGui::SetNextWindowSize(ImVec2(ReplicantHook::trainerWidth, ReplicantHook::trainerVariableHeight)), ImGuiCond_Always;
	ImGui::Begin(ReplicantHook::dllName, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	Mods::GetInstance()->DrawUI();
	ReplicantHook::gameGui();
	ImGui::End();

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

imgui_finish:
	return;
}

std::unique_ptr<RHook::D3D11Hook> g_D3D11Hook;
std::recursive_mutex g_HookMonitorMutex{};

DWORD WINAPI MainThread(LPVOID lpReserved) {
	RHook::Init();

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
	// Sleep(5000);

	// init imgui
	g_D3D11Hook = std::make_unique<RHook::D3D11Hook>(&g_HookMonitorMutex);
	g_D3D11Hook->OnPrePresent([&](RHook::DXGIHook&) { hkPresent(g_D3D11Hook.get()); });
	g_D3D11Hook->OnPreResizeBuffers([&](RHook::DXGIHook&) { hkResizeBuffers(g_D3D11Hook.get()); });

	if (!g_D3D11Hook->Hook()) {
		printf("[D3D11] Failed to hook d3d11!");
	}

	return TRUE;
}

void ShutDown() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	Mods::GetInstance()->Destroy();
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
	if (RHook::IsHelperProcess()) {
		return TRUE;
	}

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
#ifndef NDEBUG
		MessageBox(NULL, "Debug attach opportunity", "Debug", MB_ICONINFORMATION);
#endif
		DisableThreadLibraryCalls(hMod);
		CloseHandle(CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr));
		break;
	case DLL_PROCESS_DETACH:
		ShutDown();
		break;
	}

	return TRUE;
}
