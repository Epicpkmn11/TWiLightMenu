#include "common/argv.h"

enum class ParseMode {
	Normal,
	Single,
	Double,
	Comment
};

std::vector<std::string> parseArgv(const char *filename, bool skipArgs) {
	std::vector<std::string> argv;

	// Read in the file
	FILE *argfile = fopen(filename, "rb");
	if(!argfile)
		return argv;

	fseek(argfile, 0, SEEK_END);
	long fsize = ftell(argfile);
	fseek(argfile, 0, SEEK_SET);

	char *buffer = new char[fsize + 1];
	fread(buffer, 1, fsize, argfile);
	buffer[fsize] = '\0';

	fclose(argfile);

	// Parse for arguments
	char *ptr = buffer;
	std::string temp;
	ParseMode parseMode = ParseMode::Normal;
	while(*ptr) {
		switch(*ptr) {
			case '\'':
				switch(parseMode) {
					case ParseMode::Normal:
						parseMode = ParseMode::Single;
						break;
					case ParseMode::Single:
						parseMode = ParseMode::Normal;
						break;
					case ParseMode::Double:
					case ParseMode::Comment:
						break;
				}
				break;

			case '"':
				switch(parseMode) {
					case ParseMode::Normal:
						parseMode = ParseMode::Double;
						break;
					case ParseMode::Double:
						parseMode = ParseMode::Normal;
						break;
					case ParseMode::Single:
					case ParseMode::Comment:
						break;
				}
				break;

			case '\r':
			case '\n':
				if(parseMode == ParseMode::Comment && ptr[1] != '\n')
					parseMode = ParseMode::Normal;
				goto whitespace;

			case ' ':
			case '\t':
			whitespace:
				// Split unless in quotes
				if(parseMode == ParseMode::Normal) {
					if(temp.size() > 0) {
						argv.push_back(temp);

						// Only looking for file name, we're done
						if(skipArgs)
							return argv;

						temp = "";
					}
				} else {
					goto addToStr;
				}
				break;

			case '\\':
				// Skip this and add the next character directly to the
				// string without processing it
				if(ptr[1])
					ptr++;
				goto addToStr;
			
			case '#':
				// Comment, ignore to the end of the line
				parseMode = ParseMode::Comment;
				break;

			default:
			addToStr:
				if(parseMode != ParseMode::Comment)
					temp += *ptr;
				break;
		}

		ptr++;
	}

	// Push any leftover if we ended without whitespace
	if(temp.size() > 0)
		argv.emplace_back(temp);

	delete[] buffer;
	
	return argv;
}
