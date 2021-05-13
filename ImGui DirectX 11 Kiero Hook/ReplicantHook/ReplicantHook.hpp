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
	static bool _hooked;

	DWORD _getProcessID(void);
	uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	void _hook(void);
	void _unHook(void);
	static void _patch(BYTE* destination, BYTE* src, unsigned int size);
	template <typename T>
	static T readMemory(uintptr_t address);
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

	// getters
	bool isHooked(void); // main.cpp startup check
	static int getGold();
	static std::string getZone();
	static std::string getName();
	int getHealth();
	float getMagic();
	int getLevel();
	double getPlaytime();
	float getX();
	float getY();
	float getZ();
	std::string getActorModel();

	// toggles
	static bool cursorForceHidden_toggle;
	static bool forceModelsVisible_toggle;
	static bool infiniteJumps_toggle;
	static bool infiniteAirCombos_toggle;
	static bool forceCharSelect_toggle;

	// setters
	static void stealCursor(bool enable);
	static void cursorForceHidden(bool enable);
	static void setGold(int value);
	static void forceModelsVisible(bool enable);
	static void infiniteJumps(bool enable);
	static void infiniteAirCombos(bool enable);

	void setZone(std::string value);
	void setName(std::string value);
	void setHealth(int value);
	void setMagic(float value);
	void setLevel(int value);
	void setPlaytime(double value);
	void setX(float value);
	void setY(float value);
	void setZ(float value);
	void setPosition(float x, float y, float z);
	static void setActorModel(std::string model);
	static std::string setActorModelConvert(int value);

	// cheats
	void InfiniteHealth(bool enabled);
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
	static std::string zone;
	static std::string name;
	static int forceCharSelect_num;

	// saving and loading
	static void onConfigLoad(const utils::Config& cfg);
	static void onConfigSave(utils::Config& cfg);
};

template<typename T>
inline T ReplicantHook::readMemory(uintptr_t address)
{
	T value;
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ReplicantHook::_pID);
	ReadProcessMemory(pHandle, (LPCVOID)(ReplicantHook::_baseAddress + address), &value, sizeof(value), NULL);
	CloseHandle(pHandle); // close handle to prevent memory leaks
	return value;
}

template<typename T>
inline void ReplicantHook::writeMemory(uintptr_t address, T value)
{
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, ReplicantHook::_pID);
	WriteProcessMemory(pHandle, (LPVOID)(ReplicantHook::_baseAddress + address), &value, sizeof(value), NULL);
	CloseHandle(pHandle);
}

inline std::string ReplicantHook::readMemoryString(uintptr_t address)
{
	char val[20];
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ReplicantHook::_pID);
	ReadProcessMemory(pHandle, (LPCVOID)(ReplicantHook::_baseAddress + address), &val, sizeof(val), NULL);
	CloseHandle(pHandle); // close handle to prevent memory leaks
	return std::string(val);
}

inline void ReplicantHook::writeMemoryString(uintptr_t address, std::string value)
{
	SIZE_T BytesToWrite = value.length() + 1;
	SIZE_T BytesWritten;
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ReplicantHook::_pID);
	WriteProcessMemory(pHandle, (LPVOID)(ReplicantHook::_baseAddress + address), (LPCVOID)value.c_str(), BytesToWrite, &BytesWritten);
}
