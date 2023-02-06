#pragma once
#include "../utils/config.hpp"
#include <vector>
#include <string>

class Mod {
public:
	Mod(std::string name)
		: m_Name(name)
	{}

	~Mod() = default;

	virtual std::string OnInitialize() = 0;
	virtual void OnFrame() = 0;
	virtual void OnDrawUI() = 0;
	virtual void OnDestroy() = 0;
	virtual bool IsInitialized() = 0;

	virtual void OnConfigSave(utils::Config& config) = 0;
	virtual void OnConfigLoad(const utils::Config& config) = 0;

	const auto& GetName() { return m_Name; }
	void SetName(std::string name) { m_Name = name; }

private:
	std::string m_Name{};
};