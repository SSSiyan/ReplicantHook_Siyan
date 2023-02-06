#pragma once
#include "Mod.hpp"
#include <memory>

class SampleMod : public Mod
{
public:
	SampleMod()
		: Mod("SampleName") // Name of the mod here
	{}

	std::optional<std::string> OnInitialize()	override; // Must define this function but can be empty and do nothing
	void					   OnFrame()		override; // Must define this function but can be empty and do nothing
	void					   OnDrawUi()		override; // Must define this function but can be empty and do nothing
	void					   OnDestroy()		override; // Must define this function but can be empty and do nothing
	bool					   IsInitialized()	override  // Must define this function but can be empty and do nothing
	{
		return m_IsInitialized;
	}

	void					   OnConfigSave()	override; // Must define this function but can be empty and do nothing
	void					   OnConfigLoad()	override; // Must define this function but can be empty and do nothing

private:
	bool m_IsInitialized = false;

	// Mod specific data goes here

	std::unique_ptr<utility::Detour_t> m_Detour1{}, m_Detour2{};
	
	bool m_LastDetour1Enabled = false;
	bool m_Detour1Enabled = false;

	bool InitializeDetour1();
	bool InitializeDetour2();
};
