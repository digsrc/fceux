#include "common.h"
#include "mapinput.h"
#include "../../input.h"
#include "keyscan.h"
#include "keyboard.h"
#include "gui.h"
#include "../../input.h"
#include <commctrl.h>
#include "window.h"


struct INPUTDLGTHREADARGS
{
	HANDLE hThreadExit;
	HWND hwndDlg;
};

static struct
{
	EMUCMD cmd;
	int key;
} DefaultCommandMapping[]=
{
	{ EMUCMD_RESET, 					SCAN_R | KeyCombo::CTRL_BIT, },
	{ EMUCMD_PAUSE, 					SCAN_PAUSE, },
	{ EMUCMD_FRAME_ADVANCE, 			SCAN_BACKSLASH, },
	{ EMUCMD_SCREENSHOT,				SCAN_F12, },
	{ EMUCMD_HIDE_MENU_TOGGLE,			SCAN_ESCAPE },
	{ EMUCMD_SPEED_SLOWER, 				SCAN_MINUS, }, 
	{ EMUCMD_SPEED_FASTER, 				SCAN_EQUAL, },
	{ EMUCMD_SPEED_TURBO, 				SCAN_TAB, }, 
	{ EMUCMD_SAVE_SLOT_0, 				SCAN_0, },
	{ EMUCMD_SAVE_SLOT_1, 				SCAN_1, },
	{ EMUCMD_SAVE_SLOT_2, 				SCAN_2, },
	{ EMUCMD_SAVE_SLOT_3, 				SCAN_3, },
	{ EMUCMD_SAVE_SLOT_4, 				SCAN_4, },
	{ EMUCMD_SAVE_SLOT_5, 				SCAN_5, },
	{ EMUCMD_SAVE_SLOT_6, 				SCAN_6, },
	{ EMUCMD_SAVE_SLOT_7, 				SCAN_7, },
	{ EMUCMD_SAVE_SLOT_8, 				SCAN_8, },
	{ EMUCMD_SAVE_SLOT_9, 				SCAN_9, },
	{ EMUCMD_SAVE_STATE, 				SCAN_I, }, //adelikat, set to my defaults for lack of something better
	{ EMUCMD_LOAD_STATE, 				SCAN_P, }, //most people use the loadslotx / savestlotx style system which requires hogging all th F Keys.
	{ EMUCMD_MOVIE_FRAME_DISPLAY_TOGGLE, 	SCAN_PERIOD, },
	{ EMUCMD_MOVIE_INPUT_DISPLAY_TOGGLE, 	SCAN_COMMA, },
	{ EMUCMD_MISC_DISPLAY_LAGCOUNTER_TOGGLE, 	SCAN_SLASH, },
	{ EMUCMD_MOVIE_READONLY_TOGGLE, 	SCAN_Q, },
	{ EMUCMD_SAVE_STATE_SLOT_0, 		SCAN_F10 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_1, 		SCAN_F1 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_2, 		SCAN_F2 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_3, 		SCAN_F3 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_4, 		SCAN_F4 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_5, 		SCAN_F5 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_6, 		SCAN_F6 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_7, 		SCAN_F7 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_8, 		SCAN_F8 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_SAVE_STATE_SLOT_9, 		SCAN_F9 | KeyCombo::SHIFT_BIT, },
	{ EMUCMD_LOAD_STATE_SLOT_0, 		SCAN_F10, },
	{ EMUCMD_LOAD_STATE_SLOT_1, 		SCAN_F1, },
	{ EMUCMD_LOAD_STATE_SLOT_2, 		SCAN_F2, },
	{ EMUCMD_LOAD_STATE_SLOT_3, 		SCAN_F3, },
	{ EMUCMD_LOAD_STATE_SLOT_4, 		SCAN_F4, },
	{ EMUCMD_LOAD_STATE_SLOT_5, 		SCAN_F5, },
	{ EMUCMD_LOAD_STATE_SLOT_6, 		SCAN_F6, },
	{ EMUCMD_LOAD_STATE_SLOT_7, 		SCAN_F7, },
	{ EMUCMD_LOAD_STATE_SLOT_8, 		SCAN_F8, },
	{ EMUCMD_LOAD_STATE_SLOT_9, 		SCAN_F9, },
	{ EMUCMD_MOVIE_PLAY_FROM_BEGINNING, SCAN_R | KeyCombo::SHIFT_BIT,		},
	{ EMUCMD_SCRIPT_RELOAD,				SCAN_L | KeyCombo::SHIFT_BIT,		},
	{ EMUCMD_OPENROM,					SCAN_O | KeyCombo::CTRL_BIT,	    },
	{ EMUCMD_CLOSEROM,					SCAN_W | KeyCombo::CTRL_BIT,		},
	{ EMUCMD_RELOAD,					SCAN_F1 | KeyCombo::CTRL_BIT ,	},
	{ EMUCMD_MISC_UNDOREDOSAVESTATE,	SCAN_Z | KeyCombo::CTRL_BIT, },
	{ EMUCMD_MISC_TOGGLEFULLSCREEN,		SCAN_ENTER | KeyCombo::ALT_BIT, },
	{ EMUCMD_RERECORD_DISPLAY_TOGGLE,	SCAN_M,	},
	{ EMUCMD_TASEDITOR_REWIND,			SCAN_BACKSPACE, },
	{ EMUCMD_TASEDITOR_RESTORE_PLAYBACK,	SCAN_SPACE,	},
	{ EMUCMD_TASEDITOR_CANCEL_SEEKING,	SCAN_ESCAPE, },
	{ EMUCMD_TASEDITOR_SWITCH_AUTORESTORING,	SCAN_SPACE | KeyCombo::CTRL_BIT, },
	{ EMUCMD_TASEDITOR_SWITCH_MULTITRACKING,	SCAN_W, },
};

