#pragma once

#include "pch.h"

#define PRINTS
#define DRIVER_REFLECTIVELY_LOADED
#define DRIVER_TAG 'hdiS'
#define DRIVER_PREFIX "SAC: "
constexpr SIZE_T MAX_PATH = 260;


#ifdef PRINTS
typedef ULONG(NTAPI* tDbgPrint)(PCSTR Format, ...);
constexpr tDbgPrint Print = DbgPrint;
#else
constexpr VOID Print(...) {};
#endif


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
