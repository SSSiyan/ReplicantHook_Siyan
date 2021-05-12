#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

class ReplicantHook
{
public:
	static DWORD _pID;
	static uintptr_t _baseAddress;
	static uintptr_t actorPlayable;
	static bool _hooked;

	// values
	static int gold;
	static std::string zone;
	static std::string name;
	static int health;
	static float magic;
	static int level;
	static double playtime;
	static float x;
	static float y;
	static float z;

	static DWORD _getProcessID(void);
	static uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	static void _hook(void);
	static void _unHook(void);
	static void _patch(BYTE* destination, BYTE* src, unsigned int size);
	template <typename T>
	static T readMemory(uintptr_t address);
	template <typename T>
	static void writeMemory(uintptr_t address, T value);
	static std::string readMemoryString(uintptr_t address);
	static void writeMemoryString(uintptr_t address, std::string value);
	static DWORD getProcessID(void);
	static uintptr_t getBaseAddress(void);
	static void start(void);
	static void stop(void);
	static void hookStatus(void);
	static void update();

	//Getters
	static bool isHooked(void); // main.cpp startup check
	static int getGold();
	static std::string getZone();
	static std::string getName();
	static int getHealth();
	static float getMagic();
	static int getLevel();
	static double getPlaytime();
	static float getX();
	static float getY();
	static float getZ();

	//Setters
	static void setGold(int value);
	static void setZone(std::string value);
	static void setName(std::string value);
	static void setHealth(int value);
	static void setMagic(float value);
	static void setLevel(int value);
	static void setPlaytime(double value);
	static void setX(float value);
	static void setY(float value);
	static void setZ(float value);
	static void setPosition(float x, float y, float z);

	//Cheats
	static void InfiniteHealth(bool enabled);
	static void InfiniteMagic(bool enabled);

	//Models
	static void setActorModel(std::string model);
	static std::string getActorModel();

private:

};

template<typename T>
inline T ReplicantHook::readMemory(uintptr_t address)
{
	T value;
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ReplicantHook::_pID);
	ReadProcessMemory(pHandle, (LPCVOID)(ReplicantHook::_baseAddress + address), &value, sizeof(value), NULL);
	CloseHandle(pHandle); //Close handle to prevent memory leaks
	return value;
}

template<typename T>
inline void ReplicantHook::writeMemory(uintptr_t address, T value)
{
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, ReplicantHook::_pID);
	WriteProcessMemory(pHandle, (LPVOID)(ReplicantHook::_baseAddress + address), &value, sizeof(value), NULL);
	CloseHandle(pHandle);
}

