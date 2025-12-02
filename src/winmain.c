#include "config.h"
#include "resource.h"

/* Прототип процедуры обработки окна */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* Прототип процедуры обработки диалога */
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

UINT CALLBACK GetFileNameOffset(LPWSTR lpszFullFileName)
{
	UINT uOffset = (UINT)wcslen(lpszFullFileName) - NEXT;

	while (lpszFullFileName[uOffset] != BACKSLASH && uOffset)
		uOffset--;

	if (!uOffset)
		return uOffset;
	
	return uOffset + NEXT;
}

BOOL CALLBACK CreateProcessFromFile(LPWSTR lpszExeName, LPWSTR lpszFileName)
{
	// Выделение памяти под хранение аргументов командной строки
	if (LPWSTR lpszCmdLine = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * (wcslen(lpszExeName) + DBL(NEXT) + NEXT + wcslen(lpszFileName) + DBL(NEXT) + END)))
	{
		wsprintf(lpszCmdLine, L"\"%s\" \"%s\"", lpszExeName, lpszFileName);

		STARTUPINFO si = { sizeof(STARTUPINFO) }; // настройки создаваемого процесса
		PROCESS_INFORMATION pi;                   // сведения о созданном процессе

		BOOL bIsCreated;
		if (bIsCreated = CreateProcess(
			lpszExeName, // имя модуля, который необходимо запустить
			lpszCmdLine, // командная строка
			NULL,        // указатели на
			NULL,        // структуру безопасности 
			FALSE,       // дескрипторы не наследуются
			NONE,        // флаги создания процесса
			NULL,        // блок окружения
			NULL,        // каталог процесса
			&si,
			&pi
		))
		{
			// Закрытие дескриптора процесса
			CloseHandle(pi.hProcess);

			// Закрытие дескриптора потока
			CloseHandle(pi.hThread);
		}

		// Освобождение памяти от аргументов командной строки
		HeapFree(GetProcessHeap(), NONE, lpszCmdLine);

		return bIsCreated;
	}

	return FALSE;
}

/* Головная функция. Точка входа в приложение */
INT APIENTRY wWinMain(
	HINSTANCE hInstance,     // дескриптор экземпляра приложения
	HINSTANCE hPrevInstance, // дескриптор предыдущего экземпляра приложения
	LPWSTR    lpszCmdLine,   // аргументы командной строки
	INT       nCmdShow       // режим отображения окна
)
{
	/* Неиспользуемые параметры */
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpszCmdLine);

	/* Обработка аргументов командной строки */
	INT argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (!argv) // не получилось прочитать командную строку
		return EXIT_FAILURE;

	/* Создание класса окна */
	WNDCLASSEX wndClass =
	{
		sizeof(WNDCLASSEX),                             // размер структуры в байтах
		CS_HREDRAW | CS_VREDRAW,                        // стиль окна
		WndProc,                                        // процедура обработки окна
		NONE, NONE,                                     // количество дополнительных байтов
		hInstance,                                      // дескриптор экземпляра приложения
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP)),  // иконка приложения
		LoadCursor(NULL, IDC_ARROW),                    // курсор окна приложения
		(HBRUSH)COLOR_WINDOW,                           // кисть для заливки фона окна приложения
		NULL,                                           // имя или ID меню из ресурсов
		WC_APP_W,                                       // имя класса окна
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP))   // маленькая иконка приложения
	};

	/* Регистрация класса окна в системе */
	RegisterClassEx(&wndClass);

	/* Формирование заголовка окна */
	LPWSTR lpszAppTitle;

	if (argc & PASSED)
	{
		UINT uOffset = GetFileNameOffset(argv[NEXT]);

		// Строка заголовка с именем открытого файла
		if (lpszAppTitle = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * (wcslen(APP) + wcslen(argv[NEXT] + uOffset) + DBL(NEXT) + END)))
			wsprintf(lpszAppTitle, L"%s: %s", APP, argv[NEXT] + uOffset);
	}
	else
	{
		// Строка заголовка по умолчанию
		if (lpszAppTitle = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * (wcslen(APP) + END)))
			wcscat(lpszAppTitle, APP);
	}

	/* Создание окна */
	HWND hWnd = CreateWindow(
		WC_APP_W,                                                 // имя класса окна
		lpszAppTitle,                                             // имя заголовка окна
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // стиль окна
		HALF(GetSystemMetrics(SM_CXSCREEN) - WNDWIDTH),           // положение окна по X
                HALF(GetSystemMetrics(SM_CYSCREEN) - WNDHEIGHT) - SHIFT,  // положение окна по Y
		WNDWIDTH, WNDHEIGHT,                                      // ширина и высота окна
		HWND_DESKTOP,                                             // дескриптор родительского окна
		NULL,                                                     // меню
		hInstance,                                                // дескриптор экземпляра приложения
		argv[NEXT]                                                // дополнительные параметры
	);
	if (!hWnd || !lpszAppTitle) // не удалось создать окно
		return EXIT_FAILURE;

	/* Отображение окна */
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	/* Освобождение памяти заголовка */
	HeapFree(GetProcessHeap(), NONE, lpszAppTitle);

	/* Создание таблицы акселераторов */
	ACCEL accelTable[] =
	{
		{ FCONTROL | FVIRTKEY, 'N', IDM_OPEN },
		{ FCONTROL | FVIRTKEY, 'O', IDM_OPEN },
		{ FCONTROL | FVIRTKEY, 'S', IDM_SAVE }
	};
	HACCEL hAccel = CreateAcceleratorTable(accelTable, ARRAYSIZE(accelTable));

	/* Цикл обработки сообщений */
	MSG msg;
	while (GetMessage(&msg, NULL, NONE, NONE)) // извлечение сообщения из очереди
	{
		if (!TranslateAccelerator(hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg); // преобразование нажатых клавиш
			DispatchMessage(&msg);  // вызов оконной функции
		}
	}

	return (INT)msg.wParam;
}

