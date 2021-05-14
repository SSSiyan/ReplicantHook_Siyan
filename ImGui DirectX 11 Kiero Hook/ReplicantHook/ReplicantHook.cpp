#include "ReplicantHook.hpp"
#include <vector>

// dev
uintptr_t ReplicantHook::_baseAddress(NULL);
DWORD ReplicantHook::_pID(NULL);
bool ReplicantHook::_hooked(NULL);
uintptr_t ReplicantHook::actorPlayable(NULL);

// values
int ReplicantHook::gold(NULL);
int ReplicantHook::XP(NULL);
std::string ReplicantHook::zone;
std::string ReplicantHook::name;
int ReplicantHook::health(NULL);
float ReplicantHook::magic(NULL);
int ReplicantHook::level(NULL);
double ReplicantHook::playtime(NULL);
float ReplicantHook::xyzpos[3]{ 0.0f, 0.0f, 0.0f };

// int ReplicantHook::charBackup(NULL);

// toggles
bool ReplicantHook::cursorForceHidden_toggle(false);
bool ReplicantHook::forceModelsVisible_toggle(false);
bool ReplicantHook::infiniteJumps_toggle(false);
bool ReplicantHook::infiniteAirCombos_toggle(false);
bool ReplicantHook::forceCharSelect_toggle(false);
int ReplicantHook::forceCharSelect_num(0);
bool ReplicantHook::spoiler_toggle(false);
bool ReplicantHook::takeNoDamage_toggle(false);
bool ReplicantHook::dealNoDamage_toggle(false);

DWORD ReplicantHook::_getProcessID(void)
{
	// search game window
	HWND hwnd = FindWindowA(NULL, "NieR Replicant ver.1.22474487139...");
	if (hwnd == NULL)
	{
		// return if game window not found
		return 0;
	}
	DWORD pID;													  //Process ID
	GetWindowThreadProcessId(hwnd, &pID);						  //Get Process ID
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); //Open process
	if (pHandle == INVALID_HANDLE_VALUE)
	{
		// return if couldn't open the process
		return 0;
	}
	return pID;
}

uintptr_t ReplicantHook::_getModuleBaseAddress(DWORD procId, const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_stricmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap); // close handle to prevent memory leaks
	return modBaseAddr;
}

// hook nier
void ReplicantHook::_hook(void)
{
	DWORD ID = ReplicantHook::_getProcessID();
	if (ID <= 0)
		return;
	ReplicantHook::_pID = ID;
	ReplicantHook::_baseAddress = ReplicantHook::_getModuleBaseAddress(ID, "NieR Replicant ver.1.22474487139.exe");
	ReplicantHook::_hooked = true;
}

// unhook nier
void ReplicantHook::_unHook(void)
{
	ReplicantHook::_hooked = false;
	ReplicantHook::_pID = 0;
	ReplicantHook::_baseAddress = 0;
	ReplicantHook::actorPlayable = 0;
	ReplicantHook::gold = 0;
	ReplicantHook::zone = "";
	ReplicantHook::name = "";
	ReplicantHook::health = 0;
	ReplicantHook::magic = 0.0f;
	ReplicantHook::level = 0;
	ReplicantHook::playtime = 0.0;
	ReplicantHook::InfiniteMagic(false);
}

void ReplicantHook::_patch(BYTE* destination, BYTE* src, unsigned int size)
{
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ReplicantHook::_pID);
	DWORD oldprotection;
	VirtualProtectEx(pHandle, destination, size, PAGE_EXECUTE_READWRITE, &oldprotection);
	WriteProcessMemory(pHandle, destination, src, size, nullptr);
	VirtualProtectEx(pHandle, destination, size, oldprotection, &oldprotection);
	CloseHandle(pHandle);
}

DWORD ReplicantHook::getProcessID(void)
{
	return ReplicantHook::_pID;
}

uintptr_t ReplicantHook::getBaseAddress(void)
{
	return ReplicantHook::_baseAddress;
}

void ReplicantHook::start(void)
{
	ReplicantHook::_hook();
}

void ReplicantHook::stop(void)
{
	ReplicantHook::_unHook();
}

void ReplicantHook::hookStatus(void)
{
	if (ReplicantHook::_pID != ReplicantHook::_getProcessID())
	{
		ReplicantHook::_unHook();
	}
}

