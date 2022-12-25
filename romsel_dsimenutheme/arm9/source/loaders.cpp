#include "loaders.h"

#include "common/flashcard.h"
#include "fileBrowse.h"
#include "graphics/iconHandler.h"
#include "graphics/Texture.h"

#include <dirent.h>
#include <unistd.h>

int Loader::iconNum = 0;

std::vector<Loader> loaders = {
	{{".nds", ".dsi", ".ids", ".srl", ".app", ".argv"}} // Hardcode nds-bootstrap
};

Loader::Loader(const std::string &iniPath) {
	CIniFile ini(iniPath);

	useBootstrap = ini.GetInt("LOADER", "USE_BOOTSTRAP", useBootstrap);
	useRamdrive = ini.GetInt("LOADER", "USE_RAMDRIVE", useRamdrive);
	dsiMode = ini.GetInt("LOADER", "DSI_MODE", dsiMode);
	boostCpu = ini.GetInt("LOADER", "BOOST_CPU", boostCpu);
	boostVram = ini.GetInt("LOADER", "BOOST_VRAM", boostVram);
	path = ini.GetString("LOADER", "PATH", "DSI_MODE");
	ini.GetStringVector("LOADER", "EXTENSIONS", extensions, ':');

	std::string iconPath = ini.GetString("LOADER", "ICON_PATH", "");
	if(iconPath != "" && iconNum < 20) {
		Texture tex = Texture(iconPath, iconPath);
		glLoadIcon(iconNum++, tex.palette(), tex.bytes(), tex.texHeight());
	}
}

void loadLoaders(void) {
	char oldPath[PATH_MAX];
	getcwd(oldPath, PATH_MAX);
	chdir(sdFound() ? "sd:/_nds/TWiLightMenu/emulators" : "fat:/_nds/TWiLightMenu/emulators");

	std::vector<DirEntry> dirContents;
	getDirectoryContents(dirContents, {"ini"});

	for(const DirEntry &entry : dirContents) {
		loaders.emplace_back(entry.name);
	}

	chdir(oldPath);
}

const Loader *whichLoader(const std::string &filename) {
	for(uint i = 0; i < loaders.size(); i++) {
		if(extension(filename, loaders[i].extensions))
			return &loaders[i];
	}

	return nullptr;
}
