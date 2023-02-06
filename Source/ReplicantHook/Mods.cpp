#include "Mods.hpp"

// Mods get included here
#include "../mods/SampleMod.hpp"

Mods* Mods::GetInstance()
{
    if (s_Instance == nullptr) {
        s_Instance = new Mods();
    }

    return s_Instance;
}

Mods::Mods()
{
    Setup();
}

Mods::~Mods()
{
    for (auto& mod : s_Mods) {
        mod->OnDestroy();
    }
}

void Mods::Setup()
{
	s_Mods.push_back(std::make_shared<SampleMod>()); // For each mod we emplace it back here

	for (auto& mod : s_Mods) {
		s_NameToModMap[mod->GetName()] = mod;
	}
}

std::optional<std::string> Mods::Initialize()
{
	for (auto& mod : s_Mods) {
        if (const auto err = mod->OnInitialize(); err.has_value()) {
            m_IsInitialized = false;
            return err;
        }
	}

    m_IsInitialized = true;
    return {};
}

std::shared_ptr<Mod> Mods::GetMod(std::string name)
{
    if (s_NameToModMap.find(name) != s_NameToModMap.end()) {
        return s_NameToModMap.at(name);
    }

    return nullptr;
}
