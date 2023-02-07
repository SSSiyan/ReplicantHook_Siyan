#pragma once
#include <vadefs.h>
#include <stdint.h>

namespace utility {
	class Detour_t {
	public:
		Detour_t(uintptr_t target, uintptr_t destination);
		Detour_t(uintptr_t target, uintptr_t destination, uintptr_t* pOriginalCodeReturn);
		Detour_t(uintptr_t target, uintptr_t destination, uintptr_t* ppReturn, int64_t offsetFromDetour);

		Detour_t(void* target, void* destination);
		Detour_t(void* target, void* destination, void** pOriginalCodeReturn);
		Detour_t(void* target, void* destination, void** pReturn, int64_t offsetFromDetour);

		~Detour_t();

		/*
		* Returns the address of where the detour should be placed
		*/
		const auto& GetOriginal() const { return m_Original; }

		/*
		* Returns the address of the code the detour wouldl jump to
		*/
		const auto& GetDestination() const { return m_Destination; }

		/*
		* Returns the address of a copy of the original code that has been replaced by the detour jump
		*/
		const auto& GetTrampoline() const { return m_Trampoline; }

		/*
		* Returns the return address by offset from the detour
		*/
		void* GetReturnAddress(int64_t offset) { return (void*)((uintptr_t)m_Original + offset); };

		/*
		* Returns a bool that indicates the state of the detour whether if the target and destination are set (true) or not (false)
		*/
		auto IsValid() const { return m_Original != nullptr && m_Destination != nullptr; }

		/*
		* Returns a bool that indicates the state of the detour whether if the detour is in place (true) or not (false)
		*/
		auto IsEnabled() const { return m_Trampoline != nullptr; }

		/*
		* Places the detour in place and returns the address to the original code (Trampoline)
		*/
		void* Create();

		/*
		* Places the detour in place and returns the aob address offset by the value passed to it (In bytes)
		*/
		void* Create(int64_t offset);
		
		/*
		* Checks if the detour is valid automatically internally and returns a bool
		* which indicates whether the remove operation was successful or not, if the detour
		* was never created using the Create() method, the return value would be true
		*/
		bool Remove();

		/*
		* Calls Create() if the detour is not in place (Is disabled) or
		* calls Remove() if the detour is already in place (Is enabled)
		* returns the current state of the hook after enabling (true) or disabling (false) it
		*/
		bool Toggle();

		/*
		* Takes a bool and enables (true) or disables (false) the detour based on that
		* returns the current state of the hook after enabling (true) or disabling (false) it
		*/
		bool Toggle(bool state);

	private:
		void* m_Original{ nullptr };
		void* m_Destination{ nullptr };
		void* m_Trampoline{ nullptr };
	};
}
