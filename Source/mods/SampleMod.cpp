#include "SampleMod.hpp"
#include "../imgui/imgui.h"

std::string SampleMod::OnInitialize()
{
	// Code that gets executed ones on creation goes here

	if (auto res = InitializeDetour1(); res == false) {
		return "SampleMod::InitializeDetour1() failed!";
	}

	if (auto res = InitializeDetour2(); res == false) {
		return "SampleMod::InitializeDetour1() failed!";
	}

	m_LastDetour1Enabled = m_Detour1Enabled;

	return {}; // We return the error as a string or if no error we return an empty string "{}"
}

void SampleMod::OnFrame()
{
	// Gets executed before drawing the ui and on all frames whether ui is drawn or not
}

void SampleMod::OnDrawUI()
{
	// UI Code here

	ImGui::Checkbox("SampleMode::Detour1", &m_Detour1Enabled);
	
	if (m_LastDetour1Enabled != m_Detour1Enabled) {
		m_Detour1->Toggle(m_Detour1Enabled);
	}

	ImGui::Checkbox("SampleMode::Detour2", &m_Detour2Enabled);

	if (m_LastDetour2Enabled != m_Detour2Enabled) {
		m_Detour2->Toggle(m_Detour2Enabled);
	}

	m_LastDetour1Enabled = m_Detour1Enabled;
	m_LastDetour2Enabled = m_Detour2Enabled;
}

void SampleMod::OnDestroy()
{
	// Gets executed when the mod object is destroyed or can be manually called before that

	// Detour::Remove() checks if the detour is valid automatically internally and returns a bool
	// which indicates whether the remove operation was successful or not
	if (m_Detour1->Remove()) {
		printf("Well fuck, failed to remove SampleMod::m_Detour1!");
	}
	if (m_Detour2->Remove()) {
		printf("Well fuck, failed to remove SampleMod::m_Detour1!");
	}
}

void SampleMod::OnConfigSave(utils::Config& config)
{
	// Save the state
}

void SampleMod::OnConfigLoad(const utils::Config& config)
{
	// Load the state
}

// Mod specific logic
extern "C" {
	// Detour #1
	void* g_SampleMod_ReturnAddr1 = nullptr;
	void SampleModDetour1();

	// Detour #2
	void* g_SampleMod_ReturnAddr2 = nullptr;
	void SampleModDetour2();
}

bool SampleMod::InitializeDetour1()
{
	void* aobToReplaceAddr = nullptr; // The address you want to put your code in
	void* detourAddr = &SampleModDetour1; // The address of your detour function

	m_Detour1 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr, &g_SampleMod_ReturnAddr1);

	return m_Detour1->IsEnabled();
}

bool SampleMod::InitializeDetour2()
{
	void* aobToReplaceAddr = nullptr; // The address you want to put your code in
	void* detourAddr = &SampleModDetour2; // The address of your detour function

	m_Detour2 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr, &g_SampleMod_ReturnAddr2);

	return m_Detour2->IsEnabled();
}
