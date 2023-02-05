#pragma once
#include <vadefs.h>

namespace utility {
	class Detour_t {
	public:
		Detour_t(uintptr_t target, uintptr_t destination);
		Detour_t(void* target, void* destination);
		~Detour_t();

		const auto& GetOriginal()		const { return m_Original; }
		const auto& GetDestination()	const { return m_Destination; }
		const auto& GetTrampoline()		const { return m_Trampoline; }
		
		const auto& IsEnabled()			const { return m_IsEnabled; }
		const auto& IsValid()			const { return m_IsValid; }

		/*
		* Returns the address to the original code (Trampoline)
		*/
		void*		Create();
		/*
		* Returns the aob address address offset by the value passed to it
		*/
		void*		Create(int64_t offset);
		bool		Remove();

		bool		Toggle();
		bool		Toggle(bool state);

	private:
		void* m_Original{ nullptr };
		void* m_Destination{ nullptr };
		void* m_Trampoline{ nullptr };

		bool m_IsEnabled{ false };
		bool m_IsValid{ false };
	};
}
