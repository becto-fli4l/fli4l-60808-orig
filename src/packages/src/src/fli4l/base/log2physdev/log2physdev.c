/****************************************************************************
 * @file log2physdev.c                                                      *
 * @author Christoph Schulz (develop@kristov.de)                            *
 *                                                                          *
 * This program maps a Windows drive letter (e.g. D:) to the underlying     *
 * physical device (e.g. \\.\PhysicalDrive1).                               *
 *                                                                          *
 * $Id$                                                                     *
 ****************************************************************************/

#include <windows.h>
#include <stdio.h>

static char szDeviceName[MAX_PATH];

static void printError(DWORD error)
{
	LPTSTR msg;
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL,
			  error,
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			  (LPTSTR) &msg,
			  0,
			  NULL)  > 0) {
		printf ("Error: %s\n", msg);
		LocalFree(msg);
	} else {
		printf ("Error: Code %lu\n", error);
	}
}

int main(int argc, char *argv[])
{
	HANDLE hDisk;
	STORAGE_DEVICE_NUMBER diskNumber;
	DWORD bytesReturned;

	if (argc != 2) {
		printf("usage: %s <device>\n", argv[0]);
		return 1;
	}

	snprintf(szDeviceName, sizeof(szDeviceName), "\\\\.\\%s", argv[1]);
	hDisk = CreateFile(
			szDeviceName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
	);
	if (hDisk == INVALID_HANDLE_VALUE) {
		printError(GetLastError());
		return 2;
	}

	if (!DeviceIoControl(
			hDisk,
			IOCTL_STORAGE_GET_DEVICE_NUMBER,
			NULL,
			0,
			&diskNumber,
			sizeof(STORAGE_DEVICE_NUMBER),
			&bytesReturned,
			NULL)) {
		printError(GetLastError());
		CloseHandle(hDisk);
		return 3;
	}

	CloseHandle(hDisk);
	printf("\\\\.\\PhysicalDrive%lu\n", diskNumber.DeviceNumber);
	return 0;
}
