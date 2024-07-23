#pragma once
#include "pch.h"


// ** General Structures ***************************************************************************************

// Temp struct

struct Request {
	HANDLE process_id;

	PVOID target;
	PVOID buffer;

	SIZE_T size;
	SIZE_T return_size;
};