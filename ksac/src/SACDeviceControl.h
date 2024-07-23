#pragma once
#include "pch.h"

#include "SACCommon.h"
#include "SACStructs.h"
#include "ProcessUtils.h"

#define VALID_SIZE(DataSize, StructSize)(DataSize != 0 && DataSize % StructSize == 0)


// ** IOCTLS **********************************************************************************************

#define IOCTL_ATTACH CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE CTL_CODE(0x8000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTECT_UNPROTECT_PROCESS CTL_CODE(0x8000, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

// *******************************************************************************************************


NTSTATUS SACDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	SIZE_T len = 0;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	static PEPROCESS target_process = nullptr;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {

	case IOCTL_ATTACH: {
		
		Request request{};

		auto size = stack->Parameters.DeviceIoControl.InputBufferLength;

		if (!VALID_SIZE(size, sizeof(Request))) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		auto data = (Request*)Irp->AssociatedIrp.SystemBuffer;
		request.process_id = data->process_id;
		request.buffer = data->buffer;
		request.target = data->target;
		request.size = data->size;
		request.return_size = data->return_size;

		status = PsLookupProcessByProcessId(request.process_id, &target_process);

		len += sizeof(Request);

		break;
	}

	case IOCTL_READ: {

		Request request{};

		auto size = stack->Parameters.DeviceIoControl.InputBufferLength;

		if (!VALID_SIZE(size, sizeof(Request))) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		auto data = (Request*)Irp->AssociatedIrp.SystemBuffer;
		request.process_id = data->process_id;
		request.buffer = data->buffer;
		request.target = data->target;
		request.size = data->size;
		request.return_size = data->return_size;

		if (target_process != nullptr) {
			status = MmCopyVirtualMemory(target_process, request.target, 
										PsGetCurrentProcess(), request.buffer, 
										request.size, KernelMode, &request.return_size);
		}
		else {
			status = STATUS_UNSUCCESSFUL;
		}


		len += sizeof(Request);

		break;
	}

	case IOCTL_WRITE: {

		Request request{};

		auto size = stack->Parameters.DeviceIoControl.InputBufferLength;

		if (!VALID_SIZE(size, sizeof(Request))) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		auto data = (Request*)Irp->AssociatedIrp.SystemBuffer;
		request.process_id = data->process_id;
		request.buffer = data->buffer;
		request.target = data->target;
		request.size = data->size;
		request.return_size = data->return_size;

		if (target_process != nullptr) {
			status = MmCopyVirtualMemory(PsGetCurrentProcess(), request.buffer,
				target_process, request.target,
				request.size, KernelMode, &request.return_size);
		}
		else {
			status = STATUS_UNSUCCESSFUL;
		}


		len += sizeof(Request);
		break;
	}

	case IOCTL_PROTECT_UNPROTECT_PROCESS: {

		ProtectedProcess protectedProcess{};

		if (!Features.ProcessProtection) {
			//Print(DRIVER_PREFIX "Due to previous error, process protection feature is unavaliable.\n");
			debug_print(DRIVER_PREFIX "Due to previous error, process protection feature is unavaliable.\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		auto size = stack->Parameters.DeviceIoControl.InputBufferLength;

		if (!VALID_SIZE(size, sizeof(ProtectedProcess))) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		auto data = (ProtectedProcess*)Irp->AssociatedIrp.SystemBuffer;
		protectedProcess.Pid = data->Pid;
		protectedProcess.Protect = data->Protect;

		if (!VALID_PROCESS(protectedProcess.Pid)) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (protectedProcess.Protect) {
			if (SACProcessUtils->GetProtectedProcessesCount() == MAX_PIDS) {
				status = STATUS_TOO_MANY_CONTEXT_IDS;
				break;
			}

			if (SACProcessUtils->FindProcess(protectedProcess.Pid))
				break;

			if (!SACProcessUtils->AddProcess(protectedProcess.Pid)) {
				status = STATUS_UNSUCCESSFUL;
				break;
			}

			//Print(DRIVER_PREFIX "Protecting process with pid %d.\n", protectedProcess.Pid);
			debug_print(DRIVER_PREFIX "Protecting process game.\n");
		}
		else {
			if (SACProcessUtils->GetProtectedProcessesCount() == 0) {
				status = STATUS_NOT_FOUND;
				break;
			}

			if (!SACProcessUtils->RemoveProcess(protectedProcess.Pid)) {
				status = STATUS_NOT_FOUND;
				break;
			}

			//Print(DRIVER_PREFIX "Unprotecting process with pid %d.\n", protectedProcess.Pid);
			debug_print(DRIVER_PREFIX "Unprotecting process.\n");
		}

		len += sizeof(ProtectedProcess);
		break;
	}

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