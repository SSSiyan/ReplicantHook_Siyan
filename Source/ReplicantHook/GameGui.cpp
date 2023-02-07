#include "ReplicantHook.hpp"

#include "../imgui/imgui.h"
#include <array>
#include <vector>
#include <algorithm>
#include <shlobj_core.h>
#include "LicenseStrings.hpp"
#include "Mods.hpp"

constexpr std::array<const char*, 5> characterNameStrings {
	"Young Nier",		// 0
	"Prologue Nier",    // 1
	"Old Nier",			// 2
	"Papa Nier",		// 3
	"Kaine"				// 4
};

float ReplicantHook::trainerWidth = 0.0f;
float ReplicantHook::trainerMaxHeight = 0.0f;
float ReplicantHook::trainerHeightBorder = 0.0f;
float ReplicantHook::trainerVariableHeight = 0.0f;
float ReplicantHook::sameLineWidth = 0.0f;
float ReplicantHook::inputItemWidth = 0.0f;

void HelpMarker(const char* desc) {
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ReplicantHook::gameGui(void) {
	ReplicantHook::trainerWidth = 40.0f * ImGui::GetFontSize();
	ReplicantHook::trainerMaxHeight = ImGui::GetIO().DisplaySize.y * 0.9f;
	ReplicantHook::trainerHeightBorder = ImGui::GetFontSize() * 6.0f;
	ReplicantHook::sameLineWidth = ReplicantHook::trainerWidth * 0.5f;
	ReplicantHook::inputItemWidth = ReplicantHook::trainerWidth * 0.3f;

	if (ImGui::Button("Save config")) {
		ReplicantHook::onConfigSave(ReplicantHook::cfg);
	}

	if (ImGui::BeginTabBar("Trainer", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
		// static values
		int* playerLevel		= (int*)(ReplicantHook::_baseAddress + ReplicantHook::playerLevel);
		int* playerXP			= (int*)(ReplicantHook::_baseAddress + ReplicantHook::playerXP);
		int* playerGold			= (int*)(ReplicantHook::_baseAddress + ReplicantHook::playerGold);
		int* playerHP			= (int*)(ReplicantHook::_baseAddress + ReplicantHook::playerHP);
		float* playerMP			= (float*)(ReplicantHook::_baseAddress + ReplicantHook::playerMP);
		const char* playerZone	= (const char*)(ReplicantHook::_baseAddress + ReplicantHook::playerZone);
		const char* playerName	= (const char*)(ReplicantHook::_baseAddress + ReplicantHook::playerName);

		// character values
		uintptr_t* actorPtr		= (uintptr_t*)(ReplicantHook::_baseAddress + playerActor);
		uintptr_t actorBase		= *actorPtr;
		float* playerXPos		= (float*)(actorBase + 0x9C);
		float* playerYPos		= (float*)(actorBase + 0xAC);
		float* playerZPos		= (float*)(actorBase + 0xBC);

		if (ImGui::BeginTabItem("General")) {
			ImGui::BeginChild("GeneralChild");
			if (ImGui::Checkbox("Take No Damage", &ReplicantHook::takeNoDamage_toggle)) {
				ReplicantHook::takeNoDamage(ReplicantHook::takeNoDamage_toggle);
			}
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Deal No Damage", &ReplicantHook::dealNoDamage_toggle)) {
				ReplicantHook::dealNoDamage(ReplicantHook::dealNoDamage_toggle);
			}

			if (ImGui::Checkbox("Infinite Air Combos", &ReplicantHook::infiniteAirCombos_toggle)) {
				ReplicantHook::infiniteAirCombos(ReplicantHook::infiniteAirCombos_toggle);
			}
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Infinite Jumps", &ReplicantHook::infiniteJumps_toggle)) {
				ReplicantHook::infiniteJumps(ReplicantHook::infiniteJumps_toggle);
			}

			if (ImGui::Checkbox("Infinite Magic", &ReplicantHook::infiniteMagic_toggle)){
				ReplicantHook::infiniteMagic(ReplicantHook::infiniteMagic_toggle);
			}

			Mods::GetInstance()->DrawUI();

			ImGui::Separator();

			ImGui::Checkbox("Spoiler Warning", &ReplicantHook::spoiler_toggle);
			HelpMarker("Do not tick this unless you have finished every ending.");
			if (ReplicantHook::spoiler_toggle) {
				ImGui::Checkbox("Force Character Change", &ReplicantHook::forceCharSelect_toggle);
				HelpMarker("This will apply after the next load screen");
				if (ReplicantHook::forceCharSelect_toggle) {
					ImGui::PushItemWidth(120);
					ImGui::Combo("##CharSelectDropdown", &ReplicantHook::forceCharSelect_num, characterNameStrings.data(), characterNameStrings.size());
					ImGui::PopItemWidth();
				}
			}

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Stats")) {
			ImGui::BeginChild("StatsChild");
			if (actorBase) {
				ImGui::PushItemWidth(inputItemWidth);
				ImGui::Text("Player Position:");
				ImGui::InputFloat("##PlayerPositionXInput", playerXPos);
				ImGui::SameLine();
				ImGui::InputFloat("##PlayerPositionYInput", playerYPos);
				ImGui::SameLine();
				ImGui::InputFloat("##PlayerPositionZInput", playerZPos);
				ImGui::PopItemWidth();
			}
			else {
				ImGui::Text("Game Not Loaded.");
			}
				ImGui::PushItemWidth(inputItemWidth);
				ImGui::Text("Gold:");
				ImGui::InputInt("##CharacterGoldInput", playerGold, 100, 1000);
				ImGui::Text("Level:");
				ImGui::InputInt("##CharacterLevelInput", playerLevel, 1, 10);
				ImGui::Text("XP:");
				ImGui::InputInt("##CharacterXPInput", playerXP, 100, 1000);
				ImGui::Text("HP:");
				ImGui::InputInt("##CharacterHPInput", playerHP, 10, 100);
				ImGui::Text("MP:");
				ImGui::InputFloat("##CharacterMPInput", playerMP, 10, 100);
				ImGui::PopItemWidth();
				ImGui::Text("Zone Name:");
				ImGui::SameLine();
				ImGui::Text(playerZone);
				ImGui::Text("Save Name:");
				ImGui::SameLine();
				ImGui::Text(playerName);

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Inventory")) {
			ImGui::BeginChild("InventoryChild");
			ImGui::Text("Recovery");
			static bool frameOne = true;
			for (auto& inventoryEntry : recoveryInventoryVec) {
				static int index = 1;
				uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
				int* inventoryItem = (int*)inventoryItemAddress;
				ImGui::PushItemWidth(inputItemWidth / 5);
				ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
				ImGui::PopItemWidth();
				if (index != recoveryInventoryVec.size() && index % 2 != 0)
					ImGui::SameLine(sameLineWidth);
				if (index == recoveryInventoryVec.size())
					index = 0;
				index++;
			}
			ImGui::Text("Cultivation");
			for (auto& inventoryEntry : cultivationInventoryVec) {
				static int index = 1;
				uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
				int* inventoryItem = (int*)inventoryItemAddress;
				ImGui::PushItemWidth(inputItemWidth / 5);
				ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
				ImGui::PopItemWidth();
				if (index != cultivationInventoryVec.size() && index % 2 != 0)
					ImGui::SameLine(sameLineWidth);
				if (index == cultivationInventoryVec.size())
					index = 0;
				index++;
			}
			ImGui::Text("Fishing");
			for (auto& inventoryEntry : fishingInventoryVec) {
				static int index = 1;
				uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
				int* inventoryItem = (int*)inventoryItemAddress;
				ImGui::PushItemWidth(inputItemWidth / 5);
				ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
				ImGui::PopItemWidth();
				if (index != fishingInventoryVec.size() && index % 2 != 0)
					ImGui::SameLine(sameLineWidth);
				if (index == fishingInventoryVec.size())
					index = 0;
				index++;
			}
			ImGui::Text("Materials");
			for (auto& inventoryEntry : materialsInventoryVec) {
				static int index = 1;
				uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
				int* inventoryItem = (int*)inventoryItemAddress;
				ImGui::PushItemWidth(inputItemWidth / 5);
				ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
				ImGui::PopItemWidth();
				if (index != materialsInventoryVec.size() && index % 2 != 0)
					ImGui::SameLine(sameLineWidth);
				if (index == materialsInventoryVec.size())
					index = 0;
				index++;
			}
			ImGui::Text("Key");
			for (auto& inventoryEntry : keyInventoryVec) {
				static int index = 1;
				uintptr_t inventoryItemAddress = ReplicantHook::_baseAddress + inventoryEntry.second;
				int* inventoryItem = (int*)inventoryItemAddress;
				ImGui::PushItemWidth(inputItemWidth / 5);
				ImGui::InputScalar(inventoryEntry.first.c_str(), ImGuiDataType_U8, inventoryItem);
				ImGui::PopItemWidth();
				if (index != keyInventoryVec.size() && index % 2 != 0)
					ImGui::SameLine(sameLineWidth);
				if (index == keyInventoryVec.size())
					index = 0;
				index++;
			}

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("System")) {
			ImGui::BeginChild("SystemChild");
			if (ImGui::Checkbox("Disable cursor", &ReplicantHook::cursorForceHidden_toggle)) { // toggle 
				ReplicantHook::cursorForceHidden(ReplicantHook::cursorForceHidden_toggle);
			}
			HelpMarker("Disable the cursor display while using a gamepad. This can be toggled mid play with INSERT");
			ImGui::SameLine(sameLineWidth);
			if (ImGui::Checkbox("Force Models Visibile", &ReplicantHook::forceModelsVisible_toggle)) { // toggle
				ReplicantHook::forceModelsVisible(ReplicantHook::forceModelsVisible_toggle);
			}
			HelpMarker("Stop models becoming transparent when the camera gets too close");

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Credits")) {
			ImGui::BeginChild("CreditsChild");
			ImGui::TextWrapped("WARNING: PLEASE BACK UP YOUR SAVEDATA BEFORE USING THIS HOOK.");
			ImGui::TextWrapped("I haven't had any save corruption issues, but this is a long game and "
				"I would hate for anyone to lose their saves because of me.");
			ImGui::TextWrapped("By default your save is found here:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.356f, 0.764f, 0.960f, 1.0f), "My Games");
			if (ImGui::IsItemClicked()) {
				TCHAR saveGameLocation[MAX_PATH];
				TCHAR myGames[MAX_PATH] = "My Games";
				HRESULT result = SHGetFolderPathAndSubDirA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myGames, saveGameLocation);
				ShellExecuteA(NULL, "open", saveGameLocation, NULL, NULL, SW_SHOWNORMAL);
			}

			ImGui::Separator();

			struct ImGuiURL {
				std::string text;
				std::string url;
				const ImVec4 color_hover{ 0.356f, 0.764f, 0.960f, 1.00f };
				const ImVec4 color_regular{ 0.950f, 0.960f, 0.980f, 1.00f };

				void draw() {

					ImGui::TextColored(color_regular, text.c_str());
					if (ImGui::IsItemHovered()) {
						ReplicantHook::under_line(color_hover);
					}
					if (ImGui::IsItemClicked()) {
						ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
					}
				}
			};

			ImGui::Text("Check for updates here:");
			ImGuiURL repo{ ReplicantHook::repoUrl, ReplicantHook::repoUrl };
			repo.draw();

			ImGui::Separator();

			ImGui::Text("This trainer was made by:");
			static std::array<ImGuiURL, 4> links1{
				ImGuiURL { "SSSiyan", "https://twitter.com/SSSiyan" },
				ImGuiURL { "Darkness", "https://github.com/amir-120/" },
				ImGuiURL { "Asiern", "https://github.com/Asiern/" },
				ImGuiURL { "deepdarkkapustka", "https://www.youtube.com/@mstislavcapusta7573" },
			};
			for (auto& link : links1) {
				link.draw();
			}

			ImGui::Separator();

			ImGui::Text("This trainer was made using:");
			static std::array<ImGuiURL, 5> links2{
				ImGuiURL { "Dear ImGui", "https://github.com/ocornut/imgui" },
				ImGuiURL { "RHook", "https://github.com/amir-120/RHook" },
				ImGuiURL { "Detours", "https://github.com/microsoft/Detours" },
				ImGuiURL { "libffi", "https://github.com/libffi/libffi" },
				ImGuiURL { "spdlog", "https://github.com/gabime/spdlog" },
			};
			for (auto& link : links2) {
				link.draw();
			}

			ImGui::Separator();

			ImGui::Text("Licenses:");
			struct License {
				std::string name;
				std::string text;
			};
			static std::array<License, 4> licenses{
				License{ "imgui", license::imgui },
				License{ "detours", license::detours },
				License{ "libffi", license::libffi },
				License{ "spdlog", license::spdlog },
			};
			for (const auto& license : licenses) {
				if (ImGui::CollapsingHeader(license.name.c_str())) {
					ImGui::TextWrapped(license.text.c_str());
				}
			}

			trainerVariableHeight = std::clamp(ImGui::GetCursorPosY() + trainerHeightBorder, 0.0f, trainerMaxHeight);
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
