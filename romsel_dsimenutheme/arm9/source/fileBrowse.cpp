/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include "fileBrowse.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>
#include <math.h>

#include <nds.h>
#include <maxmod9.h>
#include "common/gl2d.h"

#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/iconHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "graphics/graphics.h"
#include "SwitchState.h"
#include "perGameSettings.h"
#include "graphics/ThemeTextures.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "inifile.h"
#include "flashcard.h"

#include "fileCopy.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "graphics/queueControl.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

extern bool whiteScreen;
extern bool fadeType;
extern bool fadeSpeed;
extern bool controlTopBright;
extern bool controlBottomBright;

extern std::string romPath;

extern int launchType;
extern bool slot1LaunchMethod;
extern bool useBootstrap;
extern bool bootstrapFile;
extern bool homebrewBootstrap;
extern bool gbaBiosFound[2];
extern bool useGbarunner;
extern bool arm7SCFGLocked;
extern int consoleModel;
extern bool isRegularDS;
extern int launcherApp;
extern int sysRegion;
extern bool snesEmulator;

extern const char *unlaunchAutoLoadID;

extern bool dropDown;
extern bool redoDropDown;
extern bool showbubble;
extern bool showSTARTborder;
extern bool buttonArrowTouched[2];
extern bool scrollWindowTouched;

extern bool titleboxXmoveleft;
extern bool titleboxXmoveright;

extern bool applaunchprep;

extern touchPosition touch;

extern bool showdialogbox;
extern bool dbox_showIcon;
extern bool dbox_selectMenu;

extern std::string romfolder[2];

extern bool applaunch;

extern bool gotosettings;

extern int vblankRefreshCounter;
using namespace std;

extern bool startMenu;

extern int theme;

int file_count = 0;

extern bool showDirectories;
extern bool showHidden;
extern bool showBoxArt;
extern int spawnedtitleboxes;
extern int cursorPosition[2];
extern int startMenu_cursorPosition;
extern int pagenum[2];
extern int titleboxXpos[2];
extern int titlewindowXpos[2];
int movingApp = -1;
int movingAppYpos = 0;
bool movingAppIsDir = false;
extern bool showMovingArrow;
extern double movingArrowYpos;

extern bool showLshoulder;
extern bool showRshoulder;

extern bool showProgressIcon;

char boxArtPath[40][256];

bool boxArtLoaded = false;
bool shouldersRendered = false;
bool settingsChanged = false;

bool isScrolling = false;
bool needToPlayStopSound = false;
bool stopSoundPlayed = false;
int waitForNeedToPlayStopSound = 0;

extern void SaveSettings();

extern std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

extern void loadGameOnFlashcard (const char* ndsPath, std::string filename, bool usePerGameSettings);
extern void dsCardLaunch();
extern void unlaunchSetHiyaBoot();

mm_sound_effect snd_launch;
mm_sound_effect snd_select;
mm_sound_effect snd_stop;
mm_sound_effect snd_wrong;
mm_sound_effect snd_back;
mm_sound_effect snd_switch;
mm_sound_effect mus_startup;
mm_sound_effect mus_menu;

