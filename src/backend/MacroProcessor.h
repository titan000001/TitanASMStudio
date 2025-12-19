#ifndef MACROPROCESSOR_H
#define MACROPROCESSOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

struct MacroDefinition {
    std::vector<std::string> parameters; // e.g., "&A", "&B"
    std::vector<std::string> body;       // The lines of code inside
};

class MacroProcessor {
private:
    std::map<std::string, MacroDefinition> macroTable;

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    
    // Replace all occurrences of &ARG with value in a line
    std::string substitute(std::string line, const std::map<std::string, std::string>& argsMap);

public:
    MacroProcessor();
    // Returns true if success. Writes expanded code to outputFile.
    bool expandMacros(const std::string& inputFile, const std::string& outputFile);
};

#endif
