#include "ReplicantHook.hpp"

// toggle bools
bool ReplicantHook::cursorForceHidden_toggle(false);
bool ReplicantHook::forceModelsVisible_toggle(false);
bool ReplicantHook::infiniteJumps_toggle(false);
bool ReplicantHook::infiniteAirCombos_toggle(false);
bool ReplicantHook::forceCharSelect_toggle(false);
bool ReplicantHook::spoiler_toggle(false);
bool ReplicantHook::takeNoDamage_toggle(false);
bool ReplicantHook::dealNoDamage_toggle(false);

// values
int ReplicantHook::gold(NULL);
int ReplicantHook::XP(NULL);
const char* ReplicantHook::zone;
const char* ReplicantHook::name;
float ReplicantHook::xyzpos[3]{ 0.0f, 0.0f, 0.0f };
int ReplicantHook::forceCharSelect_num(0);

// dev values
uintptr_t ReplicantHook::_baseAddress(NULL);
DWORD ReplicantHook::_pID(NULL);
bool ReplicantHook::_hooked(false);
uintptr_t ReplicantHook::actorPlayable(NULL);

// patches
void ReplicantHook::cursorForceHidden(bool enabled) { // disables the game displaying the cursor when using a gamepad
	if (enabled) {
		ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x3D48E9), 7);
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48F)) = false; // hide cursor
	}
	else {
		ReplicantHook::_patch((char*)(ReplicantHook::_baseAddress + 0x3D48E9), (char*)"\x40\x88\xB3\x8F\x00\x00\x00", 7);
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48F)) = true; // show cursor
	}
}

void ReplicantHook::forceModelsVisible(bool enabled) {
	if (enabled)
		ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x156E6A), 8);
	else
		ReplicantHook::_patch((char*)(ReplicantHook::_baseAddress + 0x156E6A), (char*)"\xF3\x0F\x10\x8B\x80\x15\x00\x00", 8);
}

void ReplicantHook::infiniteJumps(bool enabled) {
	if (enabled)
		ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x6A007A), 2);
	else
		ReplicantHook::_patch((char*)(ReplicantHook::_baseAddress + 0x6A007A), (char*)"\x84\xC0", 2);
}

void ReplicantHook::infiniteAirCombos(bool enabled) {
	if (enabled)
		ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x6C28D8), 7);
	else
		ReplicantHook::_patch((char*)(ReplicantHook::_baseAddress + 0x6C28D8), (char*)"\xFF\x84\x81\x2C\x61\x01\x00", 7);
}

void ReplicantHook::takeNoDamage(bool enabled) {
	if (enabled)
		_nop((char*)(ReplicantHook::_baseAddress + 0x5F72DED), 5);
	else
		_patch((char*)(ReplicantHook::_baseAddress + 0x5F72DED), (char*)"\x44\x89\x44\x81\x4C", 5);
}

void ReplicantHook::InfiniteMagic(bool enabled) {
	if (enabled)
		_nop((char*)(ReplicantHook::_baseAddress + 0x3BE2BE), 6);
	else
		_patch((char*)(ReplicantHook::_baseAddress + 0x3BE2BE), (char*)"\xF3\x0F\x11\x54\x81\x58", 6);
}

void ReplicantHook::dealNoDamage(bool enabled) {
	if (enabled)
		_nop((char*)(ReplicantHook::_baseAddress + 0x2A63BE), 6);
	else
		_patch((char*)(ReplicantHook::_baseAddress + 0x2A63BE), (char*)"\x89\xBB\xEC\x02\x00\x00", 6);
}

// setters
void ReplicantHook::stealCursor(bool enabled) {
	if (enabled) {
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48F)) = true; // show cursor
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48E)) = false; // disable game input
	}
	else {
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48F)) = false; // hide cursor
		(*(bool*)(ReplicantHook::_baseAddress + 0x443E48E)) = true;  // enable game input
	}
}

void ReplicantHook::forceEndgameStats(bool enabled) {
	(*(bool*)(ReplicantHook::_baseAddress + 0x435AF40)) = true;
}

void ReplicantHook::setGold(int value) {
	(*(int*)(ReplicantHook::_baseAddress + 0x4374ADC)) = value;
}

void ReplicantHook::setXP(int value) {
	(*(int*)(ReplicantHook::_baseAddress + 0x4374A90)) = value;
}

void ReplicantHook::forceCharSelect(int character) {
	(*(int*)(ReplicantHook::_baseAddress + 0x4374A48)) = character;
}

// dev functions
DWORD ReplicantHook::_getProcessID(void) {
	// search game window
	HWND hwnd = FindWindowA(NULL, "NieR Replicant ver.1.22474487139...");
	if (hwnd == NULL) {
		// return if game window not found
		return 0;
	}
	DWORD pID;													  // process ID
	GetWindowThreadProcessId(hwnd, &pID);						  // get Process ID
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); // open process
	if (pHandle == INVALID_HANDLE_VALUE) {
		// return if couldn't open the process
		return 0;
	}
	return pID;
}

uintptr_t ReplicantHook::_getModuleBaseAddress(DWORD procId, const char* modName) {
	return (uintptr_t)GetModuleHandle(NULL);
}

void ReplicantHook::_hook(void) {
	DWORD ID = ReplicantHook::_getProcessID();
	if (ID <= 0)
		return;
	ReplicantHook::_pID = ID;
	ReplicantHook::_baseAddress = ReplicantHook::_getModuleBaseAddress(ID, "NieR Replicant ver.1.22474487139.exe");
	ReplicantHook::_hooked = true;
}

void ReplicantHook::_patch(char* dst, char* src, int size) {
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

void ReplicantHook::_nop(char* dst, unsigned int size) {
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memset(dst, 0x90, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

// called on tick
void ReplicantHook::update() {
	ReplicantHook::actorPlayable = (*(uintptr_t*)(ReplicantHook::_baseAddress + 0x26F9560));
	ReplicantHook::gold = (*(int*)(ReplicantHook::_baseAddress + 0x4374ADC));
	ReplicantHook::XP = (*(int*)(ReplicantHook::_baseAddress + 0x4374A90));
	ReplicantHook::zone = (const char*)_baseAddress + 0x4374A24;
	ReplicantHook::name = (const char*)_baseAddress + 0x4374A4C;

	// show 0 rather than junk values on boot
	if (ReplicantHook::actorPlayable != 0) {
		ReplicantHook::xyzpos[0] = (*(float*)(ReplicantHook::actorPlayable + 0x9C));
		ReplicantHook::xyzpos[1] = (*(float*)(ReplicantHook::actorPlayable + 0xAC));
		ReplicantHook::xyzpos[2] = (*(float*)(ReplicantHook::actorPlayable + 0xBC));
	}

	// if char select is enabled, write the char
	if (ReplicantHook::forceCharSelect_toggle && ReplicantHook::spoiler_toggle) {
		ReplicantHook::forceCharSelect(ReplicantHook::forceCharSelect_num);

		// if character is 4, force old save stats
		if (ReplicantHook::forceCharSelect_num == 4) {
			ReplicantHook::forceEndgameStats(true);
		}
	}
}

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