void InitSound() {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect( SFX_LAUNCH );
	mmLoadEffect( SFX_SELECT );
	mmLoadEffect( SFX_STOP );
	mmLoadEffect( SFX_WRONG );
	mmLoadEffect( SFX_BACK );
	mmLoadEffect( SFX_SWITCH );
	mmLoadEffect( SFX_STARTUP );
	mmLoadEffect( SFX_MENU );

	snd_launch = {
		{ SFX_LAUNCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_select = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_stop = {
		{ SFX_STOP } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_wrong = {
		{ SFX_WRONG } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_back = {
		{ SFX_BACK } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	snd_switch = {
		{ SFX_SWITCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	mus_startup = {
		{ SFX_STARTUP } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	mus_menu = {
		{ SFX_MENU } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
}

extern bool music;

extern char usernameRendered[11];
extern bool usernameRenderedDone;

const char *gameOrderIniPath;
const char *hiddenGamesIniPath;

struct DirEntry
{
	string name;
	bool isDirectory;
};

TextEntry *pathText = nullptr;
char *path = new char[PATH_MAX];

#ifdef EMULATE_FILES
#define chdir(a) chdirFake(a)
void chdirFake(const char *dir)
{
	string pathStr(path);
	string dirStr(dir);
	if (dirStr == "..")
	{
		pathStr.resize(pathStr.find_last_of("/"));
		pathStr.resize(pathStr.find_last_of("/") + 1);
	}
	else
	{
		pathStr += dirStr;
		pathStr += "/";
	}
	strcpy(path, pathStr.c_str());
}
#endif

bool nameEndsWith(const string& name, const vector<string> extensionList)
{

	if (name.substr(0,2) == "._")	return false;	// Don't show macOS's index files

	if (name.size() == 0) return false;

	if (extensionList.size() == 0) return true;

	for (int i = 0; i < (int) extensionList.size(); i++)
	{
		const string ext = extensionList.at(i);
		if (strcasecmp(name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate(const DirEntry& lhs, const DirEntry& rhs)
{

	if (!lhs.isDirectory && rhs.isDirectory)
	{
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory)
	{
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents(vector<DirEntry>& dirContents, const vector<string> extensionList)
{

	dirContents.clear();

	file_count = 0;
	
	struct stat st;
	DIR *pdir = opendir(".");

	if (pdir == NULL)
	{
		// iprintf("Unable to open the directory.\n");
		printSmall (false, 4, 4, "Unable to open the directory.");
	}
	else
	{
		CIniFile hiddenGamesIni(hiddenGamesIniPath);
		vector<std::string> hiddenGames;
		char str[11];

		for(int i=0;true;i++) {
			sprintf(str, "%d", i);
			if(hiddenGamesIni.GetString(getcwd(path, PATH_MAX), str, "") != "") {
				hiddenGames.push_back(hiddenGamesIni.GetString(getcwd(path, PATH_MAX), str, ""));
			} else {
				break;
			}
		}

		while (true)
		{
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if (pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if (showDirectories) {
				if (dirEntry.name.compare(".") != 0 && dirEntry.name.compare("_nds") && dirEntry.name.compare("saves") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
					bool isHidden = false;
					for(int i=0;i<(int)hiddenGames.size();i++) {
						if(dirEntry.name == hiddenGames[i]) {
							isHidden = true;
							break;
						}
					}
					if(!isHidden || showHidden) {
						dirContents.push_back(dirEntry);
						file_count++;
					}
				}
			} else {
				if (dirEntry.name.compare(".") != 0 && (nameEndsWith(dirEntry.name, extensionList))) {
					bool isHidden = false;
					for(int i=0;i<(int)hiddenGames.size();i++) {
						if(dirEntry.name == hiddenGames[i]) {
							isHidden = true;
							break;
						}
					}
					if(!isHidden || showHidden) {
						dirContents.push_back(dirEntry);
						file_count++;
					}
				}
			}

			loadVolumeImage();
			loadBatteryImage();
			loadTime();
			loadDate();
			loadClockColon();
		}
		sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);

		CIniFile gameOrderIni(gameOrderIniPath);
		vector<std::string> gameOrder;

		for(int i=0;i<(int)dirContents.size();i++) {
			sprintf(str, "%d", i);
			gameOrder.push_back(gameOrderIni.GetString(getcwd(path, PATH_MAX), str, "NULL"));
		}

		for(int i=0;i<(int)gameOrder.size();i++) {
			for(int j=0;j<=(int)dirContents.size();j++) {
				if(gameOrder[i] == dirContents[j].name) {
					vector<DirEntry> dirContentsCopy;
					dirContentsCopy.push_back(dirContents[j]);
					dirContents.erase(dirContents.begin()+j);
					dirContents.insert(dirContents.begin()+i, dirContentsCopy[0]);
					break;
				}
			}
		}
		closedir(pdir);
	}
}

void getDirectoryContents(vector<DirEntry>& dirContents)
{
	vector<string> extensionList;
	getDirectoryContents(dirContents, extensionList);
}

void updatePath()
{
#ifndef EMULATE_FILES
	getcwd(path, PATH_MAX);
#else
	if (strlen(path) < 1)
	{
		path[0] = '/';
		path[1] = '\0';
	}
#endif
	if (pathText == nullptr)
	{
		printLarge(false, 2 * FONT_SX, 1 * FONT_SY, path);
		pathText = getPreviousTextEntry(false);
		pathText->anim = TextEntry::AnimType::IN;
		pathText->fade = TextEntry::FadeType::NONE;
		pathText->y = 100;
		pathText->immune = true;
	}

	int pathWidth = calcLargeFontWidth(path);
	pathText->delay = TextEntry::ACTIVE;
	pathText->finalX = min(2 * FONT_SX, -(pathWidth + 2 * FONT_SX - 256));
}

bool isTopLevel(const char *path)
{
#ifdef EMULATE_FILES
	return strlen(path) <= strlen("/");
#else
	return strlen(path) <= strlen("fat: /");
#endif
}

void waitForFadeOut (void) {
	if (!dropDown && theme == 0) {
		dropDown = true;
		for (int i = 0; i < 72; i++) {
			loadVolumeImage();
			loadBatteryImage();
			loadTime();
			loadDate();
			loadClockColon();
			swiWaitForVBlank();
		}
	} else {
		for (int i = 0; i < 25; i++) swiWaitForVBlank();
	}
}

bool nowLoadingDisplaying = false;

void displayNowLoading(void) {
	fadeType = true;	// Fade in from white
	printLargeCentered(false, 88, "Now Loading...");
	if (!isRegularDS) {
		printSmall(false, 8, 152, "Location:");
		if (secondaryDevice) {
			printSmall(false, 8, 168, "Slot-1 microSD Card");
		} else if (consoleModel < 3) {
			printSmall(false, 8, 168, "SD Card");
		} else {
			printSmall(false, 8, 168, "microSD Card");
		}
	}
	nowLoadingDisplaying = true;
	reloadFontPalettes();
	while (!screenFadedIn());
	showProgressIcon = true;
	controlTopBright = false;
}

void updateScrollingState(u32 held, u32 pressed) {

	bool isHeld = (held & KEY_LEFT) || (held & KEY_RIGHT);
	bool isPressed = (pressed & KEY_LEFT) || (pressed & KEY_RIGHT);
	
	// If we were scrolling before, but now let go of all keys, stop scrolling.
	 if (isHeld && !isPressed 
	 	&&(
			(cursorPosition[secondaryDevice] != 0 && cursorPosition[secondaryDevice] != 39) 
		)){
		isScrolling = true;
	} else if (!isHeld && !isPressed && !titleboxXmoveleft && !titleboxXmoveright) {
		isScrolling = false;
	} 

}

static bool inSelectMenu = false;

void launchSettings(void) {
	mmEffectEx(&snd_launch);
	controlTopBright = true;
	gotosettings = true;

	fadeType = false;	// Fade to white
	fifoSendValue32(FIFO_USER_01, 1);	// Fade out sound
	for (int i = 0; i < 60; i++) {
		swiWaitForVBlank();
	}
	music = false;
	mmEffectCancelAll();
	fifoSendValue32(FIFO_USER_01, 0);	// Cancel sound fade-out

	SaveSettings();
	// Launch settings
	if (sdFound()) {
		chdir("sd:/");
	}
	int err = runNdsFile ("/_nds/TWiLightMenu/settings.srldr", 0, NULL, false, false, true, true);
	iprintf ("Start failed. Error %i\n", err);
}

void exitToSystemMenu(void) {
	mmEffectEx(&snd_launch);
	controlTopBright = true;

	fadeType = false;	// Fade to white
	fifoSendValue32(FIFO_USER_01, 1);	// Fade out sound
	for (int i = 0; i < 60; i++) {
		swiWaitForVBlank();
	}
	music = false;
	mmEffectCancelAll();
	fifoSendValue32(FIFO_USER_01, 0);	// Cancel sound fade-out

	if (settingsChanged) {
		SaveSettings();
		settingsChanged = false;
	}
	if (!isDSiMode() || launcherApp == -1) {
		*(u32*)(0x02000300) = 0x434E4C54;	// Set "CNLT" warmboot flag
		*(u16*)(0x02000304) = 0x1801;
		*(u32*)(0x02000310) = 0x4D454E55;	// "MENU"
		unlaunchSetHiyaBoot();
	} else {
		u8 setRegion = 0;
		if (sysRegion == -1) {
			// Determine SysNAND region by searching region of System Settings on SDNAND
			char tmdpath[256];
			for (u8 i = 0x41; i <= 0x5A; i++)
			{
				snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
				if (access(tmdpath, F_OK) == 0)
				{
					setRegion = i;
					break;
				}
			}
		} else {
			switch(sysRegion) {
				case 0:
				default:
					setRegion = 0x4A;	// JAP
					break;
				case 1:
					setRegion = 0x45;	// USA
					break;
				case 2:
					setRegion = 0x50;	// EUR
					break;
				case 3:
					setRegion = 0x55;	// AUS
					break;
				case 4:
					setRegion = 0x43;	// CHN
					break;
				case 5:
					setRegion = 0x4B;	// KOR
					break;
			}
		}

		char unlaunchDevicePath[256];
		snprintf(unlaunchDevicePath, sizeof(unlaunchDevicePath), "nand:/title/00030017/484E41%x/content/0000000%i.app", setRegion, launcherApp);

		memcpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
		*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
		*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
		*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
		*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
		*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
		*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
		memset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
		int i2 = 0;
		for (int i = 0; i < (int)sizeof(unlaunchDevicePath); i++) {
			*(u8*)(0x02000838+i2) = unlaunchDevicePath[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
			i2 += 2;
		}
		while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
			*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
		}
	}
	fifoSendValue32(FIFO_USER_02, 1);	// ReturntoDSiMenu
}

void switchDevice(void) {
	if (bothSDandFlashcard())
	{
		mmEffectEx(&snd_switch);
		fadeType = false;	// Fade to white
		for (int i = 0; i < 30; i++) swiWaitForVBlank();
		secondaryDevice = !secondaryDevice;
		if (showBoxArt) clearBoxArt();	// Clear box art
		whiteScreen = true;
		boxArtLoaded = false;
		redoDropDown = true;
		shouldersRendered = false;
		showbubble = false;
		showSTARTborder = false;
		stopSoundPlayed = false;
		clearText();
		SaveSettings();
		settingsChanged = false;
	} else {
		mmEffectEx(&snd_launch);
		controlTopBright = true;

		fadeType = false;	// Fade to white
		fifoSendValue32(FIFO_USER_01, 1);	// Fade out sound
		for (int i = 0; i < 60; i++) {
			swiWaitForVBlank();
		}
		music = false;
		mmEffectCancelAll();
		fifoSendValue32(FIFO_USER_01, 0);	// Cancel sound fade-out

		romPath = "";
		launchType = 0;
		SaveSettings();
		if (!slot1LaunchMethod || arm7SCFGLocked) {
			dsCardLaunch();
		} else {
			if (sdFound()) {
				chdir("sd:/");
			}
			int err = runNdsFile ("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, false, true, true);
			iprintf ("Start failed. Error %i\n", err);
		}
	}
}

void launchGba(void) {
	if (!gbaBiosFound[secondaryDevice] && useGbarunner) {
		mmEffectEx(&snd_wrong);
		clearText();
		dbox_showIcon = false;
		if (!showdialogbox) {
			showdialogbox = true;
			for (int i = 0; i < 30; i++) swiWaitForVBlank();
		}
		printLarge(false, 16, 12, "Error code: BINF");
		printSmallCentered(false, 64, "The GBA BIOS is required");
		printSmallCentered(false, 78, "to run GBA games.");
		printSmallCentered(false, 112, "Place the BIOS on the root");
		printSmallCentered(false, 126, "as \"bios.bin\".");
		printSmall(false, 208, 160, "A: OK");
		int pressed = 0;
		do {
			scanKeys();
			pressed = keysDown();
			loadVolumeImage();
			loadBatteryImage();
			loadDate();
			loadTime();
			loadClockColon();
			swiWaitForVBlank();
		} while (!(pressed & KEY_A));
		clearText();
		if (!inSelectMenu) {
			showdialogbox = false;
			for (int i = 0; i < 15; i++) swiWaitForVBlank();
		}
		return;
	}

	mmEffectEx(&snd_launch);
	controlTopBright = true;

	fadeType = false;	// Fade to white
	fifoSendValue32(FIFO_USER_01, 1);	// Fade out sound
	for (int i = 0; i < 60; i++) {
		swiWaitForVBlank();
	}
	music = false;
	mmEffectCancelAll();
	fifoSendValue32(FIFO_USER_01, 0);	// Cancel sound fade-out

	SaveSettings();
	// Switch to GBA mode
	if (useGbarunner) {
		if (secondaryDevice) {
			if (useBootstrap) {
				int err = runNdsFile ("fat:/_nds/GBARunner2_fc.nds", 0, NULL, true, true, false, false);
				iprintf ("Start failed. Error %i\n", err);
			} else {
				loadGameOnFlashcard("fat:/_nds/GBARunner2_fc.nds", "GBARunner2_fc.nds", false);
			}
		} else {
			CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
			bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/GBARunner2.nds");
			bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
			int err = runNdsFile (bootstrapFile ? "sd:/_nds/nds-bootstrap-gbar2-nightly.nds" : "sd:/_nds/nds-bootstrap-gbar2-release.nds", 0, NULL, true, false, true, true);
			iprintf ("Start failed. Error %i\n", err);
		}
	} else {
		gbaSwitch();
	}
}

void mdRomTooBig(void) {
	//int bottomBright = 0;

	mmEffectEx(&snd_wrong);
	clearText();
	dbox_showIcon = false;
	showdialogbox = true;
	for (int i = 0; i < 30; i++) swiWaitForVBlank();
	printSmallCentered(false, 64, "This Mega Drive or Genesis");
	printSmallCentered(false, 78, "ROM cannot be launched,");
	printSmallCentered(false, 92, "due to the size of it");
	printSmallCentered(false, 106, "being above 3MB.");
	printSmall(false, 208, 160, "A: OK");
	int pressed = 0;
	do {
		scanKeys();
		pressed = keysDown();
		loadVolumeImage();
		loadBatteryImage();
		loadTime();
		loadDate();
		loadClockColon();
		swiWaitForVBlank();

		// Debug code for changing brightness of BG layer

		/*if (pressed & KEY_UP) {
			bottomBright--;
		} else if (pressed & KEY_DOWN) {
			bottomBright++;
		}
		
		if (bottomBright < 0) bottomBright = 0;
		if (bottomBright > 15) bottomBright = 15;

		switch (bottomBright) {
			case 0:
			default:
				REG_BLDY = 0;
				break;
			case 1:
				REG_BLDY = (0b0001 << 1);
				break;
			case 2:
				REG_BLDY = (0b0010 << 1);
				break;
			case 3:
				REG_BLDY = (0b0011 << 1);
				break;
			case 4:
				REG_BLDY = (0b0100 << 1);
				break;
			case 5:
				REG_BLDY = (0b0101 << 1);
				break;
			case 6:
				REG_BLDY = (0b0110 << 1);
				break;
			case 7:
				REG_BLDY = (0b0111 << 1);
				break;
			case 8:
				REG_BLDY = (0b1000 << 1);
				break;
			case 9:
				REG_BLDY = (0b1001 << 1);
				break;
			case 10:
				REG_BLDY = (0b1010 << 1);
				break;
			case 11:
				REG_BLDY = (0b1011 << 1);
				break;
			case 12:
				REG_BLDY = (0b1100 << 1);
				break;
			case 13:
				REG_BLDY = (0b1101 << 1);
				break;
			case 14:
				REG_BLDY = (0b1110 << 1);
				break;
			case 15:
				REG_BLDY = (0b1111 << 1);
				break;
		}*/
	} while (!(pressed & KEY_A));
	clearText();
	showdialogbox = false;
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

bool selectMenu(void) {
	inSelectMenu = true;
	clearText();
	dbox_showIcon = false;
	dbox_selectMenu = true;
	showdialogbox = true;
	int maxCursors = 0;
	int selCursorPosition = 0;
	int assignedOp[4] = {-1};
	int selIconYpos = 96;
	if (isDSiMode() && sdFound()) {
		for (int i = 0; i < 4; i++) {
			selIconYpos -= 14;
		}
		assignedOp[0] = 0;
		assignedOp[1] = 1;
		assignedOp[2] = 2;
		assignedOp[3] = 3;
		maxCursors = 3;
	} else {
		for (int i = 0; i < 3; i++) {
			selIconYpos -= 14;
		}
		if (!isRegularDS) {
			assignedOp[0] = 0;
			assignedOp[1] = 1;
			assignedOp[2] = 3;
			maxCursors = 2;
		} else {
			assignedOp[0] = 1;
			assignedOp[1] = 3;
			maxCursors = 1;
		}
	}
	for (int i = 0; i < 30; i++) swiWaitForVBlank();
	int pressed = 0;
	while (1) {
		int textYpos = selIconYpos+4;
		clearText();
		printSmallCentered(false, 16, "SELECT menu");
		printSmall(false, 24, -2+textYpos+(28*selCursorPosition), ">");
		for (int i = 0; i <= maxCursors; i++) {
			if (assignedOp[i] == 0) {
				printSmall(false, 64, textYpos, (consoleModel < 2) ? "DSi Menu" : "3DS HOME Menu");
			} else if (assignedOp[i] == 1) {
				printSmall(false, 64, textYpos, "TWLMenu++ Settings");
			} else if (assignedOp[i] == 2) {
				if (bothSDandFlashcard()) {
					if (secondaryDevice) {
						if (consoleModel < 3) {
							printSmall(false, 64, textYpos, "Switch to SD Card");
						} else {
							printSmall(false, 64, textYpos, "Switch to microSD Card");
						}
					} else {
						printSmall(false, 64, textYpos, "Switch to Slot-1 microSD");
					}
				} else {
					printSmall(false, 64, textYpos, (REG_SCFG_MC == 0x11) ? "No Slot-1 card inserted" : "Launch Slot-1 card");
				}
			} else if (assignedOp[i] == 3) {
				printSmall(false, 64, textYpos, useGbarunner ? "Start GBARunner2" : "Start GBA Mode");
			}
			textYpos += 28;
		}
		printSmallCentered(false, 160, "SELECT/B: Back, A: Select");
		scanKeys();
		pressed = keysDown();
		loadVolumeImage();
		loadBatteryImage();
		swiWaitForVBlank();
		if (pressed & KEY_UP) {
			selCursorPosition--;
			if (selCursorPosition < 0) selCursorPosition = maxCursors;
		}
		if (pressed & KEY_DOWN) {
			selCursorPosition++;
			if (selCursorPosition > maxCursors) selCursorPosition = 0;
		}
		if (pressed & KEY_A) {
			switch (assignedOp[selCursorPosition]) {
				case 0:
				default:
					exitToSystemMenu();
					break;
				case 1:
					launchSettings();
					break;
				case 2:
					if (REG_SCFG_MC != 0x11) {
						switchDevice();
						inSelectMenu = false;
						return true;
					} else {
						mmEffectEx(&snd_wrong);
					}
					break;
				case 3:
					launchGba();
					break;
			}
		}
		if ((pressed & KEY_B) || (pressed & KEY_SELECT)) {
			break;
		}
	};
	clearText();
	showdialogbox = false;
	dbox_selectMenu = false;
	inSelectMenu = false;
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
	return false;
}

void getFileInfo(SwitchState scrn, vector<DirEntry> dirContents[], bool reSpawnBoxes) {
	if (reSpawnBoxes) spawnedtitleboxes = 0;
	for(int i = 0; i < 40; i++) {
		if (i+pagenum[secondaryDevice]*40 < file_count) {
			if (dirContents[scrn].at(i+pagenum[secondaryDevice]*40).isDirectory) {
				isDirectory[i] = true;
				bnrWirelessIcon[i] = 0;
			} else {
				isDirectory[i] = false;
				std::string std_romsel_filename = dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str();
				if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nds")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "NDS")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "dsi")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "DSI")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "ids")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "IDS")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "app")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "APP")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv")
				|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "ARGV"))
				{
					getGameInfo(isDirectory[i], dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str(), i);
					bnrRomType[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gb")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GB")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sgb")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "SGB"))
				{
					bnrRomType[i] = 1;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gbc")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GBC"))
				{
					bnrRomType[i] = 2;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "nes")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "NES")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "fds")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "FDS"))
				{
					bnrRomType[i] = 3;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sms")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "SMS"))
				{
					bnrRomType[i] = 4;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gg")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GG"))
				{
					bnrRomType[i] = 5;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "gen")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "GEN"))
				{
					bnrRomType[i] = 6;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				} else if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "smc")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "SMC")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "sfc")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "SFC"))
				{
					bnrRomType[i] = 7;
					bnrWirelessIcon[i] = 0;
					isDSiWare[i] = false;
					isHomebrew[i] = 0;
				}

				if (showBoxArt) {
					// Store box art path
					snprintf (boxArtPath[i], sizeof(boxArtPath[i]), (sdFound() ? "sd:/_nds/TWiLightMenu/boxart/%s.bmp" : "fat:/_nds/TWiLightMenu/boxart/%s.bmp"), dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str());
					if (!access(boxArtPath[i], F_OK)) {
					} else if (bnrRomType[i] == 0) {
						if((std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "argv")
						|| (std_romsel_filename.substr(std_romsel_filename.find_last_of(".") + 1) == "ARGV"))
						{
							vector<char*> argarray;

							FILE *argfile = fopen(std_romsel_filename.c_str(),"rb");
								char str[PATH_MAX], *pstr;
							const char seps[]= "\n\r\t ";

							while( fgets(str, PATH_MAX, argfile) ) {
								// Find comment and end string there
								if( (pstr = strchr(str, '#')) )
									*pstr= '\0';

								// Tokenize arguments
								pstr= strtok(str, seps);

								while( pstr != NULL ) {
									argarray.push_back(strdup(pstr));
									pstr= strtok(NULL, seps);
								}
							}
							fclose(argfile);
							std_romsel_filename = argarray.at(0);
						}
						// Get game's TID
						FILE *f_nds_file = fopen(std_romsel_filename.c_str(), "rb");
						char game_TID[5];
						grabTID(f_nds_file, game_TID);
						game_TID[4] = 0;
						fclose(f_nds_file);

						snprintf (boxArtPath[i], sizeof(boxArtPath[i]), (sdFound() ? "sd:/_nds/TWiLightMenu/boxart/%s.bmp" : "fat:/_nds/TWiLightMenu/boxart/%s.bmp"), game_TID);
					}
				}
			}
			if (reSpawnBoxes) spawnedtitleboxes++;

			loadVolumeImage();
			loadBatteryImage();
			loadTime();
			loadDate();
			loadClockColon();
		}
	}
	if (nowLoadingDisplaying) {
		showProgressIcon = false;
		fadeType = false;	// Fade to white
	}
	// Load correct icons depending on cursor position
	if (cursorPosition[secondaryDevice] <= 1) {
		for(int i = 0; i < 5; i++) {
			if (bnrRomType[i] == 0 && i+pagenum[secondaryDevice]*40 < file_count) {
				iconUpdate(dirContents[scrn].at(i+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str(), i);
			}
		}
	} else if (cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
		for(int i = 0; i < 6; i++) {
			if (bnrRomType[i] == 0 && (cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40 < file_count) {
				iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2+i);
			}
		}
	} else if (cursorPosition[secondaryDevice] >= 37 && cursorPosition[secondaryDevice] <= 39) {
		for(int i = 0; i < 5; i++) {
			if (bnrRomType[i] == 0 && (35+i)+pagenum[secondaryDevice]*40 < file_count) {
				iconUpdate(dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).name.c_str(), 35+i);
			}
		}
	}
}

string browseForFile(const vector<string> extensionList, const char* username)
{
	displayNowLoading();

	gameOrderIniPath = sdFound() ? "sd:/_nds/TWiLightMenu/extras/gameorder.ini" : "fat:/_nds/TWiLightMenu/extras/gameorder.ini";
	hiddenGamesIniPath = sdFound() ? "sd:/_nds/TWiLightMenu/extras/hiddengames.ini" : "fat:/_nds/TWiLightMenu/extras/hiddengames.ini";

	int pressed = 0;
	int held = 0;
	SwitchState scrn(3);
	vector<DirEntry> dirContents[scrn.SIZE];

	getDirectoryContents(dirContents[scrn], extensionList);

	while (1) {
		getFileInfo(scrn, dirContents, true);

		while (!screenFadedOut());
		nowLoadingDisplaying = false;
		whiteScreen = false;
		fadeType = true;	// Fade in from white
		for (int i = 0; i < 5; i++) swiWaitForVBlank();
		reloadIconPalettes();
		reloadFontPalettes();
		clearText(false);
		waitForFadeOut();
		bool gameTapped = false;

		/* clearText(false);
		updatePath();
		TextPane *pane = &createTextPane(20, 3 + ENTRIES_START_ROW*FONT_SY, ENTRIES_PER_SCREEN);
		for (auto &i : dirContents[scrn])
			pane->addLine(i.visibleName.c_str());
		pane->createDefaultEntries();
		pane->slideTransition(true);

		printSmall(false, 12 - 16, 4 + 10 * (cursorPosition[secondaryDevice] - screenOffset + ENTRIES_START_ROW), ">");
		TextEntry *cursor = getPreviousTextEntry(false);
		cursor->fade = TextEntry::FadeType::IN;
		cursor->finalX += 16; */

		while (1)
		{
			// cursor->finalY = 4 + 10 * (cursorPosition[secondaryDevice] - screenOffset + ENTRIES_START_ROW);
			// cursor->delay = TextEntry::ACTIVE;

			if (cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 > ((int) dirContents[scrn].size() - 1)) {
				if (!boxArtLoaded && showBoxArt) {
					clearBoxArt();	// Clear box art
					boxArtLoaded = true;
				}
				showbubble = false;
				showSTARTborder = (theme == 1 ? true : false);
				clearText(false);	// Clear title
			} else {
				if (!boxArtLoaded && showBoxArt) {
					if (isDirectory[cursorPosition[secondaryDevice]]) {
						clearBoxArt();	// Clear box art, if it's a directory
					} else {
						loadBoxArt(boxArtPath[cursorPosition[secondaryDevice]]);	// Load box art
					}
					boxArtLoaded = true;
				}
				showbubble = true;
				showSTARTborder = true;
				titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
			}

			if (!stopSoundPlayed) {
				if ((theme == 0 && !startMenu && cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 <= ((int) dirContents[scrn].size() - 1))
				|| (theme == 0 && startMenu && startMenu_cursorPosition < (3-flashcardFound()))) {
					needToPlayStopSound = true;
				}
				stopSoundPlayed = true;
			}

			if (!shouldersRendered) {
				showLshoulder = false;
				showRshoulder = false;
				if (startMenu) {
				} else {
					if (pagenum[secondaryDevice] != 0) {
						showLshoulder = true;
					}
					if (file_count > 40+pagenum[secondaryDevice]*40) {
						showRshoulder = true;
					}
				}
				loadShoulders();
				shouldersRendered = true;
			}

			u8 current_SCFG_MC = REG_SCFG_MC;

			// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
			do
			{
				scanKeys();
				pressed = keysDown();
				held = keysDownRepeat();
				touchRead(&touch);
				updateScrollingState(held, pressed);
				if (!isScrolling) {
					buttonArrowTouched[0] = false;
					buttonArrowTouched[1] = false;
				}
				loadVolumeImage();
				loadBatteryImage();
				loadTime();
				loadDate();
				loadClockColon();
				swiWaitForVBlank();
				if (REG_SCFG_MC != current_SCFG_MC) {
					break;
				}
			}
			while (!pressed && !held);
			if (((pressed & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright)
			|| ((held & KEY_LEFT) && !titleboxXmoveleft && !titleboxXmoveright)
			|| ((pressed & KEY_TOUCH) && touch.py > 171 && touch.px < 19 && theme == 0 && !titleboxXmoveleft && !titleboxXmoveright))		// Button arrow (DSi theme)
			{
				cursorPosition[secondaryDevice] -= 1;
				if (cursorPosition[secondaryDevice] >= 0) {
					if (pressed & KEY_TOUCH) buttonArrowTouched[0] = true;
					titleboxXmoveleft = true;
					waitForNeedToPlayStopSound = 1;
					mmEffectEx(&snd_select);
					boxArtLoaded = false;
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
					if (bnrRomType[cursorPosition[secondaryDevice]-2] == 0 && (cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40 < file_count) {
						iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2);
						defer(reloadFontTextures);
					}
				}
			} else if (((pressed & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright)
					|| ((held & KEY_RIGHT) && !titleboxXmoveleft && !titleboxXmoveright)
					|| ((pressed & KEY_TOUCH) && touch.py > 171 && touch.px > 236 && theme == 0 && !titleboxXmoveleft && !titleboxXmoveright))		// Button arrow (DSi theme)
			{
				cursorPosition[secondaryDevice] += 1;
				if (cursorPosition[secondaryDevice] <= 39) {
					if (pressed & KEY_TOUCH) buttonArrowTouched[1] = true;
					titleboxXmoveright = true;
					waitForNeedToPlayStopSound = 1;
					mmEffectEx(&snd_select);
					boxArtLoaded = false;
					settingsChanged = true;
				} else {
					mmEffectEx(&snd_wrong);
				}
				if(cursorPosition[secondaryDevice] >= 3 && cursorPosition[secondaryDevice] <= 37) {
					if (bnrRomType[cursorPosition[secondaryDevice]+2] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count) {
						iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]+2);
						defer(reloadFontTextures);
					}
				}
			// Move apps
			} else if ((pressed & KEY_UP) && !titleboxXmoveleft && !titleboxXmoveright
			&& cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 < ((int) dirContents[scrn].size()))
			{
				showSTARTborder = false;
				showbubble = false;
				clearText();
				mkdir (sdFound() ? "sd:/_nds/TWiLightMenu/extras" : "fat:/_nds/TWiLightMenu/extras", 0777);
				std::string gameBeingMoved = dirContents[scrn].at((pagenum[secondaryDevice]*40)+(cursorPosition[secondaryDevice])).name;
				movingApp = (pagenum[secondaryDevice]*40)+(cursorPosition[secondaryDevice]);
				if(dirContents[scrn][movingApp].isDirectory)	movingAppIsDir = true;
				else	movingAppIsDir = false;
				iconUpdate(dirContents[scrn].at(movingApp).isDirectory, dirContents[scrn].at(movingApp).name.c_str(), -1);
				getGameInfo(dirContents[scrn].at(movingApp).isDirectory, dirContents[scrn].at(movingApp).name.c_str(), -1);
				for(int i=0;i<10;i++) {
					if (i == 9) {
						movingAppYpos += 2;
					} else {
						movingAppYpos += 8;
					}
					swiWaitForVBlank();
				}
				int orgCursorPosition = cursorPosition[secondaryDevice];
				int orgPage = pagenum[secondaryDevice];
				showMovingArrow = true;

				while(1){
					scanKeys();
					pressed = keysDown();
					held = keysDownRepeat();
					loadVolumeImage();
					loadBatteryImage();
					loadTime();
					loadDate();
					loadClockColon();
					swiWaitForVBlank(); 

					if((pressed & KEY_LEFT && !titleboxXmoveleft && !titleboxXmoveright)
					|| (held & KEY_LEFT && !titleboxXmoveleft && !titleboxXmoveright))
					{
						if(cursorPosition[secondaryDevice] > 0) {
							mmEffectEx(&snd_select);
							titleboxXmoveleft = true;
							cursorPosition[secondaryDevice]--;
							if (bnrRomType[cursorPosition[secondaryDevice]+2] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count && cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2);
								defer(reloadFontTextures);
							}
						} else {
							mmEffectEx(&snd_wrong);
						}
					}
					else if((pressed & KEY_RIGHT && !titleboxXmoveleft && !titleboxXmoveright)
					|| (held & KEY_RIGHT && !titleboxXmoveleft && !titleboxXmoveright))
					{
						if(cursorPosition[secondaryDevice]+(pagenum[secondaryDevice]*40)<(int)dirContents[scrn].size()-1 && cursorPosition[secondaryDevice] < 39) {
							mmEffectEx(&snd_select);
							titleboxXmoveright = true;
							cursorPosition[secondaryDevice]++;
							if (bnrRomType[cursorPosition[secondaryDevice]+2] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count && cursorPosition[secondaryDevice] >= 3 && cursorPosition[secondaryDevice] <= 37) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]+2);
								defer(reloadFontTextures);
							}
						} else {
							mmEffectEx(&snd_wrong);
						}
					} else if(pressed & KEY_DOWN) {
						for(int i=0;i<10;i++) {
							showMovingArrow = false;
							movingArrowYpos = 59;
							if (i == 9) {
								movingAppYpos -= 2;
							} else {
								movingAppYpos -= 8;
							}
							swiWaitForVBlank();
						}
						break;
					} else if(pressed & KEY_L) {
						if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && pagenum[secondaryDevice] != 0) {
							mmEffectEx(&snd_switch);
							fadeType = false;	// Fade to white
							for (int i = 0; i < 30; i++) swiWaitForVBlank();
							pagenum[secondaryDevice] -= 1;
							cursorPosition[secondaryDevice] = 0;
							titleboxXpos[secondaryDevice] = 0;
							titlewindowXpos[secondaryDevice] = 0;
							whiteScreen = true;
							shouldersRendered = false;
							displayNowLoading();
							getDirectoryContents(dirContents[scrn], extensionList);
							getFileInfo(scrn, dirContents, true);

							while (!screenFadedOut());
							nowLoadingDisplaying = false;
							whiteScreen = false;
							fadeType = true;	// Fade in from white
							for (int i = 0; i < 5; i++) swiWaitForVBlank();
							reloadIconPalettes();
							reloadFontPalettes();
							clearText();
						} else {
							mmEffectEx(&snd_wrong);
						}
					} else if(pressed & KEY_R) {
						if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && file_count > 40+pagenum[secondaryDevice]*40) {
							mmEffectEx(&snd_switch);
							fadeType = false;	// Fade to white
							for (int i = 0; i < 30; i++) swiWaitForVBlank();
							pagenum[secondaryDevice] += 1;
							cursorPosition[secondaryDevice] = 0;
							titleboxXpos[secondaryDevice] = 0;
							titlewindowXpos[secondaryDevice] = 0;
							whiteScreen = true;
							shouldersRendered = false;
							displayNowLoading();
							getDirectoryContents(dirContents[scrn], extensionList);
							getFileInfo(scrn, dirContents, true);

							while (!screenFadedOut());
							nowLoadingDisplaying = false;
							whiteScreen = false;
							fadeType = true;	// Fade in from white
							for (int i = 0; i < 5; i++) swiWaitForVBlank();
							reloadIconPalettes();
							reloadFontPalettes();
							clearText();
						} else {
							mmEffectEx(&snd_wrong);
						}
					}
				}
				if ((pagenum[secondaryDevice] != orgPage) || (cursorPosition[secondaryDevice] != orgCursorPosition))
				{
				CIniFile gameOrderIni(gameOrderIniPath);
				vector<std::string> gameOrder;
				char str[11];

				for(int i=0;i<(int)dirContents[scrn].size();i++) {
					sprintf(str, "%d", i);
					gameOrder.push_back(gameOrderIni.GetString(getcwd(path, PATH_MAX), str, "NULL"));
					if(gameOrder[i] == "NULL")
						gameOrder[i] = dirContents[scrn][i].name;
				}

				for(int i=0;i<(int)gameOrder.size();i++) {
					for(int j=0;j<(int)gameOrder.size();j++) {
						if(i!=j) {
							if(gameOrder[i] == gameOrder[j]) {
								gameOrder.erase(gameOrder.begin()+j);
							}
						}
					}
				}

				for(int i=gameOrder.size();true;i++) {
					sprintf(str, "%d", i);
					if(gameOrderIni.GetString(getcwd(path, PATH_MAX), str, "") != "") {
						gameOrderIni.SetString(getcwd(path, PATH_MAX), str, "");
					} else {
						break;
					}
				}

				for(int i=0;i<(int)gameOrder.size();i++) {
					bool stillExists = false;
					for(int j=0;j<(int)dirContents[scrn].size();j++) {
						if(gameOrder[i] == dirContents[scrn][j].name) {
							stillExists = true;
							break;
						}
					}
					if(!stillExists)
						gameOrder.erase(gameOrder.begin()+i);
				}

				gameOrder.erase(gameOrder.begin()+movingApp);
				gameOrder.insert(gameOrder.begin()+cursorPosition[secondaryDevice]+(pagenum[secondaryDevice]*40), gameBeingMoved);

				for(int i=0;i<(int)gameOrder.size();i++) {
					char str[9];
					sprintf(str, "%d", i);
					gameOrderIni.SetString(getcwd(path, PATH_MAX), str, gameOrder[i]);
				}
				gameOrderIni.SaveIniFile(gameOrderIniPath);
				
				getDirectoryContents(dirContents[scrn], extensionList);
				getFileInfo(scrn, dirContents, false);

				settingsChanged = true;
				boxArtLoaded = false;
				}
				movingApp = -1;

			// Scrollbar
			} else if (((pressed & KEY_TOUCH) && touch.py > 171 && touch.px >= 30 && touch.px <= 227 && theme == 0 && !titleboxXmoveleft && !titleboxXmoveright))		// Scroll bar (DSi theme))
			{
				touchPosition startTouch = touch;
				int prevPos = cursorPosition[secondaryDevice];
				showSTARTborder = false;
				scrollWindowTouched = true;
				while(1) {
					scanKeys();
					touchRead(&touch);

					if (!(keysHeld() & KEY_TOUCH)) break;

					cursorPosition[secondaryDevice] = round((touch.px-30)/4.925);
					if(cursorPosition[secondaryDevice] > 39) {
						cursorPosition[secondaryDevice] = 39;
						titlewindowXpos[secondaryDevice] = 192.075;
						titleboxXpos[secondaryDevice] = 2496;
					} else if(cursorPosition[secondaryDevice] < 0) {
						cursorPosition[secondaryDevice] = 0;
						titlewindowXpos[secondaryDevice] = 0;
						titleboxXpos[secondaryDevice] = 0;
					} else {
						titlewindowXpos[secondaryDevice] = touch.px-30;
						titleboxXpos[secondaryDevice] = (touch.px-30)/4.925 * 64;
					}
					

					// Load icons
					if (prevPos == cursorPosition[secondaryDevice]+1) {
						if(cursorPosition[secondaryDevice] > 2) {
							if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2);
							}
						}
					} else if (prevPos == cursorPosition[secondaryDevice]-1) {
						if(cursorPosition[secondaryDevice] < 37) {
							if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]+2);
							}
						}
					} else if (cursorPosition[secondaryDevice] <= 1) {
						for(int i = 0; i < 5; i++) {
							if(i%2)	swiWaitForVBlank();
							if (bnrRomType[i] == 0 && i+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at(i+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str(), i);
							}
						}
					} else if (cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
						for(int i = 0; i < 6; i++) {
							if(i%2)	swiWaitForVBlank();
							if (bnrRomType[i] == 0 && (cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2+i);
							}
						}
					} else if (cursorPosition[secondaryDevice] >= 37 && cursorPosition[secondaryDevice] <= 39) {
						for(int i = 0; i < 5; i++) {
							if(i%2)	swiWaitForVBlank();
							if (bnrRomType[i] == 0 && (35+i)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).name.c_str(), 35+i);
							}
						}
					}

					clearText();
					if(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 < ((int) dirContents[scrn].size())) {
						showbubble = true;
						titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
					} else {
						showbubble = false;
					}
					prevPos = cursorPosition[secondaryDevice];
				}
				scrollWindowTouched = false;
				titleboxXpos[secondaryDevice] = cursorPosition[secondaryDevice] * 64;
				titlewindowXpos[secondaryDevice] = cursorPosition[secondaryDevice] * 5;
				waitForNeedToPlayStopSound = 1;
				mmEffectEx(&snd_select);
				boxArtLoaded = false;
				settingsChanged = true;
				touch = startTouch;
				if(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 < ((int) dirContents[scrn].size()))
					showSTARTborder = true;

				// Draw icons 1 per vblank to prevent corruption
				if (cursorPosition[secondaryDevice] <= 1) {
					for(int i = 0; i < 5; i++) {
						swiWaitForVBlank();
						if (bnrRomType[i] == 0 && i+pagenum[secondaryDevice]*40 < file_count) {
							iconUpdate(dirContents[scrn].at(i+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str(), i);
						}
					}
				} else if (cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
					for(int i = 0; i < 6; i++) {
						swiWaitForVBlank();
						if (bnrRomType[i] == 0 && (cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40 < file_count) {
							iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2+i);
						}
					}
				} else if (cursorPosition[secondaryDevice] >= 37 && cursorPosition[secondaryDevice] <= 39) {
					for(int i = 0; i < 5; i++) {
						swiWaitForVBlank();
						if (bnrRomType[i] == 0 && (35+i)+pagenum[secondaryDevice]*40 < file_count) {
							iconUpdate(dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).name.c_str(), 35+i);
						}
					}
				}

			// Dragging icons
			} else if (((pressed & KEY_TOUCH) && touch.py > 88 && touch.py < 144 && !titleboxXmoveleft && !titleboxXmoveright)) {
				touchPosition startTouch = touch;
				touchPosition prevTouch1 = touch;
				touchPosition prevTouch2 = touch;
				int prevPos = cursorPosition[secondaryDevice];
				showSTARTborder = false;

				if(touch.px > 96 && touch.px < 160 && touch.py < 144 && touch.py > 88) {
					while(1) {
						scanKeys();
						touchRead(&touch);
						if(!(keysHeld() & KEY_TOUCH)) {
							gameTapped = true;
							break;
						}
						else if(touch.px < startTouch.px-10 || touch.px > startTouch.px+10)
							break;
					}
				}

				while(1) {
					if(gameTapped)
						break;
					scanKeys();
					touchRead(&touch);

					if (!(keysHeld() & KEY_TOUCH)) {
						bool tapped = false;
						int dX = (-(prevTouch1.px-prevTouch2.px));
						int decAmount = abs(dX);
						if(dX>0) {
							while(decAmount>.25) {
								if(theme && titleboxXpos[secondaryDevice] > 2496)
									break;
								scanKeys();
								if(keysHeld() & KEY_TOUCH) {
									tapped = true;
									break;
								}

								titlewindowXpos[secondaryDevice] = (titleboxXpos[secondaryDevice]+32)*0.078125;;
								if(titlewindowXpos[secondaryDevice] > 192.075)	titlewindowXpos[secondaryDevice] = 192.075;

								for(int i=0;i<2;i++) {
									swiWaitForVBlank();
									if(titleboxXpos[secondaryDevice] < 2496)	titleboxXpos[secondaryDevice] += decAmount/2;
									else	titleboxXpos[secondaryDevice] += decAmount/4;
								}
								decAmount = decAmount/1.25;

								cursorPosition[secondaryDevice] = round((titleboxXpos[secondaryDevice]+32)/64);

								if(cursorPosition[secondaryDevice] < 37) {
									if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count) {
										iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]+2);
									}
								}
							}
						} else if (dX<0) {
							while(decAmount>.25) {
								if(theme && titleboxXpos[secondaryDevice]<0)
									break;
								scanKeys();
								if(keysHeld() & KEY_TOUCH) {
									tapped = true;
									break;
								}

								titlewindowXpos[secondaryDevice] = (titleboxXpos[secondaryDevice]+32)*0.078125;;
								if(titlewindowXpos[secondaryDevice] < 0)	titlewindowXpos[secondaryDevice] = 0;

								for(int i=0;i<2;i++) {
									swiWaitForVBlank();
									if(titleboxXpos[secondaryDevice]>0)	titleboxXpos[secondaryDevice] -= decAmount/2;
									else	titleboxXpos[secondaryDevice] -= decAmount/4;	
								}
								decAmount = decAmount/1.25;

								cursorPosition[secondaryDevice] = round((titleboxXpos[secondaryDevice]+32)/64);

								if(cursorPosition[secondaryDevice] > 2) {
									if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40 < file_count) {
										iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2);
									}
								}
							}
						}
						if(tapped) continue;

						if (cursorPosition[secondaryDevice] < 0)
							cursorPosition[secondaryDevice] = 0;
						else if (cursorPosition[secondaryDevice] > 39)
							cursorPosition[secondaryDevice] = 39;

						// Load icons
						if (cursorPosition[secondaryDevice] <= 1) {
							for(int i = 0; i < 5; i++) {
								swiWaitForVBlank();
								if (bnrRomType[i] == 0 && i+pagenum[secondaryDevice]*40 < file_count) {
									iconUpdate(dirContents[scrn].at(i+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(i+pagenum[secondaryDevice]*40).name.c_str(), i);
								}
							}
						} else if (cursorPosition[secondaryDevice] >= 2 && cursorPosition[secondaryDevice] <= 36) {
							for(int i = 0; i < 6; i++) {
								swiWaitForVBlank();
								if (bnrRomType[i] == 0 && (cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40 < file_count) {
									iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2+i)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2+i);
								}
							}
						} else if (cursorPosition[secondaryDevice] >= 37 && cursorPosition[secondaryDevice] <= 39) {
							for(int i = 0; i < 5; i++) {
								swiWaitForVBlank();
								if (bnrRomType[i] == 0 && (35+i)+pagenum[secondaryDevice]*40 < file_count) {
									iconUpdate(dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((35+i)+pagenum[secondaryDevice]*40).name.c_str(), 35+i);
								}
							}
						}
						break;
					}

					titleboxXpos[secondaryDevice] += (-(touch.px-prevTouch1.px));
					cursorPosition[secondaryDevice] = round((titleboxXpos[secondaryDevice]+32)/64);
					titlewindowXpos[secondaryDevice] = (titleboxXpos[secondaryDevice]+32)*0.078125;
					if(titleboxXpos[secondaryDevice] > 2496) {
						if(theme)
							titleboxXpos[secondaryDevice] = 2496;
						cursorPosition[secondaryDevice] = 39;
						titlewindowXpos[secondaryDevice] = 192.075;
					} else if(titleboxXpos[secondaryDevice] < 0) {
						if(theme)
							titleboxXpos[secondaryDevice] = 0;
						cursorPosition[secondaryDevice] = 0;
						titlewindowXpos[secondaryDevice] = 0;
					}

					// Load icons
					if (prevPos == cursorPosition[secondaryDevice]+1) {
						if(cursorPosition[secondaryDevice] > 2) {
							if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]-2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]-2);
							}
						}
					} else if (prevPos == cursorPosition[secondaryDevice]-1) {
						if(cursorPosition[secondaryDevice] < 37) {
							if (bnrRomType[0] == 0 && (cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40 < file_count) {
								iconUpdate(dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at((cursorPosition[secondaryDevice]+2)+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]+2);
							}
						}
					}

					if(prevPos != cursorPosition[secondaryDevice]) {
						clearText();
						if(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 < ((int) dirContents[scrn].size())) {
							showbubble = true;
							titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
						} else {
							showbubble = false;
						}
					}
					prevTouch2 = prevTouch1;
					prevTouch1 = touch;
					prevPos = cursorPosition[secondaryDevice];
					swiWaitForVBlank();
					swiWaitForVBlank();
				}
				titlewindowXpos[secondaryDevice] = cursorPosition[secondaryDevice] * 5;;
				titleboxXpos[secondaryDevice] = cursorPosition[secondaryDevice]*64;
				boxArtLoaded = false;
				settingsChanged = true;
				touch = startTouch;
				if(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40 < ((int) dirContents[scrn].size()))
					showSTARTborder = (theme == 1 ? true : false);
			}

			if (cursorPosition[secondaryDevice] < 0)
				cursorPosition[secondaryDevice] = 0;
			else if (cursorPosition[secondaryDevice] > 39)
				cursorPosition[secondaryDevice] = 39;

			// Startup...
			if (((pressed & KEY_A) && showbubble && showSTARTborder && !titleboxXmoveleft && !titleboxXmoveright)
			|| ((pressed & KEY_START) && showbubble && showSTARTborder && !titleboxXmoveleft && !titleboxXmoveright)
			|| gameTapped)
			{
				DirEntry* entry = &dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40);
				if (entry->isDirectory)
				{
					// Enter selected directory
					mmEffectEx(&snd_select);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum[secondaryDevice] = 0;
					cursorPosition[secondaryDevice] = 0;
					titleboxXpos[secondaryDevice] = 0;
					titlewindowXpos[secondaryDevice] = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					stopSoundPlayed = false;
					clearText();
					chdir(entry->name.c_str());
					char buf[256];
					romfolder[secondaryDevice] = getcwd(buf, 256);
					SaveSettings();
					settingsChanged = false;
					return "null";
				}
				else if ((isDSiWare[cursorPosition[secondaryDevice]] && !isDSiMode())
						|| (isDSiWare[cursorPosition[secondaryDevice]] && !sdFound())
						|| (isDSiWare[cursorPosition[secondaryDevice]] && consoleModel > 1))
				{
					mmEffectEx(&snd_wrong);
					clearText();
					dbox_showIcon = true;
					showdialogbox = true;
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
					printSmallCentered(false, 112, "This game cannot be launched");
					if (isDSiMode()) {
						if (sdFound()) {
							printSmallCentered(false, 128, "as a .nds file on 3DS/2DS.");
						} else {
							printSmallCentered(false, 128, "without an SD card.");
						}
					} else {
						printSmallCentered(false, 128, "in DS mode.");
					}
					printSmall(false, 208, 160, "A: OK");
					pressed = 0;
					do {
						scanKeys();
						pressed = keysDown();
						loadVolumeImage();
						loadBatteryImage();
						loadTime();
						loadDate();
						loadClockColon();
						swiWaitForVBlank();
					} while (!(pressed & KEY_A));
					clearText();
					showdialogbox = false;
					for (int i = 0; i < 15; i++) swiWaitForVBlank();
					dbox_showIcon = false;
				}
				else
				{
					bool hasAP = false;
					bool proceedToLaunch = true;
					if (useBootstrap
					&& bnrRomType[cursorPosition[secondaryDevice]] == 0 && !isDSiWare[cursorPosition[secondaryDevice]]
					&& isHomebrew[cursorPosition[secondaryDevice]] == 0 && checkIfShowAPMsg(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name))
					{
						FILE *f_nds_file = fopen(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), "rb");
						hasAP = checkRomAP(f_nds_file);
						fclose(f_nds_file);
					}
					else if (bnrRomType[cursorPosition[secondaryDevice]] == 6)
					{
						if (getFileSize(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str()) > 0x300000) {
							proceedToLaunch = false;
							mdRomTooBig();
						}
					}
					if (hasAP) {
					clearText();
					dbox_showIcon = true;
					showdialogbox = true;
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
					printSmallCentered(false, 64, "This game may not work correctly,");
					printSmallCentered(false, 78, "if it's not AP-patched.");
					printSmallCentered(false, 112, "If the game freezes, does not");
					printSmallCentered(false, 126, "start, or doesn't seem normal,");
					printSmallCentered(false, 140, "it needs to be AP-patched.");
					printSmallCentered(false, 160, "B/A: OK, X: Don't show again");
					pressed = 0;
					while (1) {
						scanKeys();
						pressed = keysDown();
						loadVolumeImage();
						loadBatteryImage();
						loadTime();
						loadClockColon();
						swiWaitForVBlank();
						if (pressed & KEY_A) {
							pressed = 0;
							break;
						}
						if (pressed & KEY_B) {
							gameTapped = false;
							proceedToLaunch = false;
							pressed = 0;
							break;
						}
						if (pressed & KEY_X) {
							dontShowAPMsgAgain(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name);
							pressed = 0;
							break;
						}
					}
					clearText();
					showdialogbox = false;
					for (int i = 0; i < (proceedToLaunch ? 20 : 15); i++) swiWaitForVBlank();
					dbox_showIcon = false;
					}

					if (proceedToLaunch) {
					mmEffectEx(&snd_launch);
					controlTopBright = true;
					applaunch = true;
					applaunchprep = true;

					if (theme == 0) {
						showbubble = false;
						showSTARTborder = false;
						clearText(false);	// Clear title

						fadeSpeed = false;	// Slow fade speed
						for (int i = 0; i < 5; i++) {
							swiWaitForVBlank();
						}
					}
					fadeType = false;	// Fade to white
					fifoSendValue32(FIFO_USER_01, 1);	// Fade out sound
					for (int i = 0; i < 60; i++) {
						swiWaitForVBlank();
					}
					music = false;
					mmEffectCancelAll();
					fifoSendValue32(FIFO_USER_01, 0);	// Cancel sound fade-out

					clearText(true);

					// Return the chosen file
					return entry->name;
					}
				}
			}

			if (theme == 1) {
				// Launch settings by touching corner button
				if ((pressed & KEY_TOUCH) && touch.py <= 26 && touch.px <= 44 && !titleboxXmoveleft && !titleboxXmoveright)
				{
					launchSettings();
				}

				// Exit to system menu by touching corner button
				if ((pressed & KEY_TOUCH) && touch.py <= 26 && touch.px >= 212 && !isRegularDS && !titleboxXmoveleft && !titleboxXmoveright)
				{
					exitToSystemMenu();
				}

				int topIconXpos = 116;
				int savedTopIconXpos[2] = {0};
				if (isDSiMode() && sdFound()) {
					//for (int i = 0; i < 4; i++) {
						topIconXpos -= 14;
					//}
					for (int i = 0; i < 2; i++) {
						savedTopIconXpos[i] = topIconXpos;
						topIconXpos += 28;
					}
				} else {
					//for (int i = 0; i < 3; i++) {
						topIconXpos -= 14;
					//}
					for (int i = 1; i < 2; i++) {
						savedTopIconXpos[i] = topIconXpos;
						topIconXpos += 28;
					}
				}

				if (isDSiMode() && sdFound()) {
					// Switch devices or launch Slot-1 by touching button
					if ((pressed & KEY_TOUCH) && touch.py <= 26 && touch.px >= savedTopIconXpos[0] && touch.px < savedTopIconXpos[0]+24
					&& !titleboxXmoveleft && !titleboxXmoveright)
					{
						if (secondaryDevice || REG_SCFG_MC != 0x11) {
							switchDevice();
							return "null";
						} else {
							mmEffectEx(&snd_wrong);
						}
					}
				}

				// Launch GBA by touching button
				if ((pressed & KEY_TOUCH) && touch.py <= 26 && touch.px >= savedTopIconXpos[1] && touch.px < savedTopIconXpos[1]+24 && !titleboxXmoveleft && !titleboxXmoveright)
				{
					launchGba();
				}
			}

			// page switch

			if (pressed & KEY_L)
			{
				if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && pagenum[secondaryDevice] != 0) {
					mmEffectEx(&snd_switch);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum[secondaryDevice] -= 1;
					cursorPosition[secondaryDevice] = 0;
					titleboxXpos[secondaryDevice] = 0;
					titlewindowXpos[secondaryDevice] = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					stopSoundPlayed = false;
					clearText();
					SaveSettings();
					settingsChanged = false;
					displayNowLoading();
					break;
				} else {
					mmEffectEx(&snd_wrong);
				}
			} else 	if (pressed & KEY_R)
			{
				if (!startMenu && !titleboxXmoveleft && !titleboxXmoveright && file_count > 40+pagenum[secondaryDevice]*40) {
					mmEffectEx(&snd_switch);
					fadeType = false;	// Fade to white
					for (int i = 0; i < 30; i++) swiWaitForVBlank();
					pagenum[secondaryDevice] += 1;
					cursorPosition[secondaryDevice] = 0;
					titleboxXpos[secondaryDevice] = 0;
					titlewindowXpos[secondaryDevice] = 0;
					whiteScreen = true;
					if (showBoxArt) clearBoxArt();	// Clear box art
					boxArtLoaded = false;
					redoDropDown = true;
					shouldersRendered = false;
					showbubble = false;
					showSTARTborder = false;
					stopSoundPlayed = false;
					clearText();
					SaveSettings();
					settingsChanged = false;
					displayNowLoading();
					break;
				} else {
					mmEffectEx(&snd_wrong);
				}
			}

			if ((pressed & KEY_B) && showDirectories) {
				// Go up a directory
				mmEffectEx(&snd_select);
				fadeType = false;	// Fade to white
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				pagenum[secondaryDevice] = 0;
				cursorPosition[secondaryDevice] = 0;
				titleboxXpos[secondaryDevice] = 0;
				titlewindowXpos[secondaryDevice] = 0;
				whiteScreen = true;
				if (showBoxArt) clearBoxArt();	// Clear box art
				boxArtLoaded = false;
				redoDropDown = true;
				shouldersRendered = false;
				showbubble = false;
				showSTARTborder = false;
				stopSoundPlayed = false;
				clearText();
				chdir("..");
				char buf[256];
				romfolder[secondaryDevice] = getcwd(buf, 256);
				SaveSettings();
				settingsChanged = false;
				return "null";
			}

			if ((pressed & KEY_X) && !startMenu && showbubble && showSTARTborder)
			{
				CIniFile hiddenGamesIni(hiddenGamesIniPath);
				vector<std::string> hiddenGames;
				char str[11];

				for(int i=0;true;i++) {
					sprintf(str, "%d", i);
					if(hiddenGamesIni.GetString(getcwd(path, PATH_MAX), str, "") != "") {
						hiddenGames.push_back(hiddenGamesIni.GetString(getcwd(path, PATH_MAX), str, ""));
					} else {
						break;
					}
				}

				for(int i=0;i<(int)hiddenGames.size();i++) {
					for(int j=0;j<(int)hiddenGames.size();j++) {
						if(i!=j) {
							if(hiddenGames[i] == hiddenGames[j]) {
								hiddenGames.erase(hiddenGames.begin()+j);
							}
						}
					}
				}
				
				std::string gameBeingHidden = dirContents[scrn].at((pagenum[secondaryDevice]*40)+(cursorPosition[secondaryDevice])).name;
				bool unHide = false;
				int whichToUnhide;

				for(int i=0;i<(int)hiddenGames.size();i++) {
					if(hiddenGames[i] == gameBeingHidden) {
						whichToUnhide = i;
						unHide = true;
					}
				}

				clearText();
				dbox_showIcon = true;
				showdialogbox = true;
				for (int i = 0; i < 30; i++) swiWaitForVBlank();
				snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (cursorPosition[secondaryDevice]+1)+pagenum[secondaryDevice]*40, file_count);
				titleUpdate(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).isDirectory, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str(), cursorPosition[secondaryDevice]);
				printSmall(false, 16, 64, dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str());
				printSmall(false, 16, 166, fileCounter);
				printSmallCentered(false, 112, "Are you sure you want to");
				if (isDirectory[cursorPosition[secondaryDevice]]) {
					if(unHide)	printSmallCentered(false, 128, "unhide this folder?");
					else	printSmallCentered(false, 128, "hide this folder?");
				} else {
					if(unHide)	printSmallCentered(false, 128, "delete/unhide this game?");
					else	printSmallCentered(false, 128, "delete/hide this game?");
				}
				for (int i = 0; i < 90; i++) swiWaitForVBlank();
				if (isDirectory[cursorPosition[secondaryDevice]]) {
					if(unHide)	printSmall(false, 141, 160, "Y: Unhide");	
					else	printSmall(false, 155, 160, "Y: Hide");
					printSmall(false, 208, 160, "B: No");
				} else {
					if(unHide)	printSmall(false, 93, 160, "Y: Unhide");	
					else	printSmall(false, 107, 160, "Y: Hide");
					printSmall(false, 160, 160, "A: Yes");
					printSmall(false, 208, 160, "B: No");
				}
				while (1) {
					do {
						scanKeys();
						pressed = keysDownRepeat();
						swiWaitForVBlank();
					} while (!pressed);
					
					if (pressed & KEY_A && !isDirectory[cursorPosition[secondaryDevice]]) {
						fadeType = false;	// Fade to white
						for (int i = 0; i < 30; i++) swiWaitForVBlank();
						whiteScreen = true;
						remove(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name.c_str()); // Remove game/folder
						if (showBoxArt) clearBoxArt();	// Clear box art
						boxArtLoaded = false;
						shouldersRendered = false;
						showbubble = false;
						showSTARTborder = false;
						stopSoundPlayed = false;
						clearText();
						showdialogbox = false;
						SaveSettings();
						settingsChanged = false;
						return "null";
					}

					if (pressed & KEY_B) {
						break;
					}

					if (pressed & KEY_Y) {
						fadeType = false;	// Fade to white
						for (int i = 0; i < 30; i++) swiWaitForVBlank();
						whiteScreen = true;

						if(unHide) {
							hiddenGames.erase(hiddenGames.begin()+whichToUnhide);
							hiddenGames.push_back("");
						} else {
							hiddenGames.push_back(gameBeingHidden);
						}

						for(int i=0;i<(int)hiddenGames.size();i++) {
							char str[9];
							sprintf(str, "%d", i);
							hiddenGamesIni.SetString(getcwd(path, PATH_MAX), str, hiddenGames[i]);
						}
						hiddenGamesIni.SaveIniFile(hiddenGamesIniPath);

						if (showBoxArt) clearBoxArt();	// Clear box art
						boxArtLoaded = false;
						shouldersRendered = false;
						showbubble = false;
						showSTARTborder = false;
						stopSoundPlayed = false;
						clearText();
						showdialogbox = false;
						SaveSettings();
						settingsChanged = false;
						return "null";
					}
				}
				clearText();
				showdialogbox = false;
				for (int i = 0; i < 15; i++) swiWaitForVBlank();
				dbox_showIcon = false;
			}

			if ((pressed & KEY_Y) && !startMenu
			&& (isDirectory[cursorPosition[secondaryDevice]] == false) && (bnrRomType[cursorPosition[secondaryDevice]] == 0)
			&& !titleboxXmoveleft && !titleboxXmoveright && showbubble && showSTARTborder)
			{
				perGameSettings(dirContents[scrn].at(cursorPosition[secondaryDevice]+pagenum[secondaryDevice]*40).name);
			}

			if ((pressed & KEY_SELECT) && theme == 0) {
				if (selectMenu()) {
					clearText();
					showdialogbox = false;
					dbox_selectMenu = false;
					return "null";
				}
			}

		}
	}
}
