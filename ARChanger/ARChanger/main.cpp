#include "header.h"


#pragma warning(disable : 4995)  
int main(){
	SetConsoleTitle("");
	char szFullPath[MAX_PATH] = {};
	GetCurrentDirectory(MAX_PATH, szFullPath);
	PathAppendA(szFullPath, "yeye.png");
	DWORD ret = GetFileAttributesA(szFullPath); 
	if (ret == INVALID_FILE_ATTRIBUTES || GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		cout << "Please put yeye.png back" << endl;
	}
	ret = INVALID_FILE_ATTRIBUTES;
	//printf("%s\n", szFullPath);
	HWND hwnd;
	DWORD pid;
	HANDLE handle;
	DWORD addr;
	DWORD offset = 5;
	/* This is for academic purposes.
	 * Therefore, I zero out the first byte of the pattern so that if someone knows nothing about coding, he cannot just c+p this code and use/sell this hack
	 * However, if you know something about x86 asm, you may know what should be filled into the 1st byte. 
	 * Basically, osu! calculates actual ApproachTime(or "PreEmpt" by peppy) each time when it loads a map.
	 * (int PreEmpt = (int)SomeCalculation(Beatmap->ApproachRate, min, mid, max); )
	 * And this program patch the code where osu! does the truncation. (from double to int)
	 * 
	 */
	byte oPattern[] = { 0x00 ,0x0F ,0x10 ,0x45 ,0xE0 ,0xF2 ,0x0F ,0x2C ,0xC0 ,0x89 ,0x46 ,0x1C };
	byte patch[] = { 0xC7 ,0x46 ,0x1C ,0x00 ,0x00 ,0x00 ,0x00 };
	byte nPattern[] = { 0x00 ,0x0F ,0x10 ,0x45 ,0xE0 ,0xC7 ,0x46 ,0x1C };
	byte unpatch[] = { 0xF2 ,0x0F ,0x2C ,0xC0 ,0x89, 0x46, 0x1C };
	DWORD ApproachTime = 0;
	int timeleft = WLRegGlobalTimeLeft();
	if (timeleft < 0) {
		printf("Key expired\n");
		getc(stdin);
		return -1;
	}
	printf("[Global Execution Time Left]\n%d min\n\n", timeleft);
	cout << "Search \"osu!\" window" << endl;
	hwnd = FindWindowA(nullptr, TEXT("osu!"));
	if (hwnd == nullptr) {
		while (hwnd == nullptr) {
			hwnd = FindWindowA(nullptr, TEXT("osu!"));
			Sleep(100);
		}
	}
	cout << "Osu! founded" << endl;
	GetWindowThreadProcessId(hwnd, &pid);
	handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	const int oPatternLength = sizeof oPattern / sizeof BYTE;
	const int nPatternLength = sizeof nPattern / sizeof BYTE;
	const int patchLength = sizeof patch / sizeof BYTE;
	const int unpatchLength = sizeof unpatch / sizeof BYTE;
	SIZE_T numOfByte = 0;
	bool patched = false;

	ret = GetFileAttributesA(szFullPath); 
	if (ret == INVALID_FILE_ATTRIBUTES || GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		cout << "Please put yeye.png back" << endl;
	}
	ret = INVALID_FILE_ATTRIBUTES;
	while (true) {
		addr = FindPattern(handle, oPattern, oPatternLength);
		if (addr > 0) {
			cout << "Address found" << endl;
			//cout << addr << endl;
			break;
		}
		addr = FindPattern(handle, nPattern, nPatternLength);
		if (addr > 0) {
			cout << "Address found, already patched" << endl;
			//cout << addr << endl;
			patched = true;
			break;
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	DWORD effectiveAddr = addr + offset;
	bool noYeye = false;

	
	while (true) {
		cout << "Please enter ApproachTime in millisecond\n(any input lower than 100 will unpatch osu! and exit the program)" << endl;
		cin >> ApproachTime;
		ret = GetFileAttributesA(szFullPath); 
		if (ret == INVALID_FILE_ATTRIBUTES || GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			cout << "Please put yeye.png back" << endl;
			noYeye = true;
		}
		ret = INVALID_FILE_ATTRIBUTES;
		if (noYeye) {
			ApproachTime = 10;
		}
		if (ApproachTime < 100) {
			if (patched) {
				WriteProcessMemory(handle,(LPVOID)(effectiveAddr),unpatch,unpatchLength,&numOfByte);
				DeleteFile(szFullPath);
				//cout << "Write " << numOfByte << " bytes at address" << (effectiveAddr) << endl;
			}
			exit(0);
		}
		byte buf[patchLength];
		memcpy(buf, patch, 7);
		memcpy((byte*)(buf + 3),&ApproachTime,4);
		WriteProcessMemory(handle, (LPVOID)(effectiveAddr), buf, patchLength, &numOfByte);
		//cout << "Write " << numOfByte << " bytes at address" << (effectiveAddr) << endl;
		patched = true;
		numOfByte = 0;
		this_thread::sleep_for(chrono::milliseconds(500));
	}
}