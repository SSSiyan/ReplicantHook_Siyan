#include "Detour.hpp"
#include <windows.h>
#include <detours.h>

namespace utility {

	Detour_t::Detour_t(uintptr_t target, uintptr_t destination)
		: m_Original((void*)target), m_Destination((void*)destination)
	{}

	Detour_t::Detour_t(uintptr_t target, uintptr_t destination, uintptr_t* pOriginalCodeReturn)
		: m_Original((void*)target), m_Destination((void*)destination)
	{
		*pOriginalCodeReturn = (uintptr_t)Create();
	}

	Detour_t::Detour_t(uintptr_t target, uintptr_t destination, uintptr_t* pReturn, int64_t offsetFromDetour)
		: m_Original((void*)target), m_Destination((void*)destination)
	{
		*pReturn = (uintptr_t)Create(offsetFromDetour);
	}

	Detour_t::Detour_t(void* target, void* destination)
		: m_Original(target), m_Destination(destination)
	{}

	Detour_t::Detour_t(void* target, void* destination, void** pOriginalCodeReturn)
		: m_Original(target), m_Destination(destination)
	{
		*pOriginalCodeReturn = Create();
	}

	Detour_t::Detour_t(void* target, void* destination, void** pReturn, int64_t offsetFromDetour)
		: m_Original(target), m_Destination(destination)
	{
		*pReturn = Create(offsetFromDetour);
	}

	Detour_t::~Detour_t()
	{
		Remove();
	}

	void* Detour_t::Create()
	{
		if (m_Trampoline != nullptr) {
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

		if (auto status = DetourDetach(&m_Trampoline, m_Destination); status != NO_ERROR) {
			DetourTransactionCommit();
			return false;
		}

		if (auto status = DetourTransactionCommit(); status != NO_ERROR) {
			return false;
		};
		
		m_Trampoline = nullptr;

		return true;
	}

	bool Detour_t::Toggle()
	{
		bool isEnabled = m_Trampoline != nullptr;
		return Toggle(!isEnabled);
	}

	bool Detour_t::Toggle(bool state)
	{
		if (state)
			Create();
		else
			Remove();

		return m_Trampoline != nullptr;
	}

}
