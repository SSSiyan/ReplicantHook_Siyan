#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "../utils/config.hpp"
class ReplicantHook
{
private:
	// none of this needs to be private
	// values
	static DWORD _pID;
	static uintptr_t _baseAddress;
	static uintptr_t actorPlayable;
	

	DWORD _getProcessID(void);
	uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	void _hook(void);
	void _unHook(void);
	static void _patch(BYTE* destination, BYTE* src, unsigned int size);
	template <typename T>
	static T readMemory(uintptr_t address);
	template <typename T>
	static T readMemoryPointer(uintptr_t address);
	template <typename T>
	static void writeMemory(uintptr_t address, T value);
	static std::string readMemoryString(uintptr_t address);
	static void writeMemoryString(uintptr_t address, std::string value);

public:
	DWORD getProcessID(void);
	static uintptr_t getBaseAddress(void);
	void start(void);
	void stop(void);
	void hookStatus(void);
	void update();
	static bool _hooked;

	// getters
	bool isHooked(void); // main.cpp startup check
	static int getGold();
	static int getXP();
	static std::string getZone();
	static std::string getName();
	int getHealth();
	float getMagic();
	int getLevel();
	double getPlaytime();
	float getX();
	float getY();
	float getZ();
	// static void getCharBackup();

	// toggles
	static bool cursorForceHidden_toggle;
	static bool forceModelsVisible_toggle;
	static bool infiniteJumps_toggle;
	static bool infiniteAirCombos_toggle;
	static bool forceCharSelect_toggle;
	static bool spoiler_toggle;
	static bool takeNoDamage_toggle;
	static bool dealNoDamage_toggle;

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


	void setZone(std::string value);
	void setName(std::string value);
	void setHealth(int value);
	void setMagic(float value);
	void setLevel(int value);
	void setPlaytime(double value);
	static void setX(float value);
	static void setY(float value);
	static void setZ(float value);
	static void forceCharSelect(int character);
	static void forceEndgameStats(bool enabled);

	// cheats
	void InfiniteMagic(bool enabled);

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
	// static int charBackup;

	// saving and loading
	static void onConfigLoad(const utils::Config& cfg);
	static void onConfigSave(utils::Config& cfg);
};
