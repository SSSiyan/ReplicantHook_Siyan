#pragma once
#include <optional>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "Mod.hpp"

class Mods
{
public:
    Mods(Mods& other) = delete;
    void operator=(const Mods&) = delete;

    static std::unique_ptr<Mods>& GetInstance();
	
	static std::optional<std::string> Initialize();
	static Mod* GetMod(std::string name);
	static bool IsInitialized() { return m_IsInitialized; }

private:
	Mods();
	~Mods();

	void Setup();

	inline static std::vector<Mod> s_Mods{};
	inline static std::unordered_map<std::string, Mod*> s_NameToModMap{};
	inline static std::unique_ptr<Mods> s_Instance{};
	inline static bool m_IsInitialized{ false };
};