void ReplicantHook::update()
{
	ReplicantHook::actorPlayable = readMemory <uintptr_t>(0x26F72D8);
	ReplicantHook::gold = readMemory<int>(0x437284C);
	ReplicantHook::XP = readMemory<int>(0x4372800);
	ReplicantHook::zone = readMemoryString(0x4372794);
	ReplicantHook::name = readMemoryString(0x43727BC);
	ReplicantHook::health = readMemory<int>(0x43727DC);
	ReplicantHook::magic = readMemory<float>(0x43727E8);
	ReplicantHook::level = readMemory<int>(0x43727F4);
	ReplicantHook::playtime = readMemory<double>(0x4372C30);
	if (ReplicantHook::actorPlayable != 0)
	{
		ReplicantHook::xyzpos[0] = readMemoryPointer<float>((uintptr_t)ReplicantHook::actorPlayable + 0x9C);
		ReplicantHook::xyzpos[1] = readMemoryPointer<float>((uintptr_t)ReplicantHook::actorPlayable + 0xAC);
		ReplicantHook::xyzpos[2] = readMemoryPointer<float>((uintptr_t)ReplicantHook::actorPlayable + 0xBC);
	}

	// if char select is enabled, write the char every 500ms
	if (ReplicantHook::forceCharSelect_toggle && ReplicantHook::spoiler_toggle)
	{
		ReplicantHook::forceCharSelect(ReplicantHook::forceCharSelect_num);
	}
}

bool ReplicantHook::isHooked(void)
{
	return ReplicantHook::_hooked;
}

int ReplicantHook::getGold()
{
	return ReplicantHook::gold;
}

int ReplicantHook::getXP()
{
	return ReplicantHook::XP;
}

std::string ReplicantHook::getZone()
{
	return ReplicantHook::zone;
}

std::string ReplicantHook::getName()
{
	return ReplicantHook::name;
}

int ReplicantHook::getHealth()
{
	return ReplicantHook::health;
}

float ReplicantHook::getMagic()
{
	return ReplicantHook::magic;
}

int ReplicantHook::getLevel()
{
	return ReplicantHook::level;
}

double ReplicantHook::getPlaytime()
{
	return ReplicantHook::playtime;
}

float ReplicantHook::getX()
{
	return ReplicantHook::xyzpos[0];
}

float ReplicantHook::getY()
{
	return ReplicantHook::xyzpos[1];
}

float ReplicantHook::getZ()
{
	return ReplicantHook::xyzpos[2];
}

void ReplicantHook::stealCursor(bool enabled)
{
	if (enabled)
	{
		ReplicantHook::writeMemory(0x443C1FF, true);  // show cursor
		ReplicantHook::writeMemory(0x443C1FE, false); // disable game input
		
	}
	else
	{
		ReplicantHook::writeMemory(0x443C1FF, false); // hide cursor
		ReplicantHook::writeMemory(0x443C1FE, true);  // enable game input
	}
}

void ReplicantHook::cursorForceHidden(bool enabled) // disables the game displaying the cursor when using a gamepad
{
	if (enabled)
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3D3499), (BYTE*)"\x90\x90\x90\x90\x90\x90\x90", 7);
		ReplicantHook::writeMemory(0x443C1FF, false); // hide cursor
	}
	else
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3D3499), (BYTE*)"\x40\x88\xB3\x8F\x00\x00\x00", 7);
		ReplicantHook::writeMemory(0x443C1FF, true);  // show cursor
	}
}

void ReplicantHook::setGold(int value)
{
	ReplicantHook::writeMemory(0x437284C, value);
}

void ReplicantHook::setXP(int value)
{
	ReplicantHook::writeMemory(0x4372800, value);
}

void ReplicantHook::forceModelsVisible(bool enabled)
{
	if (enabled)
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x15676A), (BYTE*)"\x90\x90\x90\x90\x90\x90\x90\x90", 8);

	}
	else
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x15676A), (BYTE*)"\xF3\x0F\x10\x8B\x80\x15\x00\x00", 8);
	}
}

void ReplicantHook::infiniteJumps(bool enabled)
{
	if (enabled)
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x69EB8A), (BYTE*)"\x90\x90", 2);

	}
	else
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x69EB8A), (BYTE*)"\x84\xC0", 2);
	}
}

void ReplicantHook::infiniteAirCombos(bool enabled)
{
	if (enabled)
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x6C13E8), (BYTE*)"\x90\x90\x90\x90\x90\x90\x90", 7);

	}
	else
	{
		ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x6C13E8), (BYTE*)"\xFF\x84\x81\x2C\x61\x01\x00", 7);
	}
}

