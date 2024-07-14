#pragma once
#include "pch.h"

#define VALID_SIZE(DataSize, StructSize)(DataSize != 0 && DataSize % StructSize == 0)


// ** IOCTLS **********************************************************************************************

#define IOCTL_ATTACH CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE CTL_CODE(0x8000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// *******************************************************************************************************


NTSTATUS SACDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	SIZE_T len = 0;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {


	default: {
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}