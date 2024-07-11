#include "SAC.h"

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
#ifdef DRIVER_REFLECTIVELY_LOADED
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);



#endif

	return SACEntry(DriverObject, RegistryPath);
}


NTSTATUS SACEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

}