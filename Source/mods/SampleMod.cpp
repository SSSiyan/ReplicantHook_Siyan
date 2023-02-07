#include "SampleMod.hpp"
#include "../imgui/imgui.h"

// Mod specific logic
extern "C" {
	// Detour #1
	void* g_SampleMod_ReturnAddr1 = nullptr;
	void SampleModDetour1();

	// Detour #2
	void* g_SampleMod_ReturnAddr2 = nullptr;
	void SampleModDetour2();
}

bool SampleMod::InitializeDetour1() {
	void* aobToReplaceAddr = (void*)(ReplicantHook::_baseAddress + 0x6C28D8); // The address you want to put your code in
	void* detourAddr = &SampleModDetour1; // The address of your detour function
	m_Detour1 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr);
	return m_Detour1->IsValid();
}

// Common methods
std::string SampleMod::OnInitialize() {
	// Code that gets executed ones on creation goes here
	if (auto res = InitializeDetour1(); res == false) {
		return "SampleMod::InitializeDetour1() failed!";
	}
	m_LastDetour1Enabled = m_Detour1Enabled;
	m_IsInitialized = true;
	return {}; // We return the error as a string or if no error we return an empty string "{}"
}

void SampleMod::OnFrame() {
	// Gets executed before drawing the ui and on all frames whether ui is drawn or not
	if (m_IsInitialized) {
		if (m_Detour1->IsEnabled()) {
			// g_SampleMod_ReturnAddr1 = m_Detour1->GetTrampoline(); // If we want the return address to the original code
			g_SampleMod_ReturnAddr1 = m_Detour1->GetReturnAddress(m_Detour1ReturnOffset); // If wewant the return address to an offset of the detour jump place (In this case 7 bytes after it)
		}
	}
}

void SampleMod::OnDrawUI() {
	// UI Code here
	if (m_IsInitialized) {
		m_Detour1Enabled = m_Detour1->IsEnabled(); // We put this here so that the "m_Detour1Enabled" variable is updated before drawing each frame
		ImGui::Checkbox("SampleMod::Detour1", &m_Detour1Enabled);
		if (m_LastDetour1Enabled != m_Detour1Enabled) { // If the state changed we toggle the mod based on the new state as well
			m_Detour1Enabled = m_Detour1->Toggle(m_Detour1Enabled); // We assign the result of Toggle() to m_Detour1Enabled just to make sure it has the correct value even if Toggle() failed
			if (m_Detour1->IsEnabled()) {
				// g_SampleMod_ReturnAddr1 = m_Detour1->GetTrampoline(); // If we want the return address to the original code
				g_SampleMod_ReturnAddr1 = m_Detour1->GetReturnAddress(m_Detour1ReturnOffset); // If wewant the return address to an offset of the detour jump place (In this case 7 bytes after it)
			}
		}
		m_LastDetour1Enabled = m_Detour1Enabled;
	}
}

void SampleMod::OnDestroy() {
	// Gets executed when the mod object is destroyed or can be manually called before that
	// Detour::Remove() checks if the detour is valid automatically internally and returns a bool
	// which indicates whether the remove operation was successful or not
	if (m_Detour1->Remove()) {
		printf("Well fuck, failed to remove SampleMod::m_Detour1!");
	}
}

void SampleMod::OnConfigSave(utils::Config& config) {
	// Save the state
}

void SampleMod::OnConfigLoad(const utils::Config& config) {
	// Load the state
}
