// obliviontrainer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
using namespace std;

DWORD GetProcessId(const wchar_t* processName) {
	DWORD processId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32W procEntry;
		procEntry.dwSize = sizeof(procEntry);
		do {
			if (_wcsicmp(procEntry.szExeFile, processName) == 0) {
				processId = procEntry.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnap, &procEntry));
	}
	CloseHandle(hSnap);
	return processId;
}


uintptr_t InternalScan(char* base, size_t size, const char* pattern, const char* mask) {
	size_t patternlength = strlen(mask);
	for (size_t i = 0; i < size - patternlength; i++) {
		bool found = true;
		for (size_t j = 0; j < size - patternlength; j++) {
			if (mask[j] != '?' && pattern[j] != base[i + j]) {
				found = false;
				break;
			}
			if (found) {
				return (uintptr_t)(base + i);
			}
		}
	}
	return 0;
}

uintptr_t ExternalAOBScan(HANDLE hProcess, const char* pattern, const char* mask) {
	MEMORY_BASIC_INFORMATION mbi;
	uintptr_t address = 0;

	while (VirtualQueryEx(hProcess, (LPCVOID)address, &mbi, sizeof(mbi))) {
		if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_READWRITE) {
			vector<char>  buffer(mbi.RegionSize);
			SIZE_T bytesRead = 0;

			if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
				uintptr_t match = InternalScan(buffer.data(), mbi.RegionSize, pattern, mask);
				if (match) {
					return ((uintptr_t)mbi.BaseAddress + (match - (uintptr_t)buffer.data());
				}
			}
		}
		address += mbi.RegionSize;
	}
	return 0;
}

int main()
{
	cout << "Trainer for Oblivion" << endl;


	DWORD processId = GetProcessId(L"Oblivion.exe");
	if (processId == 0) {
		cout << "[ERROR] Oblivion.exe is not running! Open the game first.\n";
		system("pause");
		return 1;
	}

	cout << "[SUCCESS] Found process! ID: " << processId << "\n";

	DWORD accessRights = PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION;

	HANDLE processHandle = OpenProcess(accessRights, FALSE, processId);
	if (processHandle == NULL) {
		cout << "Failed to open process because" << GetLastError() << endl;
		system("pause");
		return 1;
	}

	cout << "Successfully opened process" << endl;
	const char* goldAOB = "\x29\x47\x04";
	const char* goldMask = "xxx";

	uintptr_t instructionAddress = ExternalAOBScan(processHandle, goldAOB, goldMask);

	if (instructionAddress == 0) {
		cout << "[ERROR] Could not find the instruction address!" << endl;
		system("pause");
		CloseHandle(processHandle);
		return 1;
	}

	uintptr_t goldAddress = instructionAddress;
	int goldValue = 0;

	while (true) {
		cout << "Enter the amount of gold you want: ";
		cin >> goldValue;
		if (goldValue == -1) {
			break;

		}

		WriteProcessMemory(processHandle, (LPVOID)goldAddress, &goldValue, sizeof(goldValue), NULL);
	}

	CloseHandle(processHandle);
	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