#define NUM_DEFAULT_MAPPINGS		(sizeof(DefaultCommandMapping)/sizeof(DefaultCommandMapping[0]))

static uint8 keyonce[MKK_COUNT];

static const char* ScanNames[256]=
{
	/* 0x00-0x0f */ 0, "Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equals", "Backspace", "Tab",
	/* 0x10-0x1f */ "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Left Ctrl", "A", "S",
	/* 0x20-0x2f */ "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Apostrophe", "Tilde", "Left Shift", "Backslash", "Z", "X", "C", "V",
	/* 0x30-0x3f */	"B", "N", "M", "Comma", "Period", "Slash", "Right Shift", "Numpad *", "Left Alt", "Space", "Caps Lock", "F1", "F2", "F3", "F4", "F5",
	/* 0x40-0x4f */ "F6", "F7", "F8", "F9", "F10", "NumLock", "ScrollLock", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad Minus", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad Plus", "Numpad 1",
	/* 0x50-0x5f */	"Numpad 2", "Numpad 3", "Numpad 0", "Numpad Period", 0, 0, "Backslash", "F11", "F12", 0, 0, 0, 0, 0, 0, 0,
	/* 0x60-0x6f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x70-0x7f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x80-0x8f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x90-0x9f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Numpad Enter", "Right Ctrl", 0, 0,
	/* 0xa0-0xaf */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xb0-0xbf */ 0, 0, 0, 0, 0, "Numpad Divide", 0, "PrintScrn", "Right Alt", 0, 0, 0, 0, 0, 0, 0,
	/* 0xc0-0xcf */ 0, 0, 0, 0, 0, "Pause", 0, "Home", "Up Arrow", "PgUp", 0, "Left Arrow", 0, "Right Arrow", 0, "End",
	/* 0xd0-0xdf */ "Down Arrow", "PgDn", "Ins", "Del", 0, 0, 0, 0, 0, 0, 0, "Left Win", "Right Win", "AppMenu", 0, 0,
	/* 0xe0-0xef */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xf0-0xff */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int _keyonly(int a)
{
	driver::input::keyboard::KeysState_const keys_nr = driver::input::keyboard::GetState();
	if(keys_nr[a])
	{
		if(!keyonce[a]) 
		{
			keyonce[a] = 1;
			return 1;
		}
	}
	else
	{
		keyonce[a] = 0;
	}

	return 0;
}

int GetKeyPressed()
{
	int key = 0;
	int i;

	driver::input::keyboard::KeysState_const keys_nr = driver::input::keyboard::GetState();

	for(i = 0; i < 256 && !key; ++i)
	{
		if(_keyonly(i))
		{
			key = i;
		}
	}

	return key;
}

int NothingPressed()
{
	int i;

	driver::input::keyboard::KeysState_const keys_nr = driver::input::keyboard::GetState();

	for(i = 0; i < 256; ++i)
	{
		if(keys_nr[i])
		{
			return 0;
		}
	}

	return 1;
}

int GetKeyMeta(KeyCombo combo)
{
	KeyCombo tmp(0);
	tmp.setMeta(combo.getMeta());

	switch(combo.getKey())
	{
		case SCAN_LEFTCONTROL:
		case SCAN_RIGHTCONTROL:
			tmp.setModifiers(KeyCombo::CTRL_BIT);
			break;
		
		case SCAN_LEFTALT:
		case SCAN_RIGHTALT:
			tmp.setModifiers(KeyCombo::ALT_BIT);
			break;
		
		case SCAN_LEFTSHIFT:
		case SCAN_RIGHTSHIFT:
			tmp.setModifiers(KeyCombo::SHIFT_BIT);
			break;
		
		default:
			break;
	}

	return tmp.getMeta();
}

void ClearExtraMeta(KeyCombo& combo)
{
	switch(combo.getKey())
	{
		case SCAN_LEFTCONTROL:
		case SCAN_RIGHTCONTROL:
			combo.clearModifiers(KeyCombo::CTRL_BIT);
			break;

		case SCAN_LEFTALT:
		case SCAN_RIGHTALT:
			combo.clearModifiers(KeyCombo::ALT_BIT);
			break;

		case SCAN_LEFTSHIFT:
		case SCAN_RIGHTSHIFT:
			combo.clearModifiers(KeyCombo::SHIFT_BIT);
			break;

		default:
			break;
	}
}

DWORD WINAPI NewInputDialogThread(LPVOID lpvArg)
{
	struct INPUTDLGTHREADARGS* args = (struct INPUTDLGTHREADARGS*)lpvArg;

	while (args->hThreadExit)
	{
		if (WaitForSingleObject(args->hThreadExit, 20) == WAIT_OBJECT_0)
			break;

		// Poke our owner dialog periodically.
		PostMessage(args->hwndDlg, WM_USER, 0, 0);
	}

	return 0;
}

/**
* Returns the name of a single key.
*
* @param code Keycode
*
* @return Name of the key
*
* TODO: Replace return value with parameter.
**/
const char* GetKeyName(int code)
{
	static char name[16];

	code &= 0xff;
	
	if(ScanNames[code])
	{
		return ScanNames[code];
	}

	sprintf(name, "Key 0x%.2x", code);

	return name;
}

/**
* Returns the name of a pressed key combination.
* 
* @param c A keycode
*
* @return The name of the key combination.
*
* TODO: Replace return value with parameter.
**/
char* GetKeyComboName(const KeyCombo& combo)
{
	static char text[80];

	text[0] = '\0';
	
	if(!combo.isEmpty()) {
		uint32 mods = combo.getModifiers() & KeyCombo::CTRL_BIT;
		if (mods == KeyCombo::CTRL_BIT) {
			strcat(text, "Ctrl + ");
		}
		else if (mods == KeyCombo::LCTRL_BIT) {
			strcat(text, "Left Ctrl + ");
		}
		else if (mods == KeyCombo::RCTRL_BIT) {
			strcat(text, "Right Ctrl + ");
		}

		mods = combo.getModifiers() & KeyCombo::ALT_BIT;
		if (mods == KeyCombo::ALT_BIT) {
			strcat(text, "Alt + ");
		}
		else if (mods == KeyCombo::LALT_BIT) {
			strcat(text, "Left Alt + ");
		}
		else if (mods == KeyCombo::RALT_BIT) {
			strcat(text, "Right Alt + ");
		}

		mods = combo.getModifiers() & KeyCombo::SHIFT_BIT;
		if (mods == KeyCombo::SHIFT_BIT) {
			strcat(text, "Shift + ");
		}
		else if (mods == KeyCombo::LSHIFT_BIT) {
			strcat(text, "Left Shift + ");
		}
		else if (mods == KeyCombo::RSHIFT_BIT) {
			strcat(text, "Right Shift + ");
		}

		strcat(text, GetKeyName(combo.getKey()));
	}

	return text;
}

//Callback function for the dialog where the user can change hotkeys.
BOOL CALLBACK ChangeInputDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread = NULL;
	static DWORD dwThreadId = 0;
	static struct INPUTDLGTHREADARGS threadargs;
	static KeyCombo combo(0);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Start the message thread.
			threadargs.hThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);
			threadargs.hwndDlg = hwndDlg;
			hThread = CreateThread(NULL, 0, NewInputDialogThread, (LPVOID)&threadargs, 0, &dwThreadId);
			
			combo.assign(0);
			memset(keyonce, 0, sizeof(keyonce));
			
			driver::input::keyboard::SetBackgroundAccessBit(driver::input::keyboard::BKGINPUT_GENERAL);
			SetFocus(GetDlgItem(hwndDlg, LBL_KEY_COMBO));

			CenterWindowOnScreen(hwndDlg);
		}

		return FALSE;

		case WM_COMMAND:
			switch(LOWORD(wParam))	 // CaH4e3: BN_CLICKED redundant define removed since it always 0, Esc mapping used to be handled as well (I need it too :))
			{
				case BTN_OK:
					// Send quit message.
					PostMessage(hwndDlg, WM_USER + 99, 0, 0);
					break;
				case BTN_CLEAR:
					combo.assign(-1);
					// Send quit message.
					PostMessage(hwndDlg, WM_USER + 99, 0, 0);
					break;
				default:
					break;
			}
			break;

		case WM_USER:
			{
				// Our thread sent us a timer signal.
				int newScanCode;
				int meta = GetKeyMeta(combo);

				FCEUD_UpdateInput(UPDATEINPUT_KEYBOARD);

				if((newScanCode = GetKeyPressed()) != 0)
				{
					combo.setKey(newScanCode);
					combo.setMeta(meta);
					ClearExtraMeta(combo);
					SetDlgItemText(hwndDlg, LBL_KEY_COMBO, GetKeyComboName(combo.get()));
				}
				else if(NothingPressed() && !combo.isEmpty())
				{
					PostMessage(hwndDlg, WM_USER + 99, 0, 0);		// Send quit message.
				}
			}
			break;

		case WM_CLOSE:
			// exit without changing the key mapping
			combo.assign(0);
		case WM_USER + 99:
			// Done with keyboard.
			driver::input::keyboard::ClearBackgroundAccessBit(driver::input::keyboard::BKGINPUT_GENERAL);

			// Kill the thread.
			SetEvent(threadargs.hThreadExit);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			CloseHandle(threadargs.hThreadExit);

			// End the dialog.
			EndDialog(hwndDlg, combo.get());
			return TRUE;

	default:
		break;
	}

	return FALSE;
}

