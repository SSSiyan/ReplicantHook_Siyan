#include "SampleMod.hpp"
std::unique_ptr<utility::Detour_t> g_Detour1{}, g_Detour2{};

extern "C" {
	// Detour #1
	void* g_SampleMod_ReturnAddr1 = nullptr;
	void SampleModDetour1();
	bool shouldSampleModWork = true;

	// Detour #2
	void* g_SampleMod_ReturnAddr2 = nullptr;
	void SampleModDetour2();

}

// main.cpp ReplicantHook::sampleMod1Init = InitializeSampleMod1();
bool InitializeSampleMod1() {
	/*
	ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x6C28D8), 7); // nop addr
	void* aobToReplaceAddr = (void*)(ReplicantHook::_baseAddress + 0x6C28D8); // detour addr
	void* detourAddr = &SampleModDetour1;
	g_Detour1 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr);
	void* detour1Success = g_Detour1->Create();
	g_SampleMod_ReturnAddr1 = (void*)(ReplicantHook::_baseAddress + 0x6C28DF); // ret addr
	*/

	void* detour1Success = nullptr;
	return detour1Success != nullptr ? true : false;
}

bool InitializeSampleMod2() {
	void* aobToReplaceAddr = nullptr; // The address you want to put your code in
	void* detourAddr = &SampleModDetour2; // The address of your detour function

	g_Detour2 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr);
	g_SampleMod_ReturnAddr2 = g_Detour2->Create();

	return g_SampleMod_ReturnAddr2 != nullptr ? true : false;
}
