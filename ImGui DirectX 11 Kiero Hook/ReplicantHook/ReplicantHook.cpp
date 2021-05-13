#include "ReplicantHook.hpp"

int ReplicantHook::gold(NULL);
uintptr_t ReplicantHook::_baseAddress(NULL);
DWORD ReplicantHook::_pID(NULL);
bool ReplicantHook::_hooked(NULL);

bool ReplicantHook::cursorForceHidden = false;


//uintptr_t ReplicantHook::actorPlayable(NULL);

//std::string ReplicantHook::zone(NULL);
//std::string ReplicantHook::name(NULL);

int ReplicantHook::health(NULL);
float ReplicantHook::magic(NULL);
int ReplicantHook::level(NULL);
double ReplicantHook::playtime(NULL);
float ReplicantHook::x(NULL);
float ReplicantHook::y(NULL);
float ReplicantHook::z(NULL);


DWORD ReplicantHook::_getProcessID(void)
{
	//Search game window
	HWND hwnd = FindWindowA(NULL, "NieR Replicant ver.1.22474487139...");
	if (hwnd == NULL)
	{
		//return if game window not found
		return 0;
	}
	DWORD pID;													  //Process ID
	GetWindowThreadProcessId(hwnd, &pID);						  //Get Process ID
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); //Open process
	if (pHandle == INVALID_HANDLE_VALUE)
	{
		//return if couldn't open the process
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
	CloseHandle(hSnap); //Close handle to prevent memory leaks
	return modBaseAddr;
}

//Hook to NieR
void ReplicantHook::_hook(void)
{
	DWORD ID = ReplicantHook::_getProcessID();
	if (ID <= 0)
		return;
	ReplicantHook::_pID = ID;
	ReplicantHook::_baseAddress = ReplicantHook::_getModuleBaseAddress(ID, "NieR Replicant ver.1.22474487139.exe");
	ReplicantHook::_hooked = true;
}
//unHook NieR
void ReplicantHook::_unHook(void)
{
	ReplicantHook::_hooked = false;
	ReplicantHook::_pID = 0;
	ReplicantHook::_baseAddress = 0;
	ReplicantHook::actorPlayable = 0;
	ReplicantHook::gold = 0;
	//ReplicantHook::zone = "";
	//ReplicantHook::name = "";
	ReplicantHook::health = 0;
	ReplicantHook::magic = 0.0f;
	ReplicantHook::level = 0;
	ReplicantHook::playtime = 0.0;
	ReplicantHook::x = 0;
	ReplicantHook::y = 0;
	ReplicantHook::z = 0;
	ReplicantHook::InfiniteHealth(false);
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
	ReplicantHook::actorPlayable = readMemory <uintptr_t>(0x26F72D0);
	ReplicantHook::gold = readMemory<int>(0x437284C);
	//ReplicantHook::zone = readMemoryString(0x4372794);
	//ReplicantHook::name = readMemoryString(0x43727BC);
	ReplicantHook::health = readMemory<int>(0x43727DC);
	ReplicantHook::magic = readMemory<float>(0x43727E8);
	ReplicantHook::level = readMemory<int>(0x43727F4);
	ReplicantHook::playtime = readMemory<double>(0x4372C30);
	ReplicantHook::x = readMemory<float>((uintptr_t)ReplicantHook::actorPlayable + 0x9C);
	ReplicantHook::y = readMemory<float>((uintptr_t)ReplicantHook::actorPlayable + 0xAC);
	ReplicantHook::z = readMemory<float>((uintptr_t)ReplicantHook::actorPlayable + 0xBC);
}

bool ReplicantHook::isHooked(void)
{
	return ReplicantHook::_hooked;
}

int ReplicantHook::getGold()
{
	return ReplicantHook::gold;
}
/*
std::string ReplicantHook::getZone()
{
	return ReplicantHook::zone;
}

std::string ReplicantHook::getName()
{
	return ReplicantHook::name;
}
*/
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
	return ReplicantHook::x;
}

float ReplicantHook::getY()
{
	return ReplicantHook::y;
}

float ReplicantHook::getZ()
{
	return ReplicantHook::z;
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

void ReplicantHook::hideCursor(bool enabled)
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
/*
void ReplicantHook::setZone(std::string value)
{
	ReplicantHook::writeMemoryString(0x4372794, value);
}

void ReplicantHook::setName(std::string value)
{
	ReplicantHook::writeMemoryString(0x43727BC, value);
}
*/
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

void ReplicantHook::InfiniteHealth(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x5D106DD), (BYTE*)"\x90\x90\x90\x90", 4);
	else
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x5D106DD), (BYTE*)"\x89\x44\x81\x4C", 4);
}

void ReplicantHook::InfiniteMagic(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3BDB5E), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE*)(ReplicantHook::_baseAddress + 0x3BDB5E), (BYTE*)"\xF3\x0F\x11\x54\x81\x58", 6);
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void ReplicantHook::setActorModel(std::string model)
{
	BYTE* modelBytes;
	switch (str2int(model.c_str())) {
	case str2int("nierB"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x42\x00\x00"; //nierB
		break;
	case str2int("nierT"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x54\x00\x00"; //nierT
		break;
	case str2int("nierF"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x46\x00\x00"; //nierF
		break;
	case str2int("nierY"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x59\x00\x00"; //nierY
		break;
		//case str2int("nier010"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x31\x30"; //nier010
		//	break;
		//case str2int("nier011"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x31\x31"; //nier011
		//	break;
		//case str2int("nier020"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x32\x30"; //nier020
		//	break;
		//case str2int("nier030"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x33\x30"; //nier030
		//	break;
	case str2int("kaineE"):
		modelBytes = (BYTE*)"\x6B\x61\x69\x6E\x65\x45\x00"; //kaineE
		break;
	default:
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x42\x00\x00"; //default nierB
		break;
	}
	ReplicantHook::_patch((BYTE*)(ReplicantHook::_baseAddress + 0x0B88280), modelBytes, 7);
}
/*
std::string ReplicantHook::getActorModel()
{
	return ReplicantHook::readMemoryString(0x0B88280);
}
*/