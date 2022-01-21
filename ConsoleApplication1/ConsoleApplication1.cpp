// ConsoleApplication1.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//
using namespace std;
#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <TlHelp32.h>





DWORD GetProcId(const char* procName) {
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry)) {
			do {
				if (lstrcmpi(procEntry.szExeFile, procName) == 0) {
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}

		CloseHandle(hSnap);
		return procId;
	}

}


uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_stricmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	else {
		std::cout << "INVALID_HANDLE_VALUE returned" << std::endl;
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

int main() {

	DWORD m_flFlashMxAlpha = 0x1046C;
	DWORD dwLocalPlayer = 0xDB65EC;
	DWORD dwForceJump = 0x527BC98;
	DWORD dwForceAttack = 0x32022D0;
	DWORD m_fFlags = 0x104;
	DWORD dwGlowObjectManager = 0x531A118;
	DWORD m_iTeamNum = 0xF4;
	DWORD dwEntityList = 0x4DD1E1C;
	DWORD m_iGlowIndex = 0x10488;

	
	
	HWND hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive - Direct3D 9");
	if (hwnd == NULL) {
		cout << "Cannot find window." << endl;
		Sleep(3000);
		exit(-1);
	}

	DWORD procId = GetProcId("csgo.exe");
	cout << "Proc ID: " << procId << " ";

	uintptr_t clientDLL = GetModuleBaseAddress(procId, "client.dll");
	uintptr_t panoramaDLL = GetModuleBaseAddress(procId, "client_panorama.dll");
	cout << "Base Addr: " << clientDLL << " Panorama: " << panoramaDLL;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

	while (true)
	{

		DWORD localPlayer;
		ReadProcessMemory(hProc, (LPVOID)(clientDLL + dwLocalPlayer), &localPlayer, sizeof(localPlayer), 0);

		DWORD onGround;
		ReadProcessMemory(hProc, (LPINT)(localPlayer + m_fFlags), &onGround, sizeof(onGround), 0);
		DWORD jump;
		int fj;
		ReadProcessMemory(hProc, (LPVOID)(clientDLL + dwForceJump), &jump, sizeof(jump), 0);

		if (GetAsyncKeyState(VK_RCONTROL) && onGround == 257) {
			fj = 5;
			WriteProcessMemory(hProc, (LPINT)jump, &fj, sizeof(int), 0);
		}
		else {
			fj = 4;
			WriteProcessMemory(hProc, (LPINT)jump, &fj, sizeof(int), 0);
		}
		int flashDur;
		DWORD glowObject;
		ReadProcessMemory(hProc, (LPVOID)(clientDLL + dwGlowObjectManager), &glowObject, sizeof(glowObject), 0);
		int myTeam;
		ReadProcessMemory(hProc, (LPINT)(localPlayer + m_iTeamNum), &myTeam, sizeof(myTeam), 0);
		
		for (short int i = 0; i < 32; i++) {
			DWORD entity;
			ReadProcessMemory(hProc, (LPVOID)(clientDLL + dwEntityList + i * 0x10), &entity, sizeof(entity), 0);
			if (entity != NULL) {
				int entityTeam;
				ReadProcessMemory(hProc, (LPINT)(entity + m_iTeamNum), &entityTeam, sizeof(entityTeam), 0);
				
				int glowIndex;
				ReadProcessMemory(hProc, (LPINT)(entity + m_iGlowIndex), &glowIndex, sizeof(glowIndex), 0);
				float ecx = 0.f;
				float ecy = 1.f;
				float ecz = 0.f;
				float ecw = 1.f;
				float acx = 0.f;;
				float acy = 1.f;
				float acz = 0.f;;
				float acw = 1.f;

				if (myTeam == entityTeam) {
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x8), 0), &ecx, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0xC), 0), &ecy, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x10), 0), &ecz, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x14), 0), &ecw, sizeof(float), 0);
				}
				else {
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x8), 0), &acx, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0xC), 0), &acy, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x10), 0), &acz, sizeof(float), 0);
					WriteProcessMemory(hProc, (LPVOID)(glowObject + ((glowIndex * 0x38) + 0x14), 0), &acw, sizeof(float), 0);
				}
			}
		}

		int noflashVal = 0;
		
		
		if (localPlayer == NULL) {
			while (localPlayer == NULL) {
				ReadProcessMemory(hProc, (LPVOID)(clientDLL + dwLocalPlayer), &localPlayer, sizeof(localPlayer), 0);
			}
		}
		ReadProcessMemory(hProc, (LPINT)(localPlayer + m_flFlashMxAlpha), &flashDur, sizeof(flashDur), 0);
		if (flashDur > 0) {
			WriteProcessMemory(hProc, (LPINT)(localPlayer + m_flFlashMxAlpha), &noflashVal, sizeof(noflashVal), 0);
		}
		cout << "\n skok: " << jump << " ziemia: " << onGround;//"\n Team: " << myTeam;
		
		Sleep(300);
		//
		//WriteProcessMemory(hProc, (LPVOID)dwForceJump, &jump, sizeof(int), 0);
		
	}




}