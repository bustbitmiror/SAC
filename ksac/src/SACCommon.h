#pragma once

#include "pch.h"


#define DRIVER_REFLECTIVELY_LOADED
#define DRIVER_TAG 'hdiS'
#define DRIVER_PREFIX "SAC: "
constexpr SIZE_T MAX_PATH = 260;


void debug_print(PCSTR text) {

#ifndef DEBUG
	UNREFERENCED_PARAMETER(text);
#endif // !DEBUG


	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}


inline PVOID RegistrationHandle = NULL;

struct EnabledFeatures {
	bool DriverReflectivelyLoaded = false;
	bool FunctionPatching = true;
	bool ModuleHiding = true;
	bool WriteData = true;
	bool ReadData = true;
	bool RegistryFeatures = true;
	bool ProcessProtection = true;
	bool ThreadProtection = true;
	bool FileProtection = true;
	bool EtwTiTamper = true;
	bool ApcInjection = true;
	bool CreateThreadInjection = false;
};
inline EnabledFeatures Features;