/**
* Checks whether a key should be shown in the current filter mode.
*
* @param mapn Index of the key.
* @param filter Current filter.
* @param conflictTable A populated conflict table.
*
* @return Key should be shown (true) or not shown (false)
**/
bool ShouldDisplayMapping(int mapn, int filter, const int* conflictTable)
{
	//mbg merge 7/17/06 changed to if..elseif
	if(filter == 0) /* No filter */
	{
		return true;
	}
	else if(filter <= EMUCMDTYPE_MAX) /* Filter by type */
	{
		return FCEUI_CommandTable[mapn].type == filter - 1;
	}
	else if(filter == EMUCMDTYPE_MAX + 1) /* Assigned */
	{
		return (!GetCommandKeyCombo(FCEUI_CommandTable[mapn].cmd).isEmpty());
	}
	else if(filter == EMUCMDTYPE_MAX + 2) /* Unassigned */
	{
		return GetCommandKeyCombo(FCEUI_CommandTable[mapn].cmd).isEmpty();
	}
	else if(filter == EMUCMDTYPE_MAX + 3) /* Conflicts */
	{
		return conflictTable[FCEUI_CommandTable[mapn].cmd] != 0;
	}
	else 
	{
		return 0;
	}
}

/**
* Populates a conflict table.
*
* @param conflictTable The conflict table to populate.
**/
void PopulateConflictTable(int* conflictTable)
{
	// Check whether there are conflicts between the
	// selected hotkeys.
	for(unsigned i = 0; i < EMUCMD_MAX - 1; ++i)
	{
		for(unsigned int j = i + 1; j < EMUCMD_MAX; ++j)
		{
			if(!GetCommandKeyCombo((EMUCMD)i).isEmpty() &&
				GetCommandKeyCombo((EMUCMD)i).get() == GetCommandKeyCombo((EMUCMD)j).get() &&
				 // AnS: added the condition that both commands must have the same EMUCMDFLAG_TASEDITOR, or else they are not considered conflicting
				 (FCEUI_CommandTable[i].flags & EMUCMDFLAG_TASEDITOR) == (FCEUI_CommandTable[j].flags & EMUCMDFLAG_TASEDITOR))
			{
				conflictTable[i] = 1;
				conflictTable[j] = 1;
			}
		}
	}
}