/* Определение процедуры обработки окна */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Дескрипторы дочерних окон
	static HWND hEdit,       // поле редактирования файла
                    hCombo,      // раскрывающийся список заданий
                    hButton,     // кнопка для выполнения
                    hDescFrame,  // рамка с описанием
                    hTotalFrame; // рамка с темой лабораторной

	static HMENU hMenu;                       // дескриптор меню
	static HMENU hMenuItems[ARRAYSIZE(MENU)]; // дескрипторы пунктов меню

	static WCHAR lpszExeName[MAX_PATH];       // имя приложения
	static WCHAR lpszOpenFileName[MAX_PATH];  // имя открытого файла
	static WCHAR lpszAppTitle[MAX_PATH];      // имя заголовка окна
	static WCHAR lpszWorkingDir[MAX_PATH];    // рабочий каталог
	static WCHAR lpszProjsDir[MAX_PATH];      // каталог c проектами

	static SIZE_T  nAppTitleLen; // длина заголовка окна
	static LRESULT nTaskId;      // индентификатор задания

	/* Обработка сообщений */
	switch (uMsg)
	{
		/* Создание окна */
		case WM_CREATE:
		{
			/* Создание меню */
			hMenu = CreateMenu();

			// Добавление пунктов меню
			for (UINT i = FIRST_INDEX; i < ARRAYSIZE(MENU); i++)
			{
				hMenuItems[i] = CreatePopupMenu();
				AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuItems[i], MENU[i]);
			}
			
			// Добавление пунктов подменю
			UINT_PTR uIdm = IDM_FIRSTMENUID;
			for (UINT i = FIRST_INDEX; i < ARRAYSIZE(DEFAULTSUBMENU); i++)
				for (UINT j = FIRST_INDEX; DEFAULTSUBMENU[i][j]; j++)
					AppendMenu(hMenuItems[i], MF_STRING, uIdm++, DEFAULTSUBMENU[i][j]);

			/* Получение имени приложения */
			GetModuleFileName(NULL, lpszExeName, ARRAYSIZE(lpszExeName));

			/* Получение рабочего каталога*/
			wcscpy(lpszWorkingDir, lpszExeName);
			PathCchRemoveFileSpec(lpszWorkingDir, ARRAYSIZE(lpszProjsDir));

			/* Получение каталога проектов */
			wsprintf(lpszProjsDir, L"%s\\%s", lpszWorkingDir, DEFAULTCREATEDIR);

			/*  Создание поля редактирования */
			if ((LPWSTR)((LPCREATESTRUCT)lParam)->lpCreateParams)
			{
				// Получение имени открываемого файла
				wcscpy(lpszOpenFileName, (LPWSTR)((LPCREATESTRUCT)lParam)->lpCreateParams);

				// Формирование заголовка окна
				wsprintf(lpszAppTitle, L"%s: %s", APP, lpszOpenFileName + GetFileNameOffset(lpszOpenFileName));
				nAppTitleLen = wcslen(lpszAppTitle);

				HANDLE hOpenFile;   // дескриптор открываемого файла

				LPWSTR lpszContent; // буфер для содержимого файла
				DWORD  dwFileSize;  // размер буфера

				// Открытие переданного файла
				if ((hOpenFile = CreateFile(
					lpszOpenFileName,      // имя файла
					GENERIC_READ,          // права доступа
					FILE_SHARE_READ,       // режим совместного использования
					NULL,                  // дескриптор безопасности
					OPEN_EXISTING,         // открытие существующего файла
					FILE_ATTRIBUTE_NORMAL, // атрибуты и флаги файла
					NULL                   // дескриптор файла шаблона
				)) != INVALID_HANDLE_VALUE)
				{
					// Получение размера файла
					dwFileSize = GetFileSize(hOpenFile, NULL);

					// Выделение памяти под буфер
					lpszContent = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize + sizeof(WCHAR));			
					
					// Чтение файла
					DWORD dwBytesRead;
					if (!ReadFile(hOpenFile, lpszContent, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
					{
						MessageBox(hWnd, L"Не получилось открыть или полностью прочитать этот файл.", NULL, MB_ICONERROR | MB_OK);
						DestroyWindow(hWnd);
					}

					// Закрытие файла
					CloseHandle(hOpenFile);

					// Добавление дополнительных пунктов подменю
					for (UINT i = FIRST_INDEX; i < ARRAYSIZE(OTHERSUBMENU); i++)
						for (UINT j = FIRST_INDEX; OTHERSUBMENU[i][j]; j++)
							AppendMenu(hMenuItems[i], MF_STRING, uIdm++, OTHERSUBMENU[i][j]);

					// Изменение меню
					SetMenu(hWnd, hMenu);

					hEdit = CreateWindowEx( // поле редактирования
						WS_EX_CLIENTEDGE,
						WC_EDIT,
						lpszContent,
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN,
						CW_USEDEFAULT, CW_USEDEFAULT,
						CW_USEDEFAULT, CW_USEDEFAULT,
						hWnd,
						(HMENU)IDW_EDIT,
						(HINSTANCE)GetModuleHandle(NULL),
						NULL
					);

					// Освобождение памяти буферов
					HeapFree(GetProcessHeap(), NONE, lpszContent);
				}
				else
				{
					MessageBox(hWnd, L"Не получилось открыть этот файл.", NULL, MB_ICONERROR | MB_OK);
					DestroyWindow(hWnd);
				}

				break;
			}

			/* Назначение меню окну */
			AppendMenu(*hMenuItems, MF_STRING, (UINT_PTR)IDM_EXIT, EXIT);
			SetMenu(hWnd, hMenu);
			
			/* Создание раскрывающегося списка заданий */
			hCombo = CreateWindow(
				WC_COMBOBOX_W,
				NULL,
				WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd,
				(HMENU)IDW_COMBO,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			
			// Добавление заданий
			for (UINT uNum = FIRST; uNum <= LAST; uNum++)
			{
				WCHAR lpszFullTaskName[TASKNAMESIZE + CHARACTERLENGTH + END]; // строка с полным именем задания
				wsprintf(lpszFullTaskName, TASKNAME, uNum);

				SendMessage(hCombo, CB_ADDSTRING, NONE, (LPARAM)lpszFullTaskName);
			}
			SendMessage(hCombo, CB_SETCURSEL, NONE, NONE);
			
			/* Создание кнопки запуска задания */
			hButton = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				WC_BUTTON_W,
				RUN,
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd,
				(HMENU)IDW_BUTTON,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);

			/* Создание рамки для описания задания */
			hDescFrame = CreateWindow(
				WC_BUTTON_W,
				DESCRIPTION,
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);

			/* Создание общей объединяющей рамки */
			hTotalFrame = CreateWindow(
				WC_BUTTON_W,
				TOTAL,
				WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
		}
		break;

		/* Изменение размеров окна */
		case WM_SIZE:
		{
			INT nWndWidth  = LOWORD(lParam); // ширина окна
			INT nWndHeight = HIWORD(lParam); // высота окна

			/* Центрирование элементов управления */
			if (hEdit)
			{
				MoveWindow(hEdit, HALF(nWndWidth - WNDEDITWIDTH), HALF(nWndHeight - WNDEDITHEIGHT), WNDEDITWIDTH, WNDEDITHEIGHT, TRUE);
				break;
			}

			MoveWindow(
				hCombo, // раскрывающийся список
				HALF(nWndWidth - WNDCOMBOSIZE - WNDBUTTONWIDTH), HALF(nWndHeight) - WNDBUTTONHEIGHT - SHIFT, 
				WNDCOMBOSIZE, WNDCOMBOSIZE, 
				TRUE
			);
			MoveWindow(
				hButton, // кнопка
				HALF(nWndWidth), HALF(nWndHeight) - WNDBUTTONHEIGHT - SHIFT, 
				WNDBUTTONWIDTH, WNDBUTTONHEIGHT, 
				TRUE
			);
			MoveWindow(
				hDescFrame, // рамка с описанием задания
				HALF(nWndWidth - WNDCOMBOSIZE - WNDBUTTONWIDTH), HALF(nWndHeight + WNDBUTTONHEIGHT) - SHIFT, 
				WNDDESCFRAMEWIDTH, WNDDESCFRAMEHEIGHT, 
				TRUE
			);
			MoveWindow(
				hTotalFrame, // общая рамка с названием
				HALF(nWndWidth - WNDTOTALFRAMEWIDTH), HALF(nWndHeight - WNDTOTALFRAMEHEIGHT), 
				WNDTOTALFRAMEWIDTH, WNDTOTALFRAMEHEIGHT, 
				TRUE
			);
		}
		break;

		/* Наведение курсора на дочерние окна */
		case WM_SETCURSOR:
		{
			if ((HWND)wParam == hCombo)
			{
				SetCursor(LoadCursor(NULL, IDC_HAND));
				return TRUE;
			}
			else if ((HWND)wParam != hEdit)
				SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		break;
		
		/* Выбор исполнительной команды */
		case WM_COMMAND:
		{
			/* Обработка команд */
			switch (LOWORD(wParam))
			{
				/* Обработка пункта меню "Создать" */
				case IDM_CREATE:
				{
					// Создание папки "projs", если она не существует
					DWORD dwAttrs = GetFileAttributes(lpszProjsDir);
					if (dwAttrs == INVALID_FILE_ATTRIBUTES || !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY))
						CreateDirectory(lpszProjsDir, NULL);
					
					// Создание имени нового файла по умолчанию
					wsprintf(lpszOpenFileName, L"%s\\%s.%s", lpszProjsDir, DEFAULTCREATEFILENAME, EXT);

					// Создание подходящего имени файла, если файл с подобным именем уже сужествует
					for (UINT nAttemp = FIRST; GetFileAttributes(lpszOpenFileName) != INVALID_FILE_ATTRIBUTES; nAttemp++)
					{
						ZeroMemory(lpszOpenFileName, sizeof lpszOpenFileName);
						wsprintf(lpszOpenFileName, L"%s\\%s%u.%s", lpszProjsDir, DEFAULTCREATEFILENAME, nAttemp, EXT);
					}

					HANDLE hCreateFile; // дескриптор создаваемого файла

					// Создание файла
					if ((hCreateFile = CreateFile(
						lpszOpenFileName,
						GENERIC_WRITE,
						FILE_SHARE_READ,
						NULL,
						CREATE_NEW,
						FILE_ATTRIBUTE_NORMAL,
						NULL
					)) != INVALID_HANDLE_VALUE)
					{
						// Закрытие файла
						CloseHandle(hCreateFile);

						// Открытие созданного файла в новом процессе
						if (!CreateProcessFromFile(lpszExeName, lpszOpenFileName))
							MessageBox(hWnd, L"Не получилось запустить процесс создания файла.", NULL, MB_ICONERROR | MB_OK);
					}
					else
						MessageBox(hWnd, L"Возвращен неверный дескриптор файла при создании.", NULL, MB_ICONERROR | MB_OK);
				}
				break;

				/* Обработка пункта меню "Открыть" */
				case IDM_OPEN:
				{
					// Структура для формирования диалога выбора файла
					OPENFILENAME ofn    = { DEFAULT };
					ofn.lStructSize     = sizeof(OPENFILENAME);
					ofn.hwndOwner       = hWnd;
					ofn.hInstance       = NULL;
					ofn.lpstrFilter     = OFNFILTER;
					ofn.lpstrFile       = lpszOpenFileName;
					ofn.nMaxFile        = ARRAYSIZE(lpszOpenFileName);
					ofn.lpstrInitialDir = lpszProjsDir;
					ofn.Flags           = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_PATHMUSTEXIST;
					ofn.lpstrDefExt     = EXT;

					// Создание диалога выбора файла и процесса для его открытия
					if (GetOpenFileName(&ofn))
						if (!CreateProcessFromFile(lpszExeName, lpszOpenFileName))
							MessageBox(hWnd, L"Не получилось запустить процесс открытия файла.", NULL, MB_ICONERROR | MB_OK);
				}
				break;

				/* Обработка пункта меню "Сохранить как..." */
				case IDM_SAVEAS:
				{
					// Структура для формирования диалога сохранения файла
					OPENFILENAME ofn    = { DEFAULT };
					ofn.lStructSize     = sizeof(OPENFILENAME);
					ofn.hwndOwner       = hWnd;
					ofn.hInstance       = NULL;
					ofn.lpstrFilter     = OFNFILTER;
					ofn.lpstrFile       = lpszOpenFileName;
					ofn.nMaxFile        = ARRAYSIZE(lpszOpenFileName);
					ofn.lpstrInitialDir = lpszProjsDir;
					ofn.Flags           = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_PATHMUSTEXIST;
					ofn.lpstrDefExt     = EXT;
					
					if (!GetSaveFileName(&ofn))
						break;
				}

				/* Обработка пункта меню "Сохранить" */
				case IDM_SAVE:
				{
					LPWSTR lpszContent; // строка текста поля редактирования
					INT nContentLen = GetWindowTextLength(hEdit); // длина этой строки

					// Выделение памяти под строковый буфер
					if (nContentLen && (lpszContent = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * (nContentLen + END))))
					{
						// Копирование текста из поля редактирования в буфер 
						GetWindowText(hEdit, lpszContent, nContentLen + END);
						
						HANDLE hSaveFile; // дескриптор сохраняемого файла

						// Открытие файла на сохранение
						if ((hSaveFile = CreateFile(
							lpszOpenFileName,
							GENERIC_WRITE,
							FILE_SHARE_WRITE,
							NULL, 
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL
						)) != INVALID_HANDLE_VALUE)
						{
							// Запись содержимого буфера в файл
							DWORD dwBytesWritten;
							if (!WriteFile(hSaveFile, lpszContent, sizeof(WCHAR) * nContentLen, &dwBytesWritten, NULL) || dwBytesWritten != sizeof(WCHAR) * nContentLen)
								MessageBox(hWnd, L"Не получилось сохранить файл!", NULL, MB_ICONERROR | MB_OK);

							// Изменение маркера сохранения
							if (LOWORD(wParam) != IDM_SAVEAS)
							{
								lpszAppTitle[nAppTitleLen] = NULLCHARACTER;
								SetWindowText(hWnd, lpszAppTitle);
							}

							// Закрытие файла
							CloseHandle(hSaveFile);
						}

						// Освобождение буфера
						HeapFree(GetProcessHeap(), NONE, lpszContent);
					}
				}
				break;

				/* Обработка пункта меню "Выход" */
				case IDM_EXIT:
					SendMessage(hWnd, WM_CLOSE, NONE, NONE); break;

				/* Обработка пункта меню "О программе..." */
				case IDM_ABOUT:
					MessageBox(hWnd, L"Программу написал ст.гр. И-124 Смирнов Никита Ильич для выполнения лабораторной работы № 6 по реестру Windows.", L"О программе", MB_ICONINFORMATION | MB_OK);
					break;

				/* Обработка поля ввода */
				case IDW_EDIT:
				{
					// Выполнены изменения в тексте
					if (HIWORD(wParam) == EN_CHANGE)
					{
						// Изменение маркера сохранения
						lpszAppTitle[nAppTitleLen] = NOTSAVEDMARKER;
						SetWindowText(hWnd, lpszAppTitle);
					}
				}

				/* Обработка раскрывающегося списка */
				case IDW_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE) // выбор задания
					{
						// Получение индентификатора выбранного задания
						nTaskId = SendMessage(hCombo, CB_GETCURSEL, NONE, NONE);

						// Перерисовка окна
						InvalidateRect(hWnd, NULL, TRUE);
					}
				}
				break;

				/* Обработка нажатия на кнопку */
				case IDW_BUTTON:
				{
					HKEY hKey;       // дескриптор создаваемого ключа
					LSTATUS nStatus; // состояние возврата

					/* Обработка выбранного задания */
					switch (nTaskId)
					{
						case TASK_(1): // задание № 1
						{
							/* Строка с абсолютным путем к иконке файла приложения */
							WCHAR lpszAppfileIco[wcslen(lpszWorkingDir) + NEXT + wcslen(ICOAPPFILE) + END];
							wsprintf(lpszAppfileIco, L"%s\\%s", lpszWorkingDir, ICOAPPFILE);

							/* Строка с абсолютным путем к приложению и запускаемым файлом */
							WCHAR lpszExeToOpen[wcslen(lpszExeName) + DBL(DBL(NEXT)) + DBL(NEXT) + NEXT + END];
							wsprintf(lpszExeToOpen, L"\"%s\" \"%%1\"", lpszExeName);

							/* Данные для записи в реестр */
							LPCWSTR ASSOCDATA[] = 
							{
								APPFILE,        // расширение файла
								DESCAPPFILE,    // тип расширения
								lpszAppfileIco, // иконка файла
								lpszExeToOpen   // открывающая программа
							};
							
							for (UINT uI = FIRST_INDEX; uI < ARRAYSIZE(ASSOCIATIONS); uI++)
							{
								/* Создание ключа */
								if ((nStatus = RegCreateKeyEx(
									HKEY_CLASSES_ROOT,       // предопределенный ключ
									ASSOCIATIONS[uI],        // имя создаваемого ключа
									NONE,                    // резерв
									NULL,                    // класс создаваемого ключа
									REG_OPTION_NON_VOLATILE, // опции создаваемого ключа
									KEY_WRITE,               // права доступа к ключу
									NULL,                    // дескриптор безопасности
									&hKey,                   // получение дескриптора создаваемого ключа
									NULL                     // получение данных о ликвидации 
							    	)) == ERROR_SUCCESS)
								{
									/* Установка значения для созданного ключа */
									RegSetValueEx(
										hKey,                                         // дескриптор созданного ключа 
										NULL,                                         // имя значения, если NULL, то по умолчанию
										NONE,                                         // резерв
										REG_SZ,                                       // тип значения
										(LPCBYTE)ASSOCDATA[uI],                       // данные значения
										sizeof(WCHAR) * (wcslen(ASSOCDATA[uI]) + END) // размер данных
									);

									/* Закрытие созданного/открытого ключа */
									RegCloseKey(hKey); // и применение всех изменений
								}
								else goto REG_ERROR; // переход на ошибку
							}

							/* Обновление ассоциаций проводника */
							SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
						}
						/* Отображение сообщения об успешном выполнении */
						return MessageBox(hWnd, L"Задание № 1 выполнено! Все ассоциации успешно добавлены.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(2): // задание № 2
						{
							/* Строка с абсолютным путем к иконке приложения */
							WCHAR lpszAppIco[wcslen(lpszWorkingDir) + NEXT + wcslen(ICOAPP) + END];
							wsprintf(lpszAppIco, L"%s\\%s", lpszWorkingDir, ICOAPP);

							/* Строка с абсолютным путем к приложению и запускаемым файлом */
							WCHAR lpszExeToOpen[wcslen(lpszExeName) + DBL(DBL(NEXT)) + DBL(NEXT) + NEXT + END];
							wsprintf(lpszExeToOpen, L"\"%s\" \"%%1\"", lpszExeName);

							/* Данные для изменения */
							LPCWSTR CONTEXTMENUDATA[] =
							{
								lpszAppIco,   // иконка приложения
								lpszExeToOpen // открывающая программа
							};

							for (UINT uI = FIRST_INDEX; uI < ARRAYSIZE(CONTEXTMENU); uI++)
							{
								/* Создание/открытие ключа */
								if ((nStatus = RegCreateKeyEx(
									HKEY_CLASSES_ROOT, 
									CONTEXTMENU[uI], 
									NONE, 
									NULL, 
									REG_OPTION_NON_VOLATILE, 
									KEY_WRITE, 
									NULL, 
									&hKey, 
									NULL
								)) == ERROR_SUCCESS)
								{
									/* Изменение значений ключа */
									RegSetValueEx(
										hKey,
										CONTEXTMENUVALUENAMES[uI],
										NONE,
										REG_SZ,
										(LPCBYTE)CONTEXTMENUDATA[uI],
										sizeof(WCHAR) * (wcslen(CONTEXTMENUDATA[uI]) + END)
									);

									/* Закрытие ключа */
									RegCloseKey(hKey);
								}
								else goto REG_ERROR; // переход на ошибку
							}
						}
						/*Отображение сообщения об успешном выполнении */
						return MessageBox(hWnd, L"Задание № 2 выполнено! Контекстное меню успешно добавлено.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(3): // задание № 3
						{
							/* Открытие ключа */
							if ((nStatus = RegCreateKeyEx(
								HKEY_LOCAL_MACHINE,
								REGSYS(REGPOLICIES(CURRENTVERSION)), 
								NONE, 
								NULL, 
								REG_OPTION_NON_VOLATILE, 
								KEY_SET_VALUE, 
								NULL, 
								&hKey,
								NULL
							)) == ERROR_SUCCESS)
							{
								/* Изменение данных значений ключа */
								for (UINT uI = FIRST_INDEX; uI < ARRAYSIZE(LEGALNAMES); uI++)
									RegSetValueEx(
										hKey,
										LEGALNAMES[uI],
										NONE,
										REG_SZ,
										(LPCBYTE)LEGALVALUES[uI],
										sizeof(WCHAR) * (wcslen(LEGALVALUES[uI]) + END)
									);

								/* Закрытие ключа */
								RegCloseKey(hKey);
							}
							else break;

						}
						/* Отображение сообщение об успешном выполнении */
						return MessageBox(hWnd, L"Задание № 3 выполнено! Тайное сообщение успешно добавлено.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(4): // задание № 4
						{
							/* Создание/открытие ключа */
							if ((nStatus = RegCreateKeyEx(
								HKEY_CURRENT_USER, 
								REGEXPLORER(REGPOLICIES(CURRENTVERSION)), 
								NONE, 
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_WRITE,
								NULL,
								&hKey,
								NULL
							)) == ERROR_SUCCESS)
							{
								/* Изменение значений ключа */
								RegSetValueEx(hKey, NOCONTROLPANEL, NONE, REG_DWORD, (LPCBYTE)&(DWORD){ ON }, sizeof(DWORD));
								
								/* Закрытие ключа */
								RegCloseKey(hKey);
							}
							else break;
						}
						/* Отображение сообщения об успешном выполнении */
						return MessageBox(hWnd, L"Задание № 4 выполнено! Доступ к панели управления заблокирован.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(5): // задание № 5
						{
							/* Строка с абсолютным путем к иконке приложения */
							WCHAR lpszAppIco[wcslen(lpszWorkingDir) + NEXT + wcslen(ICOAPP) + END];						
							wsprintf(lpszAppIco, L"%s\\%s", lpszWorkingDir, ICOAPP);

							/* Строка с абсолютным путем к иконке файла приложения */
							WCHAR lpszAppfileIco[wcslen(lpszWorkingDir) + NEXT + wcslen(ICOAPPFILE) + END];						
							wsprintf(lpszAppfileIco, L"%s\\%s", lpszWorkingDir, ICOAPPFILE);

							/* Данные для изменения */
							LPCWSTR APPICODATA[] =
							{
								lpszAppIco,     // иконка приложения
								lpszAppfileIco  // иконка файла приложения
							};

							/* Открытие ключа */
							if ((nStatus = RegCreateKeyEx(
								HKEY_CURRENT_USER, 
								DEFAULTICON(REGTRASH(REGEXPLORER(CURRENTVERSION))),
								NONE,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_SET_VALUE,
								NULL,
								&hKey,
								NULL
							)) == ERROR_SUCCESS)
							{
								/* Изменение значений ключа */
								for (UINT uI = FIRST_INDEX; uI < ARRAYSIZE(EMPTYFULL); uI++)
									RegSetValueEx(hKey, EMPTYFULL[uI], NONE, REG_SZ, (LPCBYTE)APPICODATA[uI], sizeof(WCHAR) * (wcslen(APPICODATA[uI]) + END));

								/* Обновление проводника */
								SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);								

								/* Закрытие ключа */
								RegCloseKey(hKey);
							}
							else break;
						}
						/* Отображение сообщения об успешном выполнении */
						return MessageBox(hWnd, L"Задание № 5 выполнено! Иконка корзины была успешно изменена.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(6):
							/* Отображение сведений о расположении файлов реестра */
							return MessageBox(hWnd, L"Задание № 6 выполнено!\nСведения о расположении файлов реестра хранятся в HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\\nhivelist.", L"Успех", MB_ICONINFORMATION | MB_OK);

						case TASK_(7):
						{
							/* Открытие ключа */
							if ((nStatus = RegCreateKeyEx(
								HKEY_LOCAL_MACHINE,
								REGRUN(CURRENTVERSION),
								NONE,
								NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey,
								NULL
							)) == ERROR_SUCCESS)
							{
								/* Создание диалогового окна */
								HWND hDlg = CreateDialog((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_VIEWBOX), hWnd, DlgProc);

								/* Получение дочернего окна LISTBOX у диалога */
								HWND hList = GetDlgItem(hDlg, IDW_LISTBOX);

								WCHAR lpszValueName[256]; // строка для хранения имени значения
								DWORD dwLenOfValueName = ARRAYSIZE(lpszValueName); // длина имени значения

								LPBYTE lpData; // буфер для хранения данных значения
								DWORD dwSizeOfData = MEGABYTE; // размер буфера данных

								/* Выделение 1 МБ памяти под буфер */
								if (!(lpData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSizeOfData)))
								{
									MessageBox(hWnd, L"Не получилось выделить 1 МБ памяти для данных, читаемых из реестра.", NULL, MB_ICONERROR | MB_OK);
									return EXIT_FAILURE;
								}

								/* Чтение всех значений ключа поиндексно */
								for (DWORD dwI = FIRST_INDEX;
									RegEnumValue(
										hKey,              // дескриптор открытого раздела
										dwI,               // индекс извлекаемого значения
										lpszValueName,     // имя значения
										&dwLenOfValueName, // размер значения
										NULL,              // резерв
										NULL,              // тип значения
										lpData,            // буфер для чтения данных
										&dwSizeOfData      // размер буфера для чтения данных
									) == ERROR_SUCCESS; 
								dwI++, dwLenOfValueName = ARRAYSIZE(lpszValueName), dwSizeOfData = MEGABYTE)
								{
									SIZE_T nLast, nFirst; // смещения для нахождения имени программы

									/* Поиск расширения .exe */
									for (nLast = wcslen((LPWSTR)lpData) - NEXT; nLast; nLast--)
										if (((LPCWSTR)lpData)[nLast] == L'e' && 
										    ((LPCWSTR)lpData)[nLast - NEXT] == L'x' &&
										    ((LPCWSTR)lpData)[nLast - DBL(NEXT)] == L'e' &&
										    ((LPCWSTR)lpData)[nLast - DBL(NEXT) - NEXT] == L'.') break;

									/* Определение начала имени */
									for (nFirst = nLast - DBL(DBL(NEXT)) - NEXT; ((LPCWSTR)lpData)[nFirst] != BACKSLASH; nFirst--);

									/* Извлечение имени программы из буфера */
									WCHAR lpszSubData[nLast - nFirst + END];
									wcsncpy(lpszSubData, (LPCWSTR)lpData + nFirst + NEXT, nLast - nFirst);
									lpszSubData[nLast - nFirst] = NULLCHARACTER;

									/* Отправка сообщения об добавлении имени программы в список */
									SendMessage(hList, LB_ADDSTRING, NONE, (LPARAM)lpszSubData);

									/* Очистка буфера */
									ZeroMemory(lpData, dwSizeOfData);
								}

								/* Отображение сообщения об успешном выполнении */
								MessageBox(hWnd, L"Задание № 7 выполнено!\nНажмите \"ОК\", чтобы увидеть подробности.", L"Успех", MB_ICONINFORMATION | MB_OK);

								/* Отображение диалогового окна */
								ShowWindow(hDlg, SW_SHOW);

								/* Освобождение памяти, выделенной под буфер */
								HeapFree(GetProcessHeap(), NONE, lpData);

								/* Добавление программы в список автозапуска */
								RegSetValueEx(hKey, APP, NONE, REG_SZ, (LPCBYTE)lpszExeName, sizeof(WCHAR) * (wcslen(lpszExeName) + END));

								/* Закрытие ключа */
								RegCloseKey(hKey);
							}
							else break;
						}
						return EXIT_SUCCESS;
					}

					/* Метка для обработки ошибки */
					REG_ERROR:
						LPWSTR lpszError; // строка для записи ошибки

						/* Получение сообщения об ошибке от системы */
						if (FormatMessage(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							nStatus,
							MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT),
							(LPWSTR)&lpszError,
							NONE,
							NULL
						))
						{
							/* Отображение сообщения о неудаче */
							MessageBox(hWnd, lpszError, NULL, MB_ICONERROR | MB_OK);

							/* Освобождение памяти */
							LocalFree(lpszError);
						}
				}
				break;
			}
		}
		break;

		/* Перерисовка окна */
		case WM_PAINT:
		{
			/* Начало перерисовки */
			PAINTSTRUCT paintInfo;                  // для хранения сведений о перерисовке
			HDC hdc = BeginPaint(hWnd, &paintInfo); // получение дескриптора устройства
			
			if (!hEdit)
			{
				/* Отключение фона у текста */
				SetBkMode(hdc, TRANSPARENT);
			
				/* Получение размера окна */
				RECT wndRect;
				GetClientRect(hWnd, &wndRect);
			
				/* Вывод текста в заданный прямоугольник */
				DrawText(
					hdc, 
					TASKDESCS[nTaskId],
					FULL,
					&(RECT){ HALF(wndRect.right - 225), HALF(wndRect.bottom - 100) + SHIFT, HALF(wndRect.right + 225), HALF(wndRect.bottom + SHIFT) + SHIFT },
					DT_LEFT | DT_VCENTER | DT_WORDBREAK | DT_EXPANDTABS
				);
			}

			/* Окончание перерисовки */
			EndPaint(hWnd, &paintInfo);
		}
		break;

		/* Закрытие окна */
		case WM_CLOSE:
		{
			/* Отображение сообщения о сохранении изменений в файле */
			if (hEdit && lpszAppTitle[nAppTitleLen] == NOTSAVEDMARKER)
			{
				/* Строка с основном именем файла */
				LPCWSTR lpszBaseFileName = lpszOpenFileName + GetFileNameOffset(lpszOpenFileName);

				/* Формирование строки с предупреждающим сообщением */
				WCHAR lpszWarningMsg[wcslen(lpszBaseFileName) + 62 + END];
				wsprintf(lpszWarningMsg, L"Файл: %s не сохранен. Вы хотите сохранить изменения в этом файле?", lpszBaseFileName);

				/* Обработка выбора пользователя */
				switch (MessageBox(hWnd, lpszWarningMsg, L"Предупреждение", MB_ICONWARNING | MB_YESNOCANCEL))
				{
					case IDCANCEL: // отмена
						return EXIT_SUCCESS;

					case IDYES:    // подтверждение сохранения
						SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_SAVE, NONE);
				}
			}
		}

		/* Завершение работы приложения */
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS); break;

		/* Остальные сообщения */
		default: 
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return EXIT_SUCCESS;
}

/* Определение процедуры обработки диалога */
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		/* Инициализация диалога */
		case WM_INITDIALOG:
			break;

		/* Обработка команд дочерних окон диалога */
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK) // нажатие "ОК"
				EndDialog(hDlg, NONE);
		}
		break;

		/* Закрытие диалога */
		case WM_CLOSE:
			EndDialog(hDlg, NONE); break;

		/* Обработка остальных сообщений по умолчанию */
		default:
			return (INT_PTR)FALSE;
	}

	return (INT_PTR)TRUE;
}