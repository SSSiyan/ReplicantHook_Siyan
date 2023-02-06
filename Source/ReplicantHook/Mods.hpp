#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../utils/config.hpp"

#include "Mod.hpp"

class Mods
{
public:
    Mods(Mods& other) = delete;
    void operator=(const Mods&) = delete;

    static Mods* GetInstance();
	
	std::string Initialize();
	std::shared_ptr<Mod> GetMod(std::string name);
	bool IsInitialized() { return m_IsInitialized; }

	void OnFrame();
	void DrawUI();
	void Destroy();

	void SaveConfig(utils::Config& config);
	void LoadConfig(const utils::Config& config);

protected:
	Mods();
	~Mods();

	void Setup();

	std::vector<std::shared_ptr<Mod>> m_Mods{};
	std::unordered_map<std::string, std::weak_ptr<Mod>> m_NameToModMap{};
	inline static Mods* s_Instance{};
	bool m_IsInitialized{ false };
};

