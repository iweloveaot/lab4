#ifndef ORDINAL_INDEX_PARSER_H
#define ORDINAL_INDEX_PARSER_H

#include <string>
#include <stdexcept>
#include <cctype>
#include "ordinal_index.h"

class OrdinalIndexParser {
public:
    static OrdinalIndex Parse(const std::string& str) {

        bool onlyDigits = true;
        for(char c : str) {
            if(!std::isdigit(static_cast<unsigned char>(c))) { 
                onlyDigits = false; 
                break; 
            }
        }
        if(onlyDigits) return OrdinalIndex(0, std::stoi(str));

        size_t omegaPos = std::string::npos;
        
        size_t p1 = str.find('w');
        size_t p2 = str.find('W');
        size_t p3 = str.find("ω");
        
        if (p1 != std::string::npos) omegaPos = p1;
        if (p2 != std::string::npos && (omegaPos == std::string::npos || p2 < omegaPos)) omegaPos = p2;
        if (p3 != std::string::npos && (omegaPos == std::string::npos || p3 < omegaPos)) omegaPos = p3;

        if(omegaPos == std::string::npos) {
            throw std::runtime_error("Invalid ordinal index. Use 'w', 'W', or 'ω'.");
        }

        int omegaPart = 1;
        if(omegaPos > 0) {
            omegaPart = std::stoi(str.substr(0, omegaPos));
        }

        int finitePart = 0;
        size_t plusPos = str.find("+", omegaPos); 
        if(plusPos != std::string::npos) {
            finitePart = std::stoi(str.substr(plusPos + 1));
        }

        return OrdinalIndex(omegaPart, finitePart);
    }
};

#endif