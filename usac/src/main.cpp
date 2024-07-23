#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>

#include "SACStructs.h"
#include "SACIoctls.h"

static DWORD get_process_id(const wchar_t* process_name) {

	DWORD process_id = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snap_shot == INVALID_HANDLE_VALUE) {
		return process_id;
	}

	PROCESSENTRY32 entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Process32FirstW(snap_shot, &entry) == TRUE) {

		if (_wcsicmp(process_name, entry.szExeFile) == 0) {
			process_id = entry.th32ProcessID;
		}
		else {
			
			while (Process32NextW(snap_shot, &entry) == TRUE) {

				if (_wcsicmp(process_name, entry.szExeFile) == 0) {
					process_id = entry.th32ProcessID;
					break;
				}

			}

		}
	}

	CloseHandle(snap_shot);

	return process_id;
}

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name) {
	std::uintptr_t module_base = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE) {
		return module_base;
	}

	MODULEENTRY32 entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32First(snap_shot, &entry) == TRUE) {
		
		if (wcsstr(module_name, entry.szModule) != nullptr) {
			module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		}
		else {

			while (Module32NextW(snap_shot, &entry) == TRUE) {

				if (wcsstr(module_name, entry.szModule) != nullptr) {
					module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}

			}

		}

	}

	CloseHandle(snap_shot);

	return module_base;
}



bool attach_to_process(HANDLE DriverHandle, const DWORD pid) {

	Request r;
	r.process_id = reinterpret_cast<HANDLE>(pid);

	return DeviceIoControl(DriverHandle, IOCTL_ATTACH, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}

template <class T>
T read_memory(HANDLE DriverHandle, const std::uintptr_t addr) {
	T temp = {};

	Request r;
	r.target = reinterpret_cast<PVOID>(addr);
	r.buffer = &temp;
	r.size = sizeof(T);

	DeviceIoControl(DriverHandle, IOCTL_READ, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

	return temp;
}

template <class T>
void write_memory(HANDLE DriverHandle, const std::uintptr_t addr, const T& value) {

	Request r;
	r.target = reinterpret_cast<PVOID>(addr);
	r.buffer = (PVOID)&value;
	r.size = sizeof(T);

	DeviceIoControl(DriverHandle, IOCTL_WRITE, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

}

bool protect_unprotect_process(HANDLE DriverHandle, DWORD pid, bool Protect) {

	ProtectedProcess r;
	r.Pid = pid;
	r.Protect = Protect;

	return DeviceIoControl(DriverHandle, IOCTL_PROTECT_UNPROTECT_PROCESS, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}


int main() {

	const HANDLE driver = CreateFile(L"\\\\.\\sac", GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create our driver handle.\n";
		std::cin.get();
		return 1;
	}




	DWORD pid = 0;

	do {

		pid = get_process_id(L"ac_client.exe");

		Sleep(1000);
	} while (pid == 0);


	/*if (pid == 0) {
		std::cout << "Failed to find ac_client.\n";
		std::cin.get();
		return 1;
	}*/

	if (protect_unprotect_process(driver, pid, true)) {
		std::cout << "ac_client protected.\n";
		std::cin.get();
	}
	else {
		std::cout << "Failed to protect the process ac_client.\n";
		std::cin.get();
	}
	

	if (protect_unprotect_process(driver, pid, false)) {
		std::cout << "ac_client unprotected.\n";
		std::cin.get();
	}
	else {
		std::cout << "Failed to unprotect the process ac_client.\n";
		std::cin.get();
	}

	//if (attach_to_process(driver, pid)) {
	//	
	//	std::cout << "Attachment successful.\n";

	//	const std::uintptr_t client = get_module_base(pid, L"ac_client.exe");

	//	if (client != 0) {

	//		while (true) {
	//			
	//			if (GetAsyncKeyState(VK_END)) {
	//				break;
	//			}

	//			DWORD offsetHP = 0xEC;

	//			const auto localplayer = read_memory<std::uintptr_t>(driver, client + 0x17E0A8);
	//			//printf("localplayer: 0x%08X\n", localplayer);

	//			if (localplayer == 0) {
	//				continue;
	//			}

	//			if (GetAsyncKeyState(VK_INSERT)) {
	//				write_memory(driver, localplayer + offsetHP, 50);
	//			}


	//		}


	//	}


	//}




	CloseHandle(driver);

	std::cin.get();

	return 0;
}