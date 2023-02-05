#include "ReplicantHook.hpp"

bool ReplicantHook::sampleMod1Init = false;

// patch toggle bools
bool ReplicantHook::cursorForceHidden_toggle = false;
bool ReplicantHook::forceModelsVisible_toggle = false;
bool ReplicantHook::infiniteJumps_toggle = false;
bool ReplicantHook::infiniteAirCombos_toggle = false;
bool ReplicantHook::forceCharSelect_toggle = false;
bool ReplicantHook::spoiler_toggle = false;
bool ReplicantHook::takeNoDamage_toggle = false;
bool ReplicantHook::dealNoDamage_toggle = false;
bool ReplicantHook::infiniteMagic_toggle = false;
int ReplicantHook::forceCharSelect_num = 0;

// gui
uintptr_t ReplicantHook::playerActor = 0x26F9560;
uintptr_t ReplicantHook::playerLevel = 0x4374A84;
uintptr_t ReplicantHook::playerXP = 0x4374A90;
uintptr_t ReplicantHook::playerGold = 0x4374ADC;
uintptr_t ReplicantHook::playerHP = 0x4374A6C;
uintptr_t ReplicantHook::playerMP = 0x4374A78;
uintptr_t ReplicantHook::playerZone = 0x4374A24;
uintptr_t ReplicantHook::playerName = 0x4374A4C;

// system
uintptr_t ReplicantHook::enableInput = 0x443E48E;
uintptr_t ReplicantHook::showCursor = 0x443E48F; // 1 byte later
uintptr_t ReplicantHook::currentCharacter = 0x4374A48;
uintptr_t ReplicantHook::playerEndgame = 0x435AF40;

// inventory
std::vector<std::pair<std::string, uintptr_t>> ReplicantHook::recoveryInventoryVec;
std::vector<std::pair<std::string, uintptr_t>> ReplicantHook::cultivationInventoryVec;
std::vector<std::pair<std::string, uintptr_t>> ReplicantHook::fishingInventoryVec;
std::vector<std::pair<std::string, uintptr_t>> ReplicantHook::materialsInventoryVec;
std::vector<std::pair<std::string, uintptr_t>> ReplicantHook::keyInventoryVec;

// dev values
uintptr_t ReplicantHook::_baseAddress(NULL);
DWORD ReplicantHook::_pID(NULL);
bool ReplicantHook::_hooked(false);

