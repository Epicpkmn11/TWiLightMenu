#ifndef LOADERS_H
#define LOADERS_H

#include "common/inifile.h"

#include <gl2d.h>
#include <memory>
#include <string>
#include <vector>

#define LOADER_NDS 0

class Loader {
	static int iconNum;

public:
	bool useBootstrap = false;
	bool useRamdrive = false;
	int dsiMode = 1;
	bool boostCpu = true;
	bool boostVram = true;
	bool dstwoPlg = false;
	std::string path;
	std::vector<std::string> extensions;
	glImage *icon = nullptr;

	Loader(const std::string &iniPath);
	Loader(const std::vector<std::string> extensions) : extensions(extensions) {}

	~Loader(void) {
		if(icon)
			delete icon;
	}
};

extern std::vector<Loader> loaders;

void loadLoaders(void);

// Returns the loader for the file if found, -1 if not
const Loader *whichLoader(const std::string &filename);

#endif // LOADERS_H