/**
* Populates the hotkey ListView according to the rules set
* by the current filter.
*
* @param hwndDlg Handle of the Map Input dialog.
**/
void PopulateMappingDisplay(HWND hwndDlg)
{
	// Get the ListView handle.
	HWND hwndListView = GetDlgItem(hwndDlg, LV_MAPPING);

	// Get the number of items in the ListView.
	int num = SendMessage(hwndListView, LVM_GETITEMCOUNT, 0, 0);

	// Get the selected filter.
	int filter = (int)SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_GETCURSEL, 0, 0);

	int* conflictTable = 0;

	// Get the filter type.
	if (filter == EMUCMDTYPE_MAX + 3)
	{
		// Set up the conflict table.
		conflictTable = (int*)malloc(sizeof(int)*EMUCMD_MAX);
		memset(conflictTable, 0, sizeof(int)*EMUCMD_MAX);

		PopulateConflictTable(conflictTable);
	}

	LVITEM lvi;


	int newItemCount = 0;

	// Populate display.
	for(int i = 0, idx = 0; i < EMUCMD_MAX; ++i)
	{
		// Check if the current key should be displayed
		// according to the current filter.
		if(ShouldDisplayMapping(i, filter, conflictTable))
		{
			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iItem = idx;
			lvi.iSubItem = 0;
			lvi.pszText = (char*)FCEUI_CommandTypeNames[FCEUI_CommandTable[i].type];
			lvi.lParam = (LPARAM)FCEUI_CommandTable[i].cmd;

			if(newItemCount<num)
				SendMessage(hwndListView, LVM_SETITEM, (WPARAM)0, (LPARAM)&lvi);
			else
				SendMessage(hwndListView, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi);

			newItemCount++;

			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT;
			lvi.iItem = idx;
			lvi.iSubItem = 1;
			lvi.pszText = FCEUI_CommandTable[i].name;

			SendMessage(hwndListView, LVM_SETITEM, (WPARAM)0, (LPARAM)&lvi);

			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT;
			lvi.iItem = idx;
			lvi.iSubItem = 2;
			lvi.pszText = GetKeyComboName(GetCommandKeyCombo(FCEUI_CommandTable[i].cmd));

			SendMessage(hwndListView, LVM_SETITEM, (WPARAM)0, (LPARAM)&lvi);

			idx++;
		}
	}

	//delete unneeded items
	for(int i=newItemCount;i<num;i++)
	{
		SendMessage(hwndListView, LVM_DELETEITEM, (WPARAM)(newItemCount), 0);
	}

	if(conflictTable)
	{
		free(conflictTable);
	}
}

