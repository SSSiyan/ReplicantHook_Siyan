#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

class ReplicantHook
{
private:
	// values
	static DWORD _pID;
	static uintptr_t _baseAddress;
	uintptr_t actorPlayable;
	static bool _hooked;

	std::string zone;
	std::string name;
	int health;
	float magic;
	int level;
	double playtime;
	float x;
	float y;
	float z;

	DWORD _getProcessID(void);
	uintptr_t _getModuleBaseAddress(DWORD procId, const char* modName);
	void _hook(void);
	void _unHook(void);
	static void _patch(BYTE* destination, BYTE* src, unsigned int size);
	template <typename T>
	T readMemory(uintptr_t address);
	template <typename T>
	static void writeMemory(uintptr_t address, T value);
	std::string readMemoryString(uintptr_t address);
	void writeMemoryString(uintptr_t address, std::string value);

public:
	DWORD getProcessID(void);
	static uintptr_t getBaseAddress(void);
	void start(void);
	void stop(void);
	void hookStatus(void);
	void update();

	//Getters
	bool isHooked(void); // main.cpp startup check
	int getGold();
	std::string getZone();
	std::string getName();
	int getHealth();
	float getMagic();
	int getLevel();
	double getPlaytime();
	float getX();
	float getY();
	float getZ();

	//Setters
	static void stealCursor(bool toggle);
	static void setGold(int value);
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

	//Cheats
	void InfiniteHealth(bool enabled);
	void InfiniteMagic(bool enabled);

	//Models
	void setActorModel(std::string model);
	std::string getActorModel();

	//Added
	static int gold;
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

