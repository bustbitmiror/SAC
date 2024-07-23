#pragma once
#include "pch.h"

extern "C" {
	#include "WindowsTypes.h"
}

#include "SACCommon.h"

inline PVOID AllocateMemory(SIZE_T size, bool paged = true) {
	if (AllocatePool2 && WindowsBuildNumber >= WIN_2004) {
		return paged ? ((tExAllocatePool2)AllocatePool2)(POOL_FLAG_PAGED, size, DRIVER_TAG) :
			((tExAllocatePool2)AllocatePool2)(POOL_FLAG_NON_PAGED_EXECUTE, size, DRIVER_TAG);
	}

#pragma warning( push )
#pragma warning( disable : 4996)
	return paged ? ExAllocatePoolWithTag(PagedPool, size, DRIVER_TAG) :
		ExAllocatePoolWithTag(NonPagedPool, size, DRIVER_TAG);
#pragma warning( pop )
}