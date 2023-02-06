#pragma once
#include "../utils/Detour.hpp"
#include <vector>
#include <optional>
#include <string>

class Mod {
public:
	Mod(std::string name)
		: m_Name(name)
	{}

	~Mod() = default;

	virtual std::optional<std::string> OnInitialize() = 0;
	virtual void OnFrame() = 0;
	virtual void OnDrawUi() = 0;
	virtual void OnDestroy() = 0;
	virtual bool IsInitialized() = 0;

	virtual void OnConfigSave() = 0;
	virtual void OnConfigLoad() = 0;

	const auto& GetName() { return m_Name; }
	void SetName(std::string name) { m_Name = name; }

private:
	std::string m_Name{};
};