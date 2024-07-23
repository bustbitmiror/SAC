#include "ProcessUtils.h"



OB_PREOP_CALLBACK_STATUS OnPreOpenProcess(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION Info) {
	UNREFERENCED_PARAMETER(RegistrationContext);

	if (Info->KernelHandle)
		return OB_PREOP_SUCCESS;

	if (SACProcessUtils->GetProtectedProcessesCount() == 0)
		return OB_PREOP_SUCCESS;

	auto Process = (PEPROCESS)Info->Object;
	auto pid = HandleToULong(PsGetProcessId(Process));

	// If the process was found on the list, remove permissions for dump / write process memory and kill the process.
	if (SACProcessUtils->FindProcess(pid)) {
		Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
		Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
		Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_CREATE_THREAD;
		Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_DUP_HANDLE;
		Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
	}

	return OB_PREOP_SUCCESS;
}


ProcessUtils::ProcessUtils()
{
	this->ProtectedProcesses.PidsCount = 0;
	this->ProtectedProcesses.LastIndex = 0;
	memset(&this->ProtectedProcesses.Processes, 0, sizeof(this->ProtectedProcesses.Processes));
	this->ProtectedProcesses.Lock.Init();
}


ProcessUtils::~ProcessUtils()
{
	ClearProtectedProcesses();
}

void ProcessUtils::ClearProtectedProcesses()
{
	AutoLock locker(this->ProtectedProcesses.Lock);

	memset(&this->ProtectedProcesses.Processes, 0, sizeof(this->ProtectedProcesses.Processes));
	this->ProtectedProcesses.PidsCount = 0;
	this->ProtectedProcesses.LastIndex = 0;
}

bool ProcessUtils::FindProcess(ULONG pid)
{
	AutoLock locker(this->ProtectedProcesses.Lock);

	for (ULONG i = 0; i <= this->ProtectedProcesses.LastIndex; i++)
		if (this->ProtectedProcesses.Processes[i] == pid)
			return true;
	return false;
}

bool ProcessUtils::AddProcess(ULONG pid)
{
	AutoLock locker(this->ProtectedProcesses.Lock);

	for (ULONG i = 0; i < MAX_PIDS; i++)
		if (this->ProtectedProcesses.Processes[i] == 0) {
			this->ProtectedProcesses.Processes[i] = pid;
			this->ProtectedProcesses.PidsCount++;

			if (i > this->ProtectedProcesses.LastIndex)
				this->ProtectedProcesses.LastIndex = i;
			return true;
		}
	return false;
}

bool ProcessUtils::RemoveProcess(ULONG pid)
{
	ULONG newLastIndex = 0;
	AutoLock locker(this->ProtectedProcesses.Lock);

	for (ULONG i = 0; i <= this->ProtectedProcesses.LastIndex; i++) {
		if (this->ProtectedProcesses.Processes[i] == pid) {
			this->ProtectedProcesses.Processes[i] = 0;

			if (i == this->ProtectedProcesses.LastIndex)
				this->ProtectedProcesses.LastIndex = newLastIndex;
			this->ProtectedProcesses.PidsCount--;
			return true;
		}
		else if (this->ProtectedProcesses.Processes[i] != 0)
			newLastIndex = i;
	}
	return false;
}
