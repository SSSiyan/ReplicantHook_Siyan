#pragma once
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Mod.hpp"

class Mods
{
public:
    Mods(Mods& other) = delete;
    void operator=(const Mods&) = delete;

    static Mods* GetInstance();
	
	static std::optional<std::string> Initialize();
	static std::shared_ptr<Mod> GetMod(std::string name);
	static bool IsInitialized() { return m_IsInitialized; }

protected:
	Mods();
	~Mods();

	void Setup();

	inline static std::vector<std::shared_ptr<Mod>> s_Mods{};
	inline static std::unordered_map<std::string, std::shared_ptr<Mod>> s_NameToModMap{};
	inline static Mods* s_Instance{};
	inline static bool m_IsInitialized{ false };
};

