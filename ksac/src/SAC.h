#pragma once

#include "pch.h"

extern "C" {
	#include "WindowsTypes.h"
}

#include "SACCommon.h"
#include "ProcessUtils.h"
#include "SACDeviceControl.h"



// Definitions.
constexpr SIZE_T REGISTERED_OB_CALLBACKS = 1;
#define DRIVER_NAME L"\\Driver\\sac"
#define DRIVER_DEVICE_NAME L"\\Device\\sac"
#define DRIVER_SYMBOLIC_LINK L"\\??\\sac"
#define OB_CALLBACKS_ALTITUDE L"31205.6171"
#define REG_CALLBACK_ALTITUDE L"31222.6172"

// Prototypes.
NTSTATUS SACEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
DRIVER_UNLOAD SACUnload;
DRIVER_DISPATCH SACDeviceControl, SACCreateClose;

bool InitializeFeatures();
void ClearAll();