/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "pch.h"

long _priSize = 0;
void* _priData = nullptr;

static UINT(WINAPI* RealGetDriveTypeW)(LPCWSTR lpRootPathName) = GetDriveTypeW;
static BOOL(WINAPI* RealGetFileAttributesExW)(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation) = GetFileAttributesExW;
static HANDLE(WINAPI* RealCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;

UINT WINAPI MyGetDriveTypeW(LPCWSTR lpRootPathName)
{
#if DEBUG
	std::wcout << "GetDriveTypeW " << lpRootPathName << std::endl;
#endif
	return DRIVE_REMOVABLE;
}

BOOL WINAPI MyGetFileAttributesExW(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	if (fInfoLevelId == GetFileExInfoStandard && std::filesystem::path(lpFileName).filename() == "resources.pri")
	{
#if DEBUG
		std::wcout << "GetFileAttributesExW " << lpFileName << std::endl;
#endif
		auto info = reinterpret_cast<LPWIN32_FILE_ATTRIBUTE_DATA>(lpFileInformation);
		info->nFileSizeHigh = 0;
		info->nFileSizeLow = _priSize;
		return TRUE;
	}
	return RealGetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
}

HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (std::filesystem::path(lpFileName).filename() == "resources.pri")
	{
#if DEBUG
		std::wcout << "CreateFileW Redirect to pipe" << std::endl;
#endif
		HANDLE hReadPipe, hWritePipe;
		if (!CreatePipe(&hReadPipe, &hWritePipe, nullptr, _priSize))
			return INVALID_HANDLE_VALUE;

		DWORD written;
		if (!WriteFile(hWritePipe, _priData, _priSize, &written, nullptr) || written != _priSize)
		{
			CloseHandle(hWritePipe);
			CloseHandle(hReadPipe);
			return INVALID_HANDLE_VALUE;
		}

		CloseHandle(hWritePipe);
		return hReadPipe;
	}
	return RealCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

extern "C" void __declspec(dllexport) PatchMrm(BYTE* data, long length)
{
#if DEBUG
	AllocConsole();

	// std::cout, std::clog, std::cerr, std::cin
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	// std::wcout, std::wclog, std::wcerr, std::wcin
	HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
	SetStdHandle(STD_ERROR_HANDLE, hConOut);
	SetStdHandle(STD_INPUT_HANDLE, hConIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();
#endif
	_priSize = length;
	_priData = malloc(length);
	if (_priData)
		memcpy(_priData, data, length);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)RealGetDriveTypeW, MyGetDriveTypeW);
	DetourAttach(&(PVOID&)RealGetFileAttributesExW, MyGetFileAttributesExW);
	DetourAttach(&(PVOID&)RealCreateFileW, MyCreateFileW);
	DetourTransactionCommit();

}

extern "C" void __declspec(dllexport) UnpatchMrm()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)RealGetDriveTypeW, MyGetDriveTypeW);
	DetourDetach(&(PVOID&)RealGetFileAttributesExW, MyGetFileAttributesExW);
	DetourDetach(&(PVOID&)RealCreateFileW, MyCreateFileW);
	DetourTransactionCommit();
	if (_priData)
		free(_priData);
	_priData = nullptr;
}