void ReplicantHook::setZone(std::string value)
{
	ReplicantHook::writeMemoryString(0x4372794, value);
}

void ReplicantHook::setName(std::string value)
{
	ReplicantHook::writeMemoryString(0x43727BC, value);
}

void ReplicantHook::setHealth(int value)
{
	ReplicantHook::writeMemory(0x43727DC, value);
}

void ReplicantHook::setMagic(float value)
{
	ReplicantHook::writeMemory(0x43727E8, value);
}

void ReplicantHook::setLevel(int value)
{
	ReplicantHook::writeMemory(0x43727F4, value);
}

void ReplicantHook::setPlaytime(double value)
{
	ReplicantHook::writeMemory(0x4372C30, value);
}

void ReplicantHook::setX(float value)
{
	ReplicantHook::writeMemory(ReplicantHook::actorPlayable + 0x9C, value);
}

void ReplicantHook::setY(float value)
{
	ReplicantHook::writeMemory(ReplicantHook::actorPlayable + 0xAC, value);
}

void ReplicantHook::setZ(float value)
{
	ReplicantHook::writeMemory(ReplicantHook::actorPlayable + 0xBC, value);
}

void ReplicantHook::setPosition(float x, float y, float z)
{
	ReplicantHook::setX(x);
	ReplicantHook::setY(y);
	ReplicantHook::setZ(z);
}

void ReplicantHook::takeNoDamage(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x5D106DD), (BYTE*)"\x90\x90\x90\x90\x90", 5);
	else
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x5D106DD), (BYTE*)"\x44\x89\x44\x81\x4C", 5);
}

void ReplicantHook::InfiniteMagic(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3BDB5E), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3BDB5E), (BYTE*)"\xF3\x0F\x11\x54\x81\x58", 6);
}

void ReplicantHook::dealNoDamage(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x2A5CBE), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x2A5CBE), (BYTE*)"\x89\xBB\xEC\x02\x00\x00", 6);
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void ReplicantHook::forceCharSelect(int character)
{
	ReplicantHook::writeMemory(0x43727B8, character);
}

/*void ReplicantHook::getCharBackup()
{
	ReplicantHook::charBackup = ReplicantHook::readMemory<int>(0x43727B8);
}*/

void ReplicantHook::onConfigLoad(const utils::Config& cfg) {
	cursorForceHidden_toggle = cfg.get<bool>("cursorForceHidden").value_or(false);
	cursorForceHidden(cursorForceHidden_toggle);

	forceModelsVisible_toggle = cfg.get<bool>("forceModelsVisible").value_or(false);
	forceModelsVisible(forceModelsVisible_toggle);

	infiniteJumps_toggle = cfg.get<bool>("infiniteJumps").value_or(false);
	infiniteJumps(infiniteJumps_toggle);

	infiniteAirCombos_toggle = cfg.get<bool>("infiniteAirCombos").value_or(false);
	infiniteAirCombos(infiniteAirCombos_toggle);

	spoiler_toggle = cfg.get<bool>("spoiler").value_or(false);

	forceCharSelect_toggle = cfg.get<bool>("forceCharSelect").value_or(false);
	forceCharSelect(forceCharSelect_toggle);

	forceCharSelect_num = cfg.get<int>("forceCharSelectNum").value_or(0);

	takeNoDamage_toggle = cfg.get<bool>("takeNoDamage").value_or(false);
	takeNoDamage(takeNoDamage_toggle);

	dealNoDamage_toggle = cfg.get<bool>("dealNoDamage").value_or(false);
	dealNoDamage(dealNoDamage_toggle);
};

void ReplicantHook::onConfigSave(utils::Config& cfg) {
	cfg.set<bool>("cursorForceHidden", cursorForceHidden_toggle);
	cfg.set<bool>("forceModelsVisible", forceModelsVisible_toggle);
	cfg.set<bool>("infiniteJumps", infiniteJumps_toggle);
	cfg.set<bool>("spoiler", spoiler_toggle);
	cfg.set<bool>("forceCharSelect", forceCharSelect_toggle);
	cfg.set<int>("forceCharSelectNum", forceCharSelect_num);
	cfg.set<bool>("takeNoDamage", takeNoDamage_toggle);
	cfg.set<bool>("dealNoDamage", dealNoDamage_toggle);
	cfg.save("Replicant_Hook.cfg");
};
