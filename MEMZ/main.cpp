#include "memz.h"

int scrw, scrh;

HWND mainWindow; // In the main window, in the main window, in the main window, ...
HFONT font;
HWND dialog;

void main() {
	scrw = GetSystemMetrics(SM_CXSCREEN);
	scrh = GetSystemMetrics(SM_CYSCREEN);

	InitCommonControls();

	dialog = NULL;

	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	font = CreateFont(lf.lfHeight, lf.lfWidth,
		lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	WNDCLASSEX c;
	c.cbSize = sizeof(WNDCLASSEX);
	c.lpfnWndProc = WindowProc;
	c.lpszClassName = L"MEMZPanel";
	c.style = CS_HREDRAW | CS_VREDRAW;
	c.cbClsExtra = 0;
	c.cbWndExtra = 0;
	c.hInstance = NULL;
	c.hIcon = 0;
	c.hCursor = 0;
	c.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	c.lpszMenuName = NULL;
	c.hIconSm = 0;

	RegisterClassEx(&c);

	RECT rect;
	rect.left = 0;
	rect.right = WINDOWWIDTH;
	rect.top = 0;
	rect.bottom = WINDOWHEIGHT;

	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

	mainWindow = CreateWindowEx(0, L"MEMZPanel", L"MEMZ Clean - Panel de Payloads", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		50, 50, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, GetModuleHandle(NULL), NULL);

	for (int p = 0; p < nPayloads; p++) {
		payloads[p].btn = CreateWindowW(L"BUTTON", payloads[p].name, (p==0?WS_GROUP:0) | WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHLIKE | BS_AUTOCHECKBOX | BS_NOTIFY,
			(p%COLUMNS)*BTNWIDTH+SPACE*(p%COLUMNS+1), (p/COLUMNS)*BTNHEIGHT + SPACE*(p/COLUMNS+1), BTNWIDTH, BTNHEIGHT,
			mainWindow, NULL, (HINSTANCE)GetWindowLong(mainWindow, GWL_HINSTANCE), NULL);
		SendMessage(payloads[p].btn, WM_SETFONT, (WPARAM)font, TRUE);

		CreateThread(NULL, NULL, &payloadThread, &payloads[p], NULL, NULL);
	}

	SendMessage(mainWindow, WM_SETFONT, (WPARAM)font, TRUE);

	ShowWindow(mainWindow, SW_SHOW);
	UpdateWindow(mainWindow);
	
	CreateThread(NULL, NULL, &keyboardThread, NULL, NULL, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (dialog == NULL || !IsDialogMessage(dialog, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	
	if (msg == WM_ACTIVATE) {
		if (wParam == NULL)
			dialog = NULL;
		else
			dialog = hwnd;
	} else if (msg == WM_DESTROY) {
		ExitProcess(0);
	} else if (msg == WM_COMMAND) {
		if (wParam == BN_CLICKED && SendMessage((HWND)lParam, BM_GETCHECK, 0, NULL) == BST_CHECKED) {
			for (int p = 0; p < nPayloads; p++) {
				if (payloads[p].btn == (HWND)lParam && !payloads[p].safe) {
					SendMessage((HWND)lParam, BM_SETCHECK, BST_UNCHECKED, NULL);
					// Most ugly formatting EVER
					if (MessageBoxA(hwnd,
						"Este payload se considera semi-perjudicial.\r\nEsto significa que puede ser seguro, pero puede causar problemas u otras cosas que tal vez no desees.\r\n\r\n\
Si tienes datos importantes o tienes cuentas online, se recomienda ejecutar este payload dentro de una \
VM para evitar problemas o cambios que tal vez no desees.\r\n\r\n\
De todas formas quieres habilitarlo?",
"MEMZ", MB_YESNO | MB_ICONWARNING) == IDYES) {
						SendMessage((HWND)lParam, BM_SETCHECK, BST_CHECKED, NULL);
					}
				}
			}
		}
	} else if (msg == WM_PAINT) {
		hdc = BeginPaint(hwnd, &ps);
		SelectObject(hdc, font);
		LPWSTR str;
		LPWSTR state = enablePayloads ? L"HABILITADOS" : L"DESHABILITADOS";
		FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			L"Los payloads estan %1. SHIFT+ESC para activar todos los payloads!", 0, 0, (LPWSTR)&str, 1024, (va_list*)&state);

		TextOut(hdc, 10, WINDOWHEIGHT - 36, str, lstrlen(str));
		TextOut(hdc, 10, WINDOWHEIGHT - 20, L"CTRL+SHIFT+S para saltar el tiempo (acelera algunos payloads)", 61);

		EndPaint(hwnd, &ps);
	} else {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

DWORD WINAPI keyboardThread(LPVOID lParam) {
	for (;;) {
		if ((GetKeyState(VK_SHIFT) & GetKeyState(VK_ESCAPE)) & 0x8000) {
			enablePayloads = !enablePayloads;

			if (!enablePayloads) {
				RECT rect;
				HWND desktop = GetDesktopWindow();
				GetWindowRect(desktop, &rect);

				RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);

				EnumWindows(&CleanWindowsProc, NULL);
			} else {
				RedrawWindow(mainWindow, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}

			while ((GetKeyState(VK_SHIFT) & GetKeyState(VK_ESCAPE)) & 0x8000) {
				Sleep(100);
			}
		} else if ((GetKeyState(VK_SHIFT) & GetKeyState(VK_CONTROL) & GetKeyState('S')) & 0x8000) {
			if (enablePayloads) {
				for (int p = 0; p < nPayloads; p++) {
					if (SendMessage(payloads[p].btn, BM_GETCHECK, 0, NULL) == BST_CHECKED) {
						payloads[p].delay = payloads[p].payloadFunction(payloads[p].times++, payloads[p].runtime += payloads[p].delay, TRUE);
					}
				}
			}
		}

		Sleep(10);
	}

	return 0;
}

BOOL CALLBACK CleanWindowsProc(HWND hwnd, LPARAM lParam) {
	DWORD pid;
	if (GetWindowThreadProcessId(hwnd, &pid) && pid == GetCurrentProcessId() && hwnd != mainWindow) {
		SendMessage(hwnd, WM_CLOSE, 0, 0);
	}
	return TRUE;
}