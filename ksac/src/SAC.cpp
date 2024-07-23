#include "SAC.h"

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
#ifdef DRIVER_REFLECTIVELY_LOADED
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	Features.DriverReflectivelyLoaded = true;


	//Print(DRIVER_PREFIX "Driver is being reflectively loaded...\n");
	debug_print(DRIVER_PREFIX "Driver is being reflectively loaded...\n");


	UNICODE_STRING driverName = RTL_CONSTANT_STRING(DRIVER_NAME);
	UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"IoCreateDriver");
	tIoCreateDriver IoCreateDriver = (tIoCreateDriver)MmGetSystemRoutineAddress(&routineName);

	if (!IoCreateDriver) {
		return STATUS_INCOMPATIBLE_DRIVER_BLOCKED;
	}

	NTSTATUS status = IoCreateDriver(&driverName, &SACEntry);

	if (!NT_SUCCESS(status)) {
		//Print(DRIVER_PREFIX "Failed to create driver: (0x%08X)\n", status);
		debug_print(DRIVER_PREFIX "Failed to create driver\n");
	}

	return status;

#else

	return SACEntry(DriverObject, RegistryPath);

#endif
}


NTSTATUS SACEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	if (!InitializeFeatures()) {
		ClearAll();
		return STATUS_INCOMPATIBLE_DRIVER_BLOCKED;
	}

	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DRIVER_DEVICE_NAME);
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(DRIVER_SYMBOLIC_LINK);
	UNICODE_STRING altitude = RTL_CONSTANT_STRING(OB_CALLBACKS_ALTITUDE);
	UNICODE_STRING regAltitude = RTL_CONSTANT_STRING(REG_CALLBACK_ALTITUDE);
	PDEVICE_OBJECT DeviceObject = nullptr;

	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		//Print(DRIVER_PREFIX "Failed to create device: (0x%08X)\n", status);
		debug_print(DRIVER_PREFIX "Failed to create device.\n");
		ClearAll();
		return status;
	}

	status = IoCreateSymbolicLink(&symbolicLink, &deviceName);

	if (!NT_SUCCESS(status)) {
		//Print(DRIVER_PREFIX "Failed to create symbolic link: (0x%08X)\n", status);
		debug_print(DRIVER_PREFIX "Failed to create symbolic link.\n");
		IoDeleteDevice(DeviceObject);
		ClearAll();
		return status;
	}

	if (!Features.DriverReflectivelyLoaded) {
		OB_OPERATION_REGISTRATION operations[] = {
		{
			PsProcessType,
			OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
			OnPreOpenProcess, nullptr
		}
		/*{
			PsThreadType,
			OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
			OnPreOpenThread, nullptr
		}*/
		};
		OB_CALLBACK_REGISTRATION registrationCallbacks = {
			OB_FLT_REGISTRATION_VERSION,
			REGISTERED_OB_CALLBACKS,
			RTL_CONSTANT_STRING(OB_CALLBACKS_ALTITUDE),
			nullptr,
			operations
		};

		status = ObRegisterCallbacks(&registrationCallbacks, &RegistrationHandle);

		if (!NT_SUCCESS(status)) {
			//Print(DRIVER_PREFIX "Failed to register process callback: (0x%08X)\n", status);
			debug_print(DRIVER_PREFIX "Failed to register process callback\n");
			dbg("text");
			status = STATUS_SUCCESS;
			Features.ProcessProtection = false;
			//Features.ThreadProtection = false;
		}

		/*status = CmRegisterCallbackEx(OnRegistryNotify, &regAltitude, DriverObject, nullptr, &NidhoggRegistryUtils->RegCookie, nullptr);

		if (!NT_SUCCESS(status)) {
			Print(DRIVER_PREFIX "Failed to register registry callback: (0x%08X)\n", status);
			status = STATUS_SUCCESS;
			Features.RegistryFeatures = false;
		}*/
	}
	else {
		DeviceObject->Flags |= DO_BUFFERED_IO;
		DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	}

	DriverObject->DriverUnload = SACUnload;  // ?
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = SACCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SACDeviceControl;

	//ExecuteInitialOperations();

	//Print(DRIVER_PREFIX "Initialization finished.\n");
	debug_print(DRIVER_PREFIX "Initialization finished.\n");
	return status;
}


void SACUnload(PDRIVER_OBJECT DriverObject) {
	//Print(DRIVER_PREFIX "Unloading...\n");
	debug_print(DRIVER_PREFIX "Unloading...\n");

	/*if (Features.RegistryFeatures) {
		NTSTATUS status = CmUnRegisterCallback(NidhoggRegistryUtils->RegCookie);

		if (!NT_SUCCESS(status)) {
			Print(DRIVER_PREFIX "Failed to unregister registry callbacks: (0x%08X)\n", status);
		}
	}*/

	ClearAll();

	// To avoid BSOD.
	if (Features.ThreadProtection && Features.ProcessProtection && RegistrationHandle) {
		ObUnRegisterCallbacks(RegistrationHandle);
		RegistrationHandle = NULL;
	}

	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(DRIVER_SYMBOLIC_LINK);
	IoDeleteSymbolicLink(&symbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}


NTSTATUS SACCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	//Irp->IoStatus.Status = STATUS_SUCCESS;
	//Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}


void ClearAll() {

}

bool InitializeFeatures() {

	RTL_OSVERSIONINFOW osVersion = { sizeof(osVersion) };
	NTSTATUS result = RtlGetVersion(&osVersion);

	if (!NT_SUCCESS(result)) {
		return false;
	}

	WindowsBuildNumber = osVersion.dwBuildNumber;
	if (WindowsBuildNumber < WIN_1507) {
		return false;
	}

	// Initialize utils.
	SACProcessUtils = new ProcessUtils();

	if (!SACProcessUtils)
		return false;



	// Initialize functions.
	if (!(PULONG)MmCopyVirtualMemory)
		Features.ReadData = false;

	if (!(PULONG)ZwProtectVirtualMemory || !Features.ReadData)
		Features.WriteData = false;

	if (!Features.WriteData || !(PULONG)PsGetProcessPeb)
		Features.FunctionPatching = false;

	if (!(PULONG)PsGetProcessPeb || !(PULONG)PsLoadedModuleList || !&PsLoadedModuleResource)
		Features.ModuleHiding = false;

	if (!(PULONG)ObReferenceObjectByName)
		Features.FileProtection = false;

	if (!(PULONG)KeInsertQueueApc)
		Features.EtwTiTamper = false;

	if (!(PULONG)KeInitializeApc || !(PULONG)KeInsertQueueApc || !(PULONG)KeTestAlertThread || !(PULONG)ZwQuerySystemInformation)
		Features.ApcInjection = false;


	return true;
}

