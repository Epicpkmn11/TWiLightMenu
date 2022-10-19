#ifndef ARGV_H
#define ARGV_H

#include <string>
#include <vector>

std::vector<std::string> parseArgv(const char *filename, bool skipArgs = false);

#endif // ARGV_H