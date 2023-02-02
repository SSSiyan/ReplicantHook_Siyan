#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

#include "../utils/config.hpp"

class ReplicantHook {
public:
	// toggle bools
	static bool _hooked;
	static bool cursorForceHidden_toggle;
	static bool forceModelsVisible_toggle;
	static bool infiniteJumps_toggle;
	static bool infiniteAirCombos_toggle;
	static bool forceCharSelect_toggle;
	static bool spoiler_toggle;
	static bool takeNoDamage_toggle;
	static bool dealNoDamage_toggle;
	static bool infiniteMagic_toggle;
	static int forceCharSelect_num;

	// dev values
	static DWORD _pID;
	static uintptr_t _baseAddress;
	static uintptr_t actorPlayable;

	static void stealCursor(bool enabled);
	static void cursorForceHidden(bool enabled);
	static void forceModelsVisible(bool enabled);
	static void infiniteJumps(bool enabled);
	static void infiniteAirCombos(bool enabled);
	static void dealNoDamage(bool enabled);
	static void takeNoDamage(bool enabled);
	static void infiniteMagic(bool enabled);

	static void forceCharSelect(int character);
	static void forceEndgameStats(bool enabled);

	// dev functions
	static DWORD _getProcessID(void);
	static uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	static void _hook(void);
	static void _patch(char* dst, char* src, int size);
	static void _nop(char* dst, unsigned int size);
	static void onConfigLoad(const utils::Config& cfg);
	static void onConfigSave(utils::Config& cfg);
private:
};
