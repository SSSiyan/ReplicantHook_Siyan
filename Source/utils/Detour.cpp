#include "Detour.hpp"
#include <windows.h>
#include <detours.h>

namespace utility {

	Detour_t::Detour_t(uintptr_t target, uintptr_t destination)
		: m_Original((void*)target), m_Destination((void*)destination)
	{}

	Detour_t::Detour_t(void* target, void* destination)
		: m_Original(target), m_Destination(destination)
	{}

	Detour_t::~Detour_t()
	{
		Remove();
	}

	void* Detour_t::Create()
	{
		if (m_IsValid && m_IsEnabled && m_Trampoline != nullptr) {
			return m_Trampoline;
		}

		if (m_Original == nullptr || m_Destination == nullptr) {
			return nullptr;
		}

		if (auto status = DetourTransactionBegin(); status != NO_ERROR) {
			return nullptr;
		}

		if (auto status = DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
			DetourTransactionCommit();
			return nullptr;
		};

		m_Trampoline = m_Original;

		if (auto status = DetourAttach(&m_Trampoline, m_Destination); status != NO_ERROR) {
			DetourTransactionCommit();
			return nullptr;
		}

		if (auto status = DetourTransactionCommit(); status != NO_ERROR) {
			return nullptr;
		};

		m_IsEnabled = true;
		m_IsValid = true;

		return m_Trampoline;
	}

	void* Detour_t::Create(int64_t offset)
	{
		if (Create() == nullptr)
			return nullptr;

		return (void*)((uintptr_t)m_Original + offset);
	}

	bool Detour_t::Remove()
	{
		if (m_IsEnabled == false) {
			return true;
		}

		if (m_Original == nullptr || m_Destination == nullptr || m_Trampoline == nullptr) {
			return false;
		}

		if (auto status = DetourTransactionBegin(); status != NO_ERROR) {
			return false;
		}

		if (auto status = DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
			DetourTransactionCommit();
			return false;
		};

		m_Trampoline = m_Original;

		if (auto status = DetourDetach(&m_Trampoline, m_Destination); status != NO_ERROR) {
			DetourTransactionCommit();
			return false;
		}

		if (auto status = DetourTransactionCommit(); status != NO_ERROR) {
			return false;
		};
		
		m_Trampoline = nullptr;
		m_IsValid = false;
		m_IsEnabled = false;

		return true;
	}

	bool Detour_t::Toggle()
	{
		return Toggle(!m_IsEnabled);
	}

	bool Detour_t::Toggle(bool state)
	{
		if (state)
			Create();
		else
			Remove();

		return m_IsEnabled;
	}

}
