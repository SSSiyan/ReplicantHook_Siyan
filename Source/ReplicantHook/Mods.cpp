#include "Mods.hpp"

// Mods get included here
#include "../mods/SampleMod.hpp"

Mods* Mods::GetInstance()
{
	if (s_Instance == nullptr)
		s_Instance = new Mods();

	return s_Instance;
}

Mods::Mods()
{
    Setup();
}

Mods::~Mods()
{
	Destroy();
}

void Mods::Setup()
{
	m_Mods.push_back(std::make_shared<SampleMod>()); // For each mod we emplace it back here

	for (auto& mod : m_Mods)
		m_NameToModMap[mod->GetName()] = mod;
}

std::string Mods::Initialize()
{
	for (auto& mod : m_Mods) {
        if (const auto err = mod->OnInitialize(); !err.empty()) {
            m_IsInitialized = false;
            return err;
        }
	}

    m_IsInitialized = true;
    return {};
}

std::shared_ptr<Mod> Mods::GetMod(std::string name)
{
	if (m_NameToModMap.find(name) != m_NameToModMap.end())
		return m_NameToModMap.at(name).lock();

	return {};
}

void Mods::OnFrame()
{
	for (auto& mod : m_Mods)
		mod->OnFrame();
}

void Mods::DrawUI()
{
	for (auto& mod : m_Mods)
		mod->OnDrawUI();
}

void Mods::Destroy()
{
	if (!m_IsInitialized)
		return;

	for (auto& mod : m_Mods)
		mod->OnDestroy();

	m_IsInitialized = false;
}

void Mods::SaveConfig(utils::Config& config)
{
	for (auto& mod : m_Mods)
		mod->OnConfigSave(config);
}

void Mods::LoadConfig(const utils::Config& config)
{
	for (auto& mod : m_Mods)
		mod->OnConfigLoad(config);
}
