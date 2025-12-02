#ifndef CONFIG_H_
#define CONFIG_H_

#include <windows.h>
#include <pathcch.h>
#include <shlobj.h>

#define TASK_(param)     ((param) - NEXT)
#define DBL(param)       ((param) * 2)
#define HALF(param)      ((param) / 2)
#define DEFAULTICON(key) key L"\\DefaultIcon"
#define REGSHELL(key)    key L"\\shell"
#define REGCMD(key)      key L"\\command"
#define REGOPEN(key)     key L"\\open"
#define REGOPENIN(key)   L"Открыть в " L"\"" key L"\""
#define REGPOLICIES(key) key L"\\Policies"
#define REGSYS(key)      key L"\\System"
#define REGEXPLORER(key) key L"\\Explorer"
#define REGRUN(key)      key L"\\Run"
#define REGTRASH(key)    key L"\\CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}"

#define APP                   L"Лаб. № 6"
#define RUN                   L"ВЫПОЛНИТЬ"
#define DESCRIPTION           L"Описание"
#define TOTAL                 L"Системный реестр Windows"
#define TASKNAME              L"ЗАДАНИЕ № %u"
#define DEFAULTCREATEFILENAME L"untitled"
#define DEFAULTCREATEDIR      L"projs"
#define DESCAPPFILE           L"Файл программы \"Лаб. № 6\""
#define OFNFILTER             L"APP Файлы (*.lab)\0*.lab\0Все файлы (*.*)\0*.*\0\0"
#define EXT                   L"lab"
#define ICOAPP                L"res\\imgs\\lab.ico"
#define ICOAPPFILE            L"res\\imgs\\labfile.ico"
#define APPFILE               L"labfile"
#define CURRENTVERSION        L"Software\\Microsoft\\Windows\\CurrentVersion"
#define NOCONTROLPANEL        L"NoControlPanel"
#define FILE                  L"Файл"
#define HELP                  L"Справка"
#define EXIT                  L"Выход"
#define ASTERISK              L"*"

#define WC_APP_W      L"APP"
#define WC_EDIT_W     L"EDIT"
#define WC_BUTTON_W   L"BUTTON"
#define WC_COMBOBOX_W L"COMBOBOX"

#define FULL               -1
#define DEFAULT             0
#define NONE                0
#define FIRST_INDEX         0
#define NULLCHARACTER       0
#define ON                  1
#define NEXT                1
#define CHARACTERLENGTH     1
#define END                 1
#define FIRST               1
#define PASSED              2
#define LAST                7
#define TASKNAMESIZE        10
#define WNDBUTTONHEIGHT     25
#define SHIFT               50
#define WNDDESCFRAMEHEIGHT  120
#define WNDBUTTONWIDTH      120
#define WNDCOMBOSIZE        125
#define WNDDESCFRAMEWIDTH   (WNDBUTTONWIDTH + WNDCOMBOSIZE)
#define WNDTOTALFRAMEWIDTH  (WNDDESCFRAMEWIDTH + 100)
#define WNDTOTALFRAMEHEIGHT (WNDDESCFRAMEHEIGHT + WNDBUTTONHEIGHT + 100)
#define WNDHEIGHT           350
#define WNDWIDTH            450
#define WNDEDITWIDTH        3 * WNDWIDTH / 4
#define WNDEDITHEIGHT       3 * WNDHEIGHT / 4
#define KILOBYTE            1024
#define MEGABYTE            KILOBYTE * 1024
#define BACKSLASH           L'\\'
#define NOTSAVEDMARKER      L'*'

#define IDM_FIRSTMENUID     30001
#define IDM_CREATE          30001
#define IDM_OPEN            30002
#define IDM_ABOUT           30003
#define IDM_SAVE            30004
#define IDM_SAVEAS          30005
#define IDM_EXIT            30006
#define IDW_EDIT            40001
#define IDW_COMBO           40002
#define IDW_BUTTON          40003

CONST UINT_PTR SUBMENUIDS[] =
{
	IDM_CREATE,
	IDM_OPEN,
	IDM_SAVE,
	IDM_SAVEAS,
	IDM_EXIT,
	IDM_ABOUT
};

CONST LPCWSTR MENU[] =
{
	L"Файл",
	L"Справка"
};

CONST LPCWSTR DEFAULTSUBMENU[][3] =
{
	{ L"Создать", L"Открыть", NULL },
	{ L"О программе...", NULL }
};

CONST LPCWSTR OTHERSUBMENU[][4] =
{
	{ L"Сохранить", L"Сохранить как...", L"Выход", NULL }
};

CONST LPCWSTR TASKDESCS[] = 
{
	L"«Создание нового расширения для запуска нескольких приложений»",
	L"«Создание собственных контекстных меню»",
	L"«Создание тайного сообщения при входе в систему»",
	L"«Ограничение доступа к определенным найстройкам Windows»",
	L"«Изменение иконок системных объектов через реестр»",
	L"«Определение расположения файлов реестра»",
	L"«Определение программ в автозапуске»"
};

LPCWSTR ASSOCIATIONS[] =
{
	L"." EXT,
	APPFILE,
	DEFAULTICON(APPFILE),
	REGCMD(REGOPEN(REGSHELL(APPFILE)))
};

LPCWSTR CONTEXTMENU[] =
{
	REGSHELL(ASTERISK) "\\" REGOPENIN(APP),
	REGCMD(REGSHELL(ASTERISK) "\\" REGOPENIN(APP))
};

LPCWSTR CONTEXTMENUVALUENAMES[] =
{
	L"Icon",
	NULL
};

LPCWSTR LEGALNAMES[] = 
{
	L"legalnoticecaption",
	L"legalnoticetext"
};

LPCWSTR LEGALVALUES[] = 
{
	APP,
	TOTAL
};

LPCWSTR EMPTYFULL[] =
{
	L"empty",
	L"full"
};

#endif