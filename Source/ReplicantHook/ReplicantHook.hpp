#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "../imgui/imgui.h"
#include "../utils/config.hpp"
#include <vector>

class ReplicantHook {
public:

	static bool sampleMod1Init;
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

	static void getInventoryAddresses(void);
	static uintptr_t playerActor;
	static uintptr_t playerLevel;
	static uintptr_t playerXP;
	static uintptr_t playerGold;
	static uintptr_t playerHP;
	static uintptr_t playerMP;
	static uintptr_t playerZone;
	static uintptr_t playerName;

	// other
	static uintptr_t enableInput;
	static uintptr_t showCursor;
	static uintptr_t currentCharacter;
	static uintptr_t playerEndgame;
	static std::vector<std::pair<std::string, uintptr_t>> recoveryInventoryVec;
	static std::vector<std::pair<std::string, uintptr_t>> cultivationInventoryVec;
	static std::vector<std::pair<std::string, uintptr_t>> fishingInventoryVec;
	static std::vector<std::pair<std::string, uintptr_t>> materialsInventoryVec;
	static std::vector<std::pair<std::string, uintptr_t>> keyInventoryVec;

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
