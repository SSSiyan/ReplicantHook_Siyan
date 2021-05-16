#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

#include "../utils/config.hpp"

class ReplicantHook
{
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
	
	// values
	static int health;
	static float magic;
	static int level;
	static double playtime;
	static float x;
	static float y;
	static float z;
	static int gold;
	static int XP;
	static std::string zone;
	static std::string name;
	static int forceCharSelect_num;
	static float xyzpos[3];

	// dev values
	static DWORD _pID;
	static uintptr_t _baseAddress;
	static uintptr_t actorPlayable;
	static DWORD _getProcessID(void);
	static uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	static void _hook(void);
	static void _unHook(void);
	static void _patch(BYTE* destination, BYTE* src, unsigned int size);

	// getters
	static bool isHooked(void); // main.cpp startup check
	static int getGold();
	static int getXP();
	static std::string getZone();
	static std::string getName();
	static int getHealth();
	static float getMagic();
	static int getLevel();
	static double getPlaytime();
	static float getX();
	static float getY();
	static float getZ();

	// setters
	static void stealCursor(bool enabled);
	static void cursorForceHidden(bool enabled);
	static void setGold(int value);
	static void setXP(int value);
	static void forceModelsVisible(bool enabled);
	static void infiniteJumps(bool enabled);
	static void infiniteAirCombos(bool enabled);
	static void dealNoDamage(bool enabled);
	static void takeNoDamage(bool enabled);
	static void InfiniteMagic(bool enabled);
	static void setZone(std::string value);
	static void setName(std::string value);
	static void setHealth(int value);
	static void setMagic(float value);
	static void setLevel(int value);
	static void setPlaytime(double value);
	static void setX(float value);
	static void setY(float value);
	static void setZ(float value);
	static void forceCharSelect(int character);
	static void forceEndgameStats(bool enabled);

	// dev functions
	static DWORD getProcessID(void);
	static uintptr_t getBaseAddress(void);
	static void start(void);
	static void stop(void);
	static void hookStatus(void);
	static void update();
	template <typename T>
	static T readMemory(uintptr_t address);
	template <typename T>
	static T readMemoryPointer(uintptr_t address);
	template <typename T>
	static void writeMemory(uintptr_t address, T value);
	static std::string readMemoryString(uintptr_t address);
	static void writeMemoryString(uintptr_t address, std::string value);
	static void onConfigLoad(const utils::Config& cfg);
	static void onConfigSave(utils::Config& cfg);
private:
};
