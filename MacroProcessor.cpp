#include "MacroProcessor.h"

MacroProcessor::MacroProcessor() {}

std::string MacroProcessor::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> MacroProcessor::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

std::string MacroProcessor::substitute(std::string line, const std::map<std::string, std::string>& argsMap) {
    std::string result = line;
    for (auto const& [key, val] : argsMap) {
        // Find key (e.g., "&A") and replace with val (e.g., "50")
        size_t pos = 0;
        while ((pos = result.find(key, pos)) != std::string::npos) {
            result.replace(pos, key.length(), val);
            pos += val.length();
        }
    }
    return result;
}

bool MacroProcessor::expandMacros(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);
    
    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "MacroProcessor Error: Could not open files." << std::endl;
        return false;
    }

    std::string line;
    bool definingMacro = false;
    std::string currentMacroName = "";
    MacroDefinition currentMacro;

    while (std::getline(inFile, line)) {
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty()) {
            outFile << line << std::endl;
            continue;
        }

        // Tokenize by space to check for MACRO keyword
        std::stringstream ss(trimmedLine);
        std::string firstToken;
        ss >> firstToken;

        if (firstToken == "MACRO") {
            definingMacro = true;
            // Format: MACRO Name Arg1 Arg2...
            ss >> currentMacroName;
            currentMacro = MacroDefinition();
            
            std::string arg;
            while (ss >> arg) {
                // Support comma-separated args? Usually specific parsing.
                // Here assume space separated parameters starting with &
                // e.g. MACRO SUM &A &B
                if (arg.back() == ',') arg.pop_back();
                currentMacro.parameters.push_back(arg);
            }
            continue; // Do not write definition to output
        }

        if (trimmedLine == "MEND") {
            if (definingMacro) {
                macroTable[currentMacroName] = currentMacro;
                definingMacro = false;
            } else {
                std::cerr << "Error: MEND without MACRO" << std::endl;
            }
            continue;
        }

        if (definingMacro) {
            currentMacro.body.push_back(line); // Store original line indentation
            continue;
        }

        // Check if line is a Macro Call
        // Tokenize line to find Label: Macro Args... or Macro Args...
        // For simplicity, assume: [Label:] MacroName Arg1, Arg2
        
        std::string macroName = firstToken;
        std::string label = "";
        std::vector<std::string> callArgs;
        
        // Check for Label
        bool hasLabel = false;
        if (macroName.back() == ':') {
            hasLabel = true;
            label = macroName;
            ss >> macroName; // Next token should be macro name
        }

        if (macroTable.count(macroName)) {
            // It IS a macro call!
            
            // 1. Output the Label if any
            if (hasLabel) {
                 // Write label on its own line
                 outFile << label << std::endl;
            }

            // 2. Parse Arguments (rest of the line)
            // Need to handle "50, 51" or "50 51"
            std::string restOfLine;
            std::getline(ss, restOfLine);
            if (restOfLine.empty() && !ss.eof()) {
                 // ss might have consumed everything if args were read? No, getline reads rest.
            }
            
            // Clean split by comma or space
            std::replace(restOfLine.begin(), restOfLine.end(), ',', ' ');
            std::stringstream argSS(restOfLine);
            std::string argVal;
            while (argSS >> argVal) {
                callArgs.push_back(argVal);
            }

            MacroDefinition& def = macroTable[macroName];
            
            if (callArgs.size() != def.parameters.size()) {
                std::cerr << "Warning: Macro " << macroName << " expects " << def.parameters.size() 
                          << " args, got " << callArgs.size() << std::endl;
            }

            // 3. Map Parameters
            std::map<std::string, std::string> argsMap;
            for (size_t i = 0; i < def.parameters.size() && i < callArgs.size(); i++) {
                argsMap[def.parameters[i]] = callArgs[i];
            }

            // 4. Expand Body
            outFile << "; Begin Macro Expansion: " << macroName << std::endl;
            for (const std::string& bodyLine : def.body) {
                outFile << substitute(bodyLine, argsMap) << std::endl;
            }
            outFile << "; End Macro Expansion" << std::endl;

        } else {
            // Not a macro, just write the line
            outFile << line << std::endl;
        }
    }

    inFile.close();
    outFile.close();
    return true;
}