// patches
void ReplicantHook::cursorForceHidden(bool enabled) { // disables the game displaying the cursor when using a gamepad
	if (enabled) {
		ReplicantHook::_nop((char*)(ReplicantHook::_baseAddress + 0x3D48E9), 7);
		*(bool*)(ReplicantHook::_baseAddress + showCursor) = false; // hide cursor
	}
	else {
		ReplicantHook::_patch((char*)(ReplicantHook::_baseAddress + 0x3D48E9), (char*)"\x40\x88\xB3\x8F\x00\x00\x00", 7);
		*(bool*)(ReplicantHook::_baseAddress + showCursor) = true; // show cursor
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
		_nop((char*)(ReplicantHook::_baseAddress + 0x3BE25D), 5);
	else
		_patch((char*)(ReplicantHook::_baseAddress + 0x3BE25D), (char*)"\x44\x89\x44\x81\x4C", 5);
}

void ReplicantHook::dealNoDamage(bool enabled) {
	if (enabled)
		_nop((char*)(ReplicantHook::_baseAddress + 0x2A63BE), 6);
	else
		_patch((char*)(ReplicantHook::_baseAddress + 0x2A63BE), (char*)"\x89\xBB\xEC\x02\x00\x00", 6);
}

void ReplicantHook::infiniteMagic(bool enabled) {
	if (enabled)
		_nop((char*)ReplicantHook::_baseAddress + 0x3BE2BE, 6);
	else
		_patch((char*)ReplicantHook::_baseAddress + 0x3BE2BE, (char*)"\xF3\x0F\x11\x54\x81\x58", 6);
}

// static values
void ReplicantHook::stealCursor(bool enabled) {
	if (enabled) {
		*(bool*)(ReplicantHook::_baseAddress + enableInput) = false; // disable game input
		*(bool*)(ReplicantHook::_baseAddress + showCursor) = true; // show cursor
	}
	else {
		*(bool*)(ReplicantHook::_baseAddress + enableInput) = true;  // enable game input
		*(bool*)(ReplicantHook::_baseAddress + showCursor) = false; // hide cursor
	}
}

void ReplicantHook::forceEndgameStats(bool enabled) {
	*(bool*)(ReplicantHook::_baseAddress + playerEndgame) = true; // playerEndgame in main.cpp
}

void ReplicantHook::forceCharSelect(int character) {
	*(int*)(ReplicantHook::_baseAddress + currentCharacter) = character;
}

// dev functions
void ReplicantHook::getInventoryAddresses(void) { // called in InitHook
	// Recovery
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Medicinal Herb", 0x4374AE0);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Health Salve", 0x4374AE1);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Recovery Potion", 0x4374AE2);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Strength Drop", 0x4374AF5);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Strength Capsule", 0x4374AF6);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Magic Drop", 0x4374AF7);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Magic Capsule", 0x4374AF8);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Defense Drop", 0x4374AF9);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Defense Capsule", 0x4374AFA);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Spirit Drop", 0x4374AFB);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Spirit Capsule", 0x4374AFC);
	recoveryInventoryVec.emplace_back<std::string, uintptr_t>("Antidotal Weed", 0x4374AFF);
	// Cultivation
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Speed Fertilizer", 0x4374B09);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Flowering Fertilizer", 0x4374B0A);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bounty Fertilizer", 0x4374B0B);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pumpkin Seed", 0x4374B0E);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Watermelon Seed", 0x4374B0F);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Melon Seed", 0x4374B10);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gourd Seed", 0x4374B11);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tomato Seed", 0x4374B12);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Eggplant Seed", 0x4374B13);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bell Pepper Seed", 0x4374B14);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Been Seed", 0x4374B15);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Wheat Seed", 0x4374B16);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Rice Plant Seed", 0x4374B17);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Dahlia Seed", 0x4374B18);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tulip Seed", 0x4374B19);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Freesia Bulb", 0x4374B1A);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Red Moonflower Seed", 0x4374B1B);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gold Moonlfower Seed", 0x4374B1C);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Peach Moonflower Seed", 0x4374B1D);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pink Moonflower Seed", 0x4374B1E);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Blue Moonflower Seed", 0x4374B1F);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Indigo Moonflower Seed", 0x4374B20);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("White Moonflower Seed", 0x4374B21);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pumpkin", 0x4374B27);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Watermelon", 0x4374B28);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Melon", 0x4374B29);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gourd", 0x4374B2A);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tomato", 0x4374B2B);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Eggplant", 0x4374B2C);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Bell Pepper", 0x4374B2D);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Beans", 0x4374B2E);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Wheat", 0x4374B2F);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Rice", 0x4374B30);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Dahlia", 0x4374B31);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Tulip", 0x4374B32);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Freesia", 0x4374B33);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Red Moonflower", 0x4374B34);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Gold Moonlfower", 0x4374B35);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Peach Moonflower", 0x4374B36);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Pink Moonflower", 0x4374B37);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Blue Moonflower", 0x4374B38);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("Indigo Moonflower", 0x4374B39);
	cultivationInventoryVec.emplace_back<std::string, uintptr_t>("White Moonflower", 0x4374B3A);
	// Fishing
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Lugworm", 0x4374B45);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Earthworm", 0x4374B46);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Lure", 0x4374B47);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Sardine", 0x4374B4F);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Carp", 0x4374B50);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Blowfish", 0x4374B51);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Bream", 0x4374B52);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Shark", 0x4374B53);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Blue Marlin", 0x4374B54);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Dunkleosteus", 0x4374B55);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Rainbow Trout", 0x4374B56);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Black Bass", 0x4374B57);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Giant Catfish", 0x4374B58);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Royal Fish", 0x4374B59);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Hyneria", 0x4374B5A);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Sandfish", 0x4374B5B);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Rhizodont", 0x4374B5C);
	fishingInventoryVec.emplace_back<std::string, uintptr_t>("Shaman Fish", 0x4374B5D);
	// Materials
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Aquatic Plant", 0x4374B63);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Deadwood", 0x4374B64);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusty Bucket", 0x4374B65);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Empty Can", 0x4374B66);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Gold Ore", 0x4374B6A);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Silvert Ore", 0x4374B6B);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Copper Ore", 0x4374B6C);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Iron Ore", 0x4374B6D);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Crystal", 0x4374B6E);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pyrite", 0x4374B6F);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Moldavite", 0x4374B70);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Meteorite", 0x4374B71);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Amber", 0x4374B72);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Fluorite", 0x4374B73);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Clay", 0x4374B74);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Berries", 0x4374B79);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Royal Fern", 0x4374B7A);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Tree Branch", 0x4374B7B);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Log", 0x4374B7C);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Natural Rubber", 0x4374B7D);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Ivy", 0x4374B7E);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Lichen", 0x4374B7F);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mushroom", 0x4374B80);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Sap", 0x4374B81);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mutton", 0x4374B87);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Meat", 0x4374B88);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wool", 0x4374B89);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Hide", 0x4374B8A);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wolf Hide", 0x4374B8B);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Wolf Fang", 0x4374B8C);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Giant Spider Silk", 0x4374B8D);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bat Fang", 0x4374B8E);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bat Wing", 0x4374B8F);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Goat Meat", 0x4374B90);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Goat Hide", 0x4374B91);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Venison", 0x4374B92);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rainbow Spider Silk", 0x4374B93);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Boar Liver", 0x4374B94);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Scorpion Claw", 0x4374B95);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Scorpion Tail", 0x4374B96);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dented Metal Board", 0x4374B97);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Stripped Bolt ", 0x4374B98);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Lens", 0x4374B99);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Severed Cable", 0x4374B9A);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Arm", 0x4374B9B);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Antena", 0x4374B9C);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Motor", 0x4374B9D);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Battery", 0x4374B9E);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mysterious Switch", 0x4374B9F);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Large Gear", 0x4374BA0);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Titanium Alloy", 0x4374BA1);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Memory Alloy", 0x4374BA2);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusted Clump", 0x4374BA3);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Machine Oil", 0x4374BA4);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Forlorn Necklace", 0x4374BA9);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Twisted Ring", 0x4374BAA);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Earring", 0x4374BAB);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pretty Choker", 0x4374BAC);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Metal Piercing", 0x4374BAD);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Subdued Bracelet", 0x4374BAE);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Technical Guide", 0x4374BAF);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Grubby Book", 0x4374BB0);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Thick Dictionary", 0x4374BB1);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Closed Book", 0x4374BB2);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Used Coloring Book", 0x4374BB3);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Old Schoolbook", 0x4374BB4);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dirty Bag", 0x4374BB5);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Flashyh Hat", 0x4374BB6);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Leather Gloves", 0x4374BB7);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Silk Handkerchief", 0x4374BB8);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Leather Boots", 0x4374BB9);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Complex Machine", 0x4374BBA);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Elaborate Machine", 0x4374BBB);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Simple Machine", 0x4374BBC);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Stopped Clock", 0x4374BBD);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Wristwatch", 0x4374BBE);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Rusty Kitchen Knife", 0x4374BBF);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Saw", 0x4374BC0);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Dented Metal Bat", 0x4374BC1);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Shell", 0x4374BC3);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Gastropod", 0x4374BC4);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Bivalve", 0x4374BC5);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Seaweed", 0x4374BC6);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Empty Bottle", 0x4374BC7);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Driftwood", 0x4374BC8);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Pearl", 0x4374BC9);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Black Pearl", 0x4374BCA);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Crab", 0x4374BCB);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Starfish", 0x4374BCC);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Sea Turtle Egg", 0x4374BD2);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Broken Pottery", 0x4374BD3);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Desert Rose", 0x4374BD4);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Giant Egg", 0x4374BD5);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Damascus Steel", 0x4374BD6);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Eagle Egg", 0x4374BD7);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Chicken Egg", 0x4374BD8);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Mouse Tail", 0x4374BDA);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Lizard Tail", 0x4374BDB);
	materialsInventoryVec.emplace_back<std::string, uintptr_t>("Deer Antler", 0x4374BDF);
	// Key
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Moon Key", 0x4374BE0);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Star Key", 0x4374BE1);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Light Key", 0x4374BE2);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Darkness Key", 0x4374BE3);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Fine Flour", 0x4374BE4);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Coarse FLour", 0x4374BE5);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Perfume Bottle", 0x4374BE6);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Postman's Parcel", 0x4374BE7);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Lover's Letter", 0x4374BE8);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Water Filter", 0x4374BE9);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Royal Compass", 0x4374BEA);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Vapor Moss", 0x4374BEB);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Valley Spoder Silk", 0x4374BEC);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Animal Guidebook", 0x4374BED);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Ore Guidebook", 0x4374BEE);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Plant Guidebook", 0x4374BEF);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Book", 0x4374BF0);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Blue Book", 0x4374BF1);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Lady's Elixer", 0x4374BF2);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Lady's Elixer+", 0x4374BF3);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for The Aerie", 0x4374BF4);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for Seafront", 0x4374BF5);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Cookbook", 0x4374BF6);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Parcel for Facade", 0x4374BF7);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Max's Herbs", 0x4374BF8);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo", 0x4374BF9);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 2", 0x4374BFA);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 3", 0x4374BFB);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Drifting Cargo 4", 0x4374BFC);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Package", 0x4374BFD);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Mermaid Tear", 0x4374BFE);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Mandrake Leaf", 0x4374BFF);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Energizer", 0x4374C00);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Tad Oil", 0x4374C01);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Sleep-B-Gone", 0x4374C02);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Antidote", 0x4374C03);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Gold Bracelet", 0x4374C04);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Elite Kitchen Knife", 0x4374C05);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Elevator Parts", 0x4374C06);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Dirty Treasure Map", 0x4374C07);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Restored Treasure Map", 0x4374C08);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Jade Hair Ornament", 0x4374C09);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Employee List", 0x4374C0A);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Small Safe", 0x4374C0B);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Safe Key", 0x4374C0C);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Greaty Tree Root", 0x4374C0D);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Eye of Power", 0x4374C0E);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Ribbon", 0x4374C0F);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Yonah's Ribbon", 0x4374C10);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Bronze Key", 0x4374C11);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Brass Key", 0x4374C12);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Boar Tusk", 0x4374C13);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Pressed Freesia", 0x4374C14);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Potted Freesia", 0x4374C15);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Fressia (Delivery)", 0x4374C16);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Pile of Junk", 0x4374C17);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Old Gold Coin", 0x4374C18);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Market Map", 0x4374C19);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("AA Keycard", 0x4374C1A);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("KA Keycard", 0x4374C1B);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("SA Keycard", 0x4374C1C);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("TA Keycard", 0x4374C1D);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("NA Keycard", 0x4374C1E);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("HA Keycard", 0x4374C1F);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("MA Keycard", 0x4374C20);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("YA Keycard", 0x4374C21);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("RA Keycard", 0x4374C22);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("WA Keycard", 0x4374C23);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Cultivator's Handbook", 0x4374C24);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Bag", 0x4374C25);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Lantern", 0x4374C26);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Empty Lantern", 0x4374C27);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Hold Key ", 0x4374C28);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Passageway Key", 0x4374C29);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Goat Key", 0x4374C2A);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Lizard Key", 0x4374C2B);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Unlocking Procedure Memo ", 0x4374C2C);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Jewel", 0x4374C2D);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Red Flowers ", 0x4374C2E);
	keyInventoryVec.emplace_back<std::string, uintptr_t>("Apples", 0x4374C2F);
}

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

