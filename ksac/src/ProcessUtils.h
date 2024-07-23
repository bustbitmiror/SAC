#pragma once

#include "pch.h"
#include "MemoryHelper.h"

extern "C" {
	#include "WindowsTypes.h"
}
#include "SACCommon.h"


// Definitions.
constexpr SIZE_T MAX_PIDS = 256;
constexpr SIZE_T MAX_TIDS = 256;
constexpr SIZE_T SYSTEM_PROCESS_PID = 0x4;
constexpr SIZE_T PROCESS_TERMINATE = 0x1;
constexpr SIZE_T PROCESS_CREATE_THREAD = 0x2;
constexpr SIZE_T PROCESS_VM_READ = 0x10;
constexpr SIZE_T PROCESS_VM_OPERATION = 0x8;


#define VALID_PROCESS(Pid)(Pid > 0 && Pid != SYSTEM_PROCESS_PID)


struct ProtectedProcessesList {
	FastMutex Lock;
	ULONG LastIndex;
	ULONG PidsCount;
	ULONG Processes[MAX_PIDS];
};

struct ProtectedProcess {
	ULONG Pid;
	bool Protect;
};




class ProcessUtils {

public:

	void* operator new(size_t size) {
		return AllocateMemory(size, false);
	}

	void operator delete(void* p) {
		if (p)
			ExFreePoolWithTag(p, DRIVER_TAG);
	}


	ProcessUtils();
	~ProcessUtils();

	void ClearProtectedProcesses();
	bool FindProcess(ULONG pid);
	bool AddProcess(ULONG pid);
	bool RemoveProcess(ULONG pid);

	ULONG GetProtectedProcessesCount() { return this->ProtectedProcesses.PidsCount; }

private:
	ProtectedProcessesList ProtectedProcesses;

};

inline ProcessUtils* SACProcessUtils;


OB_PREOP_CALLBACK_STATUS OnPreOpenProcess(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION Info);
