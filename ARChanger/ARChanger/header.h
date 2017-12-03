#pragma once
#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <TlHelp32.h>
#include <psapi.h>
#include <thread>
#include <chrono>
#include <Shlwapi.h>
#include <Pathcch.h>
#include "..\..\Include\C\WinlicenseSDK.h"
#pragma comment(lib, "Shlwapi.lib")

using namespace std;

LPVOID GetBaseAddress(HANDLE hProc)
{
	MODULEINFO miInfo;
	if (GetModuleInformation(hProc, nullptr, &miInfo, sizeof(miInfo)))
		return miInfo.EntryPoint;
	return nullptr;
}

DWORD GetMemorySize(HANDLE hProc)
{
	PROCESS_MEMORY_COUNTERS pmcInfo;
	if (GetProcessMemoryInfo(hProc, &pmcInfo, sizeof(pmcInfo)))
		return static_cast<DWORD>(pmcInfo.WorkingSetSize);
	return 0;
}

DWORD FindPattern(HANDLE ProcessHandle, const byte Signature[], unsigned const int ByteCount) {
	const static unsigned int bufsiz = 4096;
	const static unsigned int StartAdress = reinterpret_cast<int>(GetBaseAddress(ProcessHandle));
	const static unsigned int EndAdress = StartAdress + GetMemorySize(ProcessHandle);
	static bool match;

	byte buffer[bufsiz];

	for (auto i = StartAdress; i < EndAdress; i += bufsiz - ByteCount/2) {
		ReadProcessMemory(ProcessHandle, reinterpret_cast<LPCVOID>(i), &buffer, bufsiz, nullptr);
		for (unsigned int a = 0; a < bufsiz; a++) {
			match = true;

			for (unsigned int j = 0; j < ByteCount && match; j++) {
				if (buffer[a + j] != Signature[j] && Signature[j] != 0x00) match = false;
			}
			if (match) {
				return i + a;
			}
		}
	}
	return 0;
}