/**
* Initializes the ListView control that shows the key mappings.
*
* @param The handle of the Map Input dialog.
*
* @return The handle of the ListView control.
**/
HWND InitializeListView(HWND hwndDlg)
{
	HWND hwndListView = GetDlgItem(hwndDlg, LV_MAPPING);
	LVCOLUMN lv;

	// Set full row select.
	SendMessage(hwndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	// Init ListView columns.
	memset(&lv, 0, sizeof(lv));
	lv.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lv.fmt = LVCFMT_LEFT;
	lv.pszText = "Type";
	lv.cx = 80;

	SendMessage(hwndListView, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&lv);

	memset(&lv, 0, sizeof(lv));
	lv.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lv.fmt = LVCFMT_LEFT;
	lv.pszText = "Command";
	lv.cx = 240;

	SendMessage(hwndListView, LVM_INSERTCOLUMN, (WPARAM)1, (LPARAM)&lv);

	memset(&lv, 0, sizeof(lv));
	lv.mask = LVCF_FMT | LVCF_TEXT;
	lv.fmt = LVCFMT_LEFT;
	lv.pszText = "Input";

	SendMessage(hwndListView, LVM_INSERTCOLUMN, (WPARAM)2, (LPARAM)&lv);

	return hwndListView;
}

/**
* Fills the filter combobox.
*
* @param hwndDlg Handle of the Map Input dialog.
**/
void InitFilterComboBox(HWND hwndDlg)
{
	// Populate the filter combobox.
	SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_INSERTSTRING, 0, (LPARAM)"None");

	unsigned i;

	for(i = 0; i < EMUCMDTYPE_MAX; ++i)
	{
		SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_INSERTSTRING, i+1, (LPARAM)FCEUI_CommandTypeNames[i]);
	}

	SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_INSERTSTRING, ++i, (LPARAM)"Assigned");
	SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_INSERTSTRING, ++i, (LPARAM)"Unassigned");
	SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_INSERTSTRING, ++i, (LPARAM)"Conflicts");

	// Default filter is "none".
	SendDlgItemMessage(hwndDlg, COMBO_FILTER, CB_SETCURSEL, 0, 0);
}

