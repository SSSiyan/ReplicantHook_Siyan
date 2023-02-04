#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "../imgui/imgui.h"
#include "../utils/config.hpp"

class ReplicantHook {
public:
	// patches
	// static bool imguiDraw;
	static void stealCursor(bool enabled);
	static bool cursorForceHidden_toggle;
	static void cursorForceHidden(bool enabled);
	static bool forceModelsVisible_toggle;
	static void forceModelsVisible(bool enabled);
	static bool infiniteJumps_toggle;
	static void infiniteJumps(bool enabled);
	static bool infiniteAirCombos_toggle;
	static void infiniteAirCombos(bool enabled);
	static bool dealNoDamage_toggle;
	static void dealNoDamage(bool enabled);
	static bool takeNoDamage_toggle;
	static void takeNoDamage(bool enabled);
	static bool infiniteMagic_toggle;
	static void infiniteMagic(bool enabled);
	static bool spoiler_toggle;
	static bool forceCharSelect_toggle;
	static int forceCharSelect_num;
	static void forceCharSelect(int character);
	static void forceEndgameStats(bool enabled);

	// dev
	static DWORD _getProcessID(void);
	static uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	static void _hook(void);
	static void _patch(char* dst, char* src, int size);
	static void _nop(char* dst, unsigned int size);
	static void onConfigLoad(const utils::Config& cfg);
	static void onConfigSave(utils::Config& cfg);

	static bool _hooked;
	static DWORD _pID;
	static uintptr_t _baseAddress;
	static uintptr_t actorPlayable;
	static inline utils::Config cfg{ "Replicant_Hook.cfg" };
	static inline const char* cfgString{ "Replicant_Hook.cfg" };
	static inline const char* dllName{"Siyan's Replicant Hook 1.32"};
	static inline const char* repoUrl{ "https://github.com/SSSiyan/ReplicantHook_Siyan" };

	// gui
	static void gameGui(void);
	static void under_line(const ImColor& col);

	static float trainerWidth;
	static float trainerMaxHeight;
	static float trainerHeightBorder;
	static float trainerVariableHeight;
	static float sameLineWidth;
	static float inputItemWidth;
private:
};
