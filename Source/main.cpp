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
#include <algorithm>

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

float trainerWidth = 0.0f;
float trainerMaxHeight = 0.0f;
float trainerHeightBorder = 0.0f;
float trainerVariableHeight = 0.0f;
float sameLineWidth = 0.0f;
float inputItemWidth = 0.0f;

void HelpMarker(const char* desc) {
	ImGui::SameLine();
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
		ReplicantHook::stealCursor(true);
	else
		ReplicantHook::stealCursor(false);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
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

    pDevice->CreateRenderTargetView(pBuffer, NULL, &mainRenderTargetView);

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

void InitHook() {
	// get process ID and module base address
	ReplicantHook::_hook();

	// load settings, must happen after hook
	ReplicantHook::onConfigLoad(cfg);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
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

	// keybinds
	// open menu
	if (GetAsyncKeyState(VK_DELETE) & 1) {
		OpenedHook();
	}
	// toggle gamepad cursor display
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		ReplicantHook::cursorForceHidden_toggle =! ReplicantHook::cursorForceHidden_toggle;
		ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
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
		goto imgui_finish;
	}


	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0)), ImGuiCond_Always;
	ImGui::SetNextWindowSize(ImVec2(trainerWidth, trainerVariableHeight)), ImGuiCond_Always;

	ImGui::Begin("Siyan's Replicant Hook 1.31", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	trainerWidth = 40.0f * ImGui::GetFontSize();
	trainerMaxHeight = ImGui::GetIO().DisplaySize.y * 0.6f;
	trainerHeightBorder = ImGui::GetFontSize() * 6.0f;
	sameLineWidth = trainerWidth * 0.5f;
	inputItemWidth = trainerWidth * 0.3f;

	if (ImGui::Button("Save config")) {
		ReplicantHook::onConfigSave(cfg);
	}

	if (ImGui::BeginTabBar("Trainer", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
		// static values
		int* playerLevel			= (int*)0x7FF69C1A4A84;
		int* playerXP				= (int*)0x7FF69C1A4A90;
		int* playerGold			= (int*)0x7FF69C1A4ADC;
		int* playerHP			= (int*)0x7FF69C1A4A6C;
		float* playerMP			= (float*)0x7FF69C1A4A78;
		const char* playerZone	= (const char*)0x7FF69C1A4A24;
		const char* playerName	= (const char*)0x7FF69C1A4A4C;
		bool* playerEndgame     = (bool*)0x7FF69C18AF40;

		// character values
		uintptr_t* actorPtr		= (uintptr_t*)0x7FF69A529560;
		uintptr_t actorBase		= *actorPtr;
		float* playerXPos		= (float*)(actorBase + 0x9C);
		float* playerYPos		= (float*)(actorBase + 0xAC);
		float* playerZPos		= (float*)(actorBase + 0xBC);
		
		if (ImGui::BeginTabItem("General")) {
			ImGui::BeginChild("GeneralChild");
			if (ImGui::Checkbox("Take No Damage", &ReplicantHook::takeNoDamage_toggle)) {
				ReplicantHook::takeNoDamage(ReplicantHook::takeNoDamage_toggle);
			}
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Deal No Damage", &ReplicantHook::dealNoDamage_toggle)) {
				ReplicantHook::dealNoDamage(ReplicantHook::dealNoDamage_toggle);
			}
			
			if (ImGui::Checkbox("Infinite Air Combos", &ReplicantHook::infiniteAirCombos_toggle)) {
				ReplicantHook::infiniteAirCombos(ReplicantHook::infiniteAirCombos_toggle);
			}
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Infinite Jumps", &ReplicantHook::infiniteJumps_toggle)) {
				ReplicantHook::infiniteJumps(ReplicantHook::infiniteJumps_toggle);
			}

			if (ImGui::Checkbox("Infinite Magic", &ReplicantHook::infiniteMagic_toggle)){
				ReplicantHook::infiniteMagic(ReplicantHook::infiniteMagic_toggle);
			}

			ImGui::Separator();

			ImGui::Checkbox("Spoiler Warning", &ReplicantHook::spoiler_toggle);
			HelpMarker("Do not tick this unless you have finished every ending.");
			if (ReplicantHook::spoiler_toggle) {
				ImGui::Checkbox("Force Character Change", &ReplicantHook::forceCharSelect_toggle);
				HelpMarker("This will apply after the next load screen");
				if (ReplicantHook::forceCharSelect_toggle) {
					ImGui::PushItemWidth(120);
					ImGui::Combo("##CharSelectDropdown", &ReplicantHook::forceCharSelect_num, characterNameStrings.data(), characterNameStrings.size());
					ImGui::PopItemWidth();
				}
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Stats")) {
			ImGui::BeginChild("StatsChild");
			if (actorBase) {
				ImGui::PushItemWidth(inputItemWidth);
				ImGui::Text("Gold:");
				ImGui::InputInt("##CharacterGoldInput", playerGold, 100, 1000);
				ImGui::Text("Level:");
				ImGui::InputInt("##CharacterLevelInput", playerLevel, 1, 10);
				ImGui::Text("XP:");
				ImGui::InputInt("##CharacterXPInput", playerXP, 100, 1000);
				ImGui::Text("HP:");
				ImGui::InputInt("##CharacterHPInput", playerHP, 10, 100);
				ImGui::Text("MP:");
				ImGui::InputFloat("##CharacterMPInput", playerMP, 10, 100);
				ImGui::Text("Player Position:");
				ImGui::InputFloat("##PlayerPositionXInput", playerXPos);
				ImGui::SameLine();
				ImGui::InputFloat("##PlayerPositionYInput", playerYPos);
				ImGui::SameLine();
				ImGui::InputFloat("##PlayerPositionZInput", playerZPos);
				ImGui::PopItemWidth();
				ImGui::Text("Zone Name:");
				ImGui::SameLine();
				ImGui::Text(playerZone);
				ImGui::Text("Save Name:");
				ImGui::SameLine();
				ImGui::Text(playerName);
			}
			else {
				ImGui::Text("Game Not Loaded.");
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Inventory")) {
			ImGui::BeginChild("InventoryChild");
			// Recovery
			std::vector<std::pair<std::string, uintptr_t>> recoveryInventoryVec;
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Medicinal Herb", 0x4374AE0);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Health Salve", 0x4374AE1);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Recovery Potion", 0x4374AE2);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Strength Drop", 0x4374AF5);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Strength Capsule", 0x4374AF6);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Magic Drop", 0x4374AF7);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Magic Capsule", 0x4374AF8);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Defense Drop", 0x4374AF9);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Defense Capsule", 0x4374AFA);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Spirit Drop", 0x4374AFB);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Spirit Capsule", 0x4374AFC);
			recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Antidotal Weed", 0x4374AFF);
			// Cultivation
			std::vector<std::pair<std::string, uintptr_t>> cultivationInventoryVec;
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Speed Fertilizer", 0x4374B09);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Flowering Fertilizer", 0x4374B0A);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bounty Fertilizer", 0x4374B0B);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pumpkin Seed", 0x4374B0E);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Watermelon Seed", 0x4374B0F);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Melon Seed", 0x4374B10);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gourd Seed", 0x4374B11);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tomato Seed", 0x4374B12);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Eggplant Seed", 0x4374B13);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bell Pepper Seed", 0x4374B14);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Been Seed", 0x4374B15);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Wheat Seed", 0x4374B16);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Rice Plant Seed", 0x4374B17);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Dahlia Seed", 0x4374B18);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tulip Seed", 0x4374B19);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Freesia Bulb", 0x4374B1A);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Red Moonflower Seed", 0x4374B1B);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gold Moonlfower Seed", 0x4374B1C);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Peach Moonflower Seed", 0x4374B1D);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pink Moonflower Seed", 0x4374B1E);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Blue Moonflower Seed", 0x4374B1F);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Indigo Moonflower Seed", 0x4374B20);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("White Moonflower Seed", 0x4374B21);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pumpkin", 0x4374B27);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Watermelon", 0x4374B28);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Melon", 0x4374B29);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gourd", 0x4374B2A);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tomato", 0x4374B2B);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Eggplant", 0x4374B2C);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bell Pepper", 0x4374B2D);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Beans", 0x4374B2E);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Wheat", 0x4374B2F);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Rice", 0x4374B30);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Dahlia", 0x4374B31);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tulip", 0x4374B32);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Freesia", 0x4374B33);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Red Moonflower", 0x4374B34);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gold Moonlfower", 0x4374B35);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Peach Moonflower", 0x4374B36);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pink Moonflower", 0x4374B37);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Blue Moonflower", 0x4374B38);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Indigo Moonflower", 0x4374B39);
			cultivationInventoryVec.emplace_back<std::string, uintptr_t>("White Moonflower", 0x4374B3A);
			// Fishing
			std::vector<std::pair<std::string, uintptr_t>> fishingInventoryVec;
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Lugworm", 0x4374B45);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Earthworm", 0x4374B46);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Lure", 0x4374B47);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Sardine", 0x4374B4F);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Carp", 0x4374B50);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Blowfish", 0x4374B51);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Bream", 0x4374B52);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Shark", 0x4374B53);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Blue Marlin", 0x4374B54);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Dunkleosteus", 0x4374B55);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Rainbow Trout", 0x4374B56);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Black Bass", 0x4374B57);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Giant Catfish", 0x4374B58);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Royal Fish", 0x4374B59);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Hyneria", 0x4374B5A);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Sandfish", 0x4374B5B);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Rhizodont", 0x4374B5C);
			fishingInventoryVec.emplace_back<std::string, uintptr_t>("Shaman Fish", 0x4374B5D);
			// Materials
			std::vector<std::pair<std::string, uintptr_t>> materialsInventoryVec;
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Aquatic Plant", 0x4374B63);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Deadwood", 0x4374B64);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusty Bucket", 0x4374B65);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Empty Can", 0x4374B66);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Gold Ore", 0x4374B6A);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Silvert Ore", 0x4374B6B);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Copper Ore", 0x4374B6C);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Iron Ore", 0x4374B6D);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Crystal", 0x4374B6E);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pyrite", 0x4374B6F);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Moldavite", 0x4374B70);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Meteorite", 0x4374B71);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Amber", 0x4374B72);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Fluorite", 0x4374B73);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Clay", 0x4374B74);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Berries", 0x4374B79);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Royal Fern", 0x4374B7A);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Tree Branch", 0x4374B7B);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Log", 0x4374B7C);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Natural Rubber", 0x4374B7D);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Ivy", 0x4374B7E);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Lichen", 0x4374B7F);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mushroom", 0x4374B80);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Sap", 0x4374B81);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mutton", 0x4374B87);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Meat", 0x4374B88);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wool", 0x4374B89);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Hide", 0x4374B8A);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wolf Hide", 0x4374B8B);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wolf Fang", 0x4374B8C);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Giant Spider Silk", 0x4374B8D);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bat Fang", 0x4374B8E);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bat Wing", 0x4374B8F);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Goat Meat", 0x4374B90);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Goat Hide", 0x4374B91);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Venison", 0x4374B92);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rainbow Spider Silk", 0x4374B93);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Liver", 0x4374B94);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Scorpion Claw", 0x4374B95);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Scorpion Tail", 0x4374B96);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dented Metal Board", 0x4374B97);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Stripped Bolt ", 0x4374B98);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Lens", 0x4374B99);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Severed Cable", 0x4374B9A);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Arm", 0x4374B9B);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Antena", 0x4374B9C);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Motor", 0x4374B9D);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Battery", 0x4374B9E);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mysterious Switch", 0x4374B9F);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Large Gear", 0x4374BA0);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Titanium Alloy", 0x4374BA1);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Memory Alloy", 0x4374BA2);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusted Clump", 0x4374BA3);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Machine Oil", 0x4374BA4);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Forlorn Necklace", 0x4374BA9);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Twisted Ring", 0x4374BAA);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Earring", 0x4374BAB);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pretty Choker", 0x4374BAC);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Metal Piercing", 0x4374BAD);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Subdued Bracelet", 0x4374BAE);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Technical Guide", 0x4374BAF);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Grubby Book", 0x4374BB0);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Thick Dictionary", 0x4374BB1);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Closed Book", 0x4374BB2);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Used Coloring Book", 0x4374BB3);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Old Schoolbook", 0x4374BB4);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dirty Bag", 0x4374BB5);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Flashyh Hat", 0x4374BB6);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Leather Gloves", 0x4374BB7);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Silk Handkerchief", 0x4374BB8);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Leather Boots", 0x4374BB9);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Complex Machine", 0x4374BBA);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Elaborate Machine", 0x4374BBB);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Simple Machine", 0x4374BBC);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Stopped Clock", 0x4374BBD);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Wristwatch", 0x4374BBE);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusty Kitchen Knife", 0x4374BBF);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Saw", 0x4374BC0);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dented Metal Bat", 0x4374BC1);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Shell", 0x4374BC3);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Gastropod", 0x4374BC4);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bivalve", 0x4374BC5);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Seaweed", 0x4374BC6);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Empty Bottle", 0x4374BC7);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Driftwood", 0x4374BC8);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pearl", 0x4374BC9);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Black Pearl", 0x4374BCA);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Crab", 0x4374BCB);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Starfish", 0x4374BCC);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Sea Turtle Egg", 0x4374BD2);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Pottery", 0x4374BD3);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Desert Rose", 0x4374BD4);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Giant Egg", 0x4374BD5);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Damascus Steel", 0x4374BD6);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Eagle Egg", 0x4374BD7);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Chicken Egg", 0x4374BD8);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mouse Tail", 0x4374BDA);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Lizard Tail", 0x4374BDB);
			materialsInventoryVec.emplace_back<std::string, uintptr_t>("Deer Antler", 0x4374BDF);
			// Key
			std::vector<std::pair<std::string, uintptr_t>> keyInventoryVec;
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Moon Key", 0x4374BE0);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Star Key", 0x4374BE1);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Light Key", 0x4374BE2);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Darkness Key", 0x4374BE3);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Fine Flour", 0x4374BE4);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Coarse FLour", 0x4374BE5);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Perfume Bottle", 0x4374BE6);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Postman's Parcel", 0x4374BE7);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Lover's Letter", 0x4374BE8);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Water Filter", 0x4374BE9);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Royal Compass", 0x4374BEA);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Vapor Moss", 0x4374BEB);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Valley Spoder Silk", 0x4374BEC);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Animal Guidebook", 0x4374BED);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Ore Guidebook", 0x4374BEE);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Plant Guidebook", 0x4374BEF);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Book", 0x4374BF0);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Blue Book", 0x4374BF1);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Lady's Elixer", 0x4374BF2);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Lady's Elixer+", 0x4374BF3);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for The Aerie", 0x4374BF4);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for Seafront", 0x4374BF5);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Cookbook", 0x4374BF6);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for Facade", 0x4374BF7);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Max's Herbs", 0x4374BF8);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo", 0x4374BF9);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 2", 0x4374BFA);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 3", 0x4374BFB);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 4", 0x4374BFC);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Package", 0x4374BFD);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Mermaid Tear", 0x4374BFE);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Mandrake Leaf", 0x4374BFF);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Energizer", 0x4374C00);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Tad Oil", 0x4374C01);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Sleep-B-Gone", 0x4374C02);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Antidote", 0x4374C03);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Gold Bracelet", 0x4374C04);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Elite Kitchen Knife", 0x4374C05);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Elevator Parts", 0x4374C06);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Dirty Treasure Map", 0x4374C07);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Restored Treasure Map", 0x4374C08);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Jade Hair Ornament", 0x4374C09);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Employee List", 0x4374C0A);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Small Safe", 0x4374C0B);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Safe Key", 0x4374C0C);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Greaty Tree Root", 0x4374C0D);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Eye of Power", 0x4374C0E);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Ribbon", 0x4374C0F);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Yonah's Ribbon", 0x4374C10);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Bronze Key", 0x4374C11);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Brass Key", 0x4374C12);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Boar Tusk", 0x4374C13);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Pressed Freesia", 0x4374C14);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Potted Freesia", 0x4374C15);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Fressia (Delivery)", 0x4374C16);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Pile of Junk", 0x4374C17);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Gold Coin", 0x4374C18);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Market Map", 0x4374C19);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("AA Keycard", 0x4374C1A);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("KA Keycard", 0x4374C1B);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("SA Keycard", 0x4374C1C);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("TA Keycard", 0x4374C1D);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("NA Keycard", 0x4374C1E);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("HA Keycard", 0x4374C1F);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("MA Keycard", 0x4374C20);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("YA Keycard", 0x4374C21);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("RA Keycard", 0x4374C22);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("WA Keycard", 0x4374C23);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Cultivator's Handbook", 0x4374C24);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Bag", 0x4374C25);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Lantern", 0x4374C26);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Empty Lantern", 0x4374C27);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Hold Key ", 0x4374C28);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Passageway Key", 0x4374C29);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Goat Key", 0x4374C2A);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Lizard Key", 0x4374C2B);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Unlocking Procedure Memo ", 0x4374C2C);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Jewel", 0x4374C2D);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Flowers ", 0x4374C2E);
			keyInventoryVec.emplace_back<std::string, uintptr_t>("Apples", 0x4374C2F);

			if (actorBase) {
				ImGui::Text("Recovery");
				for (auto& inventoryEntry : recoveryInventoryVec) {
					static int index = 0;
					uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
					int* inventoryItem = (int*)inventoryItemAddress;
					ImGui::PushItemWidth(inputItemWidth / 5);
					ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
					ImGui::PopItemWidth();
					if (index % 2 == 0 && index != recoveryInventoryVec.size())
						ImGui::SameLine(sameLineWidth);
					index++;
				}
				ImGui::Text("Cultivation");
				for (auto& inventoryEntry : cultivationInventoryVec) {
					static int index = 0;
					uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
					int* inventoryItem = (int*)inventoryItemAddress;
					ImGui::PushItemWidth(inputItemWidth / 5);
					ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
					ImGui::PopItemWidth();
					if (index % 2 == 0 && index != cultivationInventoryVec.size())
						ImGui::SameLine(sameLineWidth);
					index++;
				}
				ImGui::Text("Fishing");
				for (auto& inventoryEntry : fishingInventoryVec) {
					static int index = 0;
					uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
					int* inventoryItem = (int*)inventoryItemAddress;
					ImGui::PushItemWidth(inputItemWidth / 5);
					ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
					ImGui::PopItemWidth();
					if (index % 2 == 0  && index != fishingInventoryVec.size())
						ImGui::SameLine(sameLineWidth);
					index++;
				}
				ImGui::Text("Materials");
				for (auto& inventoryEntry : materialsInventoryVec) {
					static int index = 0;
					uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
					int* inventoryItem = (int*)inventoryItemAddress;
					ImGui::PushItemWidth(inputItemWidth / 5);
					ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
					ImGui::PopItemWidth();
					if (index % 2 == 0 && index != materialsInventoryVec.size())
						ImGui::SameLine(sameLineWidth);
					index++;
				}
				ImGui::Text("Key");
				for (auto& inventoryEntry : keyInventoryVec) {
					static int index = 0;
					uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
					int* inventoryItem = (int*)inventoryItemAddress;
					ImGui::PushItemWidth(inputItemWidth / 5);
					ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
					ImGui::PopItemWidth();
					if (index % 2 == 0 && index != keyInventoryVec.size())
						ImGui::SameLine(sameLineWidth);
					index++;
				}
			}
			else {
				ImGui::Text("Game Not Loaded.");
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("System")) {
			ImGui::BeginChild("SystemChild");
			if (ImGui::Checkbox("Disable cursor", &ReplicantHook::cursorForceHidden_toggle)) { // toggle 
				ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			}
			HelpMarker("Disable the cursor display while using a gamepad. This can be toggled mid play with INSERT");
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Force Models Visibile", &ReplicantHook::forceModelsVisible_toggle)) { // toggle
				ReplicantHook::forceModelsVisible(ReplicantHook::forceModelsVisible_toggle);
			}
			HelpMarker("Stop models becoming transparent when the camera gets too close");

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Credits")) {
			ImGui::BeginChild("CreditsChild");
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

			ImGui::Separator();

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

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
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