void ReplicantHook::under_line(const ImColor& col) {
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;
	ImGui::GetWindowDrawList()->AddLine(min, max, col, 1.0f);
}

void ReplicantHook::onConfigLoad(const utils::Config& cfg) {
	cursorForceHidden_toggle = cfg.get<bool>("cursorForceHiddenToggle").value_or(false);
	cursorForceHidden(cursorForceHidden_toggle);

	forceModelsVisible_toggle = cfg.get<bool>("forceModelsVisibleToggle").value_or(false);
	forceModelsVisible(forceModelsVisible_toggle);

	infiniteJumps_toggle = cfg.get<bool>("infiniteJumpsToggle").value_or(false);
	infiniteJumps(infiniteJumps_toggle);

	infiniteAirCombos_toggle = cfg.get<bool>("infiniteAirCombosToggle").value_or(false);
	infiniteAirCombos(infiniteAirCombos_toggle);

	spoiler_toggle = cfg.get<bool>("spoilerToggle").value_or(false);
	forceCharSelect_toggle = cfg.get<bool>("forceCharSelectToggle").value_or(false);
	forceCharSelect(forceCharSelect_toggle);
	forceCharSelect_num = cfg.get<int>("forceCharSelectNumValue").value_or(0);

	takeNoDamage_toggle = cfg.get<bool>("takeNoDamageToggle").value_or(false);
	takeNoDamage(takeNoDamage_toggle);

	dealNoDamage_toggle = cfg.get<bool>("dealNoDamageToggle").value_or(false);
	dealNoDamage(dealNoDamage_toggle);

	infiniteMagic_toggle = cfg.get<bool>("infiniteMagicToggle").value_or(false);
	infiniteMagic(infiniteMagic_toggle);
}

void ReplicantHook::onConfigSave(utils::Config& cfg) {
	cfg.set<bool>("cursorForceHiddenToggle", cursorForceHidden_toggle);
	cfg.set<bool>("forceModelsVisibleToggle", forceModelsVisible_toggle);
	cfg.set<bool>("infiniteJumpsToggle", infiniteJumps_toggle);
	cfg.set<bool>("infiniteAirCombosToggle", infiniteAirCombos_toggle);
	cfg.set<bool>("spoilerToggle", spoiler_toggle);
	cfg.set<bool>("forceCharSelectToggle", forceCharSelect_toggle);
	cfg.set<int>("forceCharSelectNumValue", forceCharSelect_num);
	cfg.set<bool>("takeNoDamageToggle", takeNoDamage_toggle);
	cfg.set<bool>("dealNoDamageToggle", dealNoDamage_toggle);
	cfg.set<bool>("infiniteMagicToggle", infiniteMagic_toggle);

	cfg.save(cfgString);
}
