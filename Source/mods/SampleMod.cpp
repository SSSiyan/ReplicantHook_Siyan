#include "SampleMod.hpp"
#include "../utils/Detour.hpp"
#include <memory>

std::unique_ptr<utility::Detour_t> g_Detour1{}, g_Detour2{};

extern "C" {
	// Detour #1
	void* g_SampleMod_ReturnAddr1 = nullptr;
	void SampleModDetour1();

	// Detoue #2
	void* g_SampleMod_ReturnAddr2 = nullptr;
	void SampleModDetour2();
}

bool InitializeSampleMod1() {
	void* aobToReplaceAddr = nullptr; // The address you want to put your code in
	void* detourAddr = &SampleModDetour1; // The address of your detour function

	g_Detour1 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr);
	g_SampleMod_ReturnAddr1 = g_Detour1->Create();

	return g_SampleMod_ReturnAddr1 != nullptr ? true : false;
}

bool InitializeSampleMod2() {
	void* aobToReplaceAddr = nullptr; // The address you want to put your code in
	void* detourAddr = &SampleModDetour2; // The address of your detour function

	g_Detour2 = std::make_unique<utility::Detour_t>(aobToReplaceAddr, detourAddr);
	g_SampleMod_ReturnAddr2 = g_Detour2->Create();

	return g_SampleMod_ReturnAddr2 != nullptr ? true : false;
}