/**
* Checks what ListView line was selected and shows the dialog
* that prompts the user to enter a hotkey.
**/
void AskForHotkey(HWND hwndListView)
{
	int nSel = SendMessage(hwndListView, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);

	if (nSel != -1)
	{
		LVITEM lvi;

		// Get the corresponding input
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask = LVIF_PARAM;
		lvi.iItem = nSel;
		lvi.iSubItem = 0;

		SendMessage(hwndListView, LVM_GETITEM, 0, (LPARAM)&lvi);

		EMUCMD nCmd = (EMUCMD)lvi.lParam;

		int nRet = DialogBox(fceu_hInstance, "NEWINPUT", hwndListView, ChangeInputDialogProc);
		
		if (nRet)
		{
			if(nRet < 0) nRet = 0; // nRet will be -1 when the user selects "clear".
			SetCommandKeyCombo(nCmd, KeyCombo(nRet));

			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT;
			lvi.iItem = nSel;
			lvi.iSubItem = 2;
			lvi.pszText = GetKeyComboName(GetCommandKeyCombo(nCmd));
			SendMessage(hwndListView, LVM_SETITEM, (WPARAM)0, (LPARAM)&lvi);
		}
	}
}

/**
* Restores the default hotkey mapping.
**/
void ApplyDefaultCommandMapping()
{
	for(unsigned int i = 0; i < EMUCMD_MAX; ++i) {
		SetCommandKeyCombo((EMUCMD)i, KeyCombo(0));
	}

	for(unsigned i = 0; i < NUM_DEFAULT_MAPPINGS; ++i)
	{
		SetCommandKeyCombo(DefaultCommandMapping[i].cmd, KeyCombo(DefaultCommandMapping[i].key));
	}
}

/**
* Callback function of the Map Input dialog
**/
BOOL CALLBACK MapInputDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			HWND hwndListView = InitializeListView(hwndDlg);

			InitFilterComboBox(hwndDlg);

			// Now populate the mapping display.
			PopulateMappingDisplay(hwndDlg);

			// Autosize last column.
			SendMessage(hwndListView, LVM_SETCOLUMNWIDTH, (WPARAM)2, MAKELPARAM(LVSCW_AUTOSIZE_USEHEADER, 0));

			CenterWindowOnScreen(hwndDlg);
		}

		return FALSE;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE)
		{
			PopulateMappingDisplay(hwndDlg);
			break;
		}
		
		switch(LOWORD(wParam))
		{
			case IDOK:
				UpdateMenuHotkeys();
				EndDialog(hwndDlg, 1);
				return TRUE;

			case BTN_CANCEL:  // here true cause of ESC button handling as EXIT ;)
				EndDialog(hwndDlg, 0);
				return TRUE;

			case BTN_RESTORE_DEFAULTS:
				ApplyDefaultCommandMapping();
				PopulateMappingDisplay(hwndDlg);
				return TRUE;

			default:
				break;
		}
		break;
		
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
			EndDialog(hwndDlg, 0);
			break;
	case WM_NOTIFY:

		switch(LOWORD(wParam))
		{
			case LV_MAPPING:
				if (lParam)
				{
					NMHDR* pnm = (NMHDR*)lParam;

					if (pnm->code == LVN_ITEMACTIVATE)
					{
						HWND hwndListView = GetDlgItem(hwndDlg, LV_MAPPING);

						AskForHotkey(hwndListView);

						// TODO: Only redraw if Conflicts filter
						// is active.
						PopulateMappingDisplay(hwndDlg);
					}

					return TRUE;
				}
				break;

			default:
				break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

/**
* Show input mapping configuration dialog.
**/
void MapInput(void)
{
	// Make a backup of the current mappings, in case the user changes their mind.
	int backupmapping[EMUCMD_MAX];
	for(unsigned int i = 0; i < EMUCMD_MAX; ++i) {
		backupmapping[i] = GetCommandKeyCombo((EMUCMD)i).get();
	}

	if(!DialogBox(fceu_hInstance, "MAPINPUT", hAppWnd, MapInputDialogProc))
	{
		for(unsigned int i = 0; i < EMUCMD_MAX; ++i) {
			SetCommandKeyCombo((EMUCMD)i, KeyCombo(backupmapping[i]));
		}
	}
}
