#ifndef ORDINAL_INDEX_H
#define ORDINAL_INDEX_H

#include <string>

struct OrdinalIndex {
    int omegaPart;
    int finitePart;

    OrdinalIndex(int omega = 0, int finite = 0)
        : omegaPart(omega), finitePart(finite) {}

    OrdinalIndex operator+(int value) const {
        return OrdinalIndex(omegaPart, finitePart + value);
    }

    // НОВОЕ: Математически верный сдвиг при вставке бесконечности (omega) в точку X
    OrdinalIndex ShiftByOmega(const OrdinalIndex& X) const {
        if (this->omegaPart == X.omegaPart) {
            // Элемент был в том же омега-блоке. Он уезжает на следующий уровень.
            // Его новая конечная часть = старая конечная часть - точка вставки.
            return OrdinalIndex(this->omegaPart + 1, this->finitePart - X.finitePart);
        } else {
            // Элемент был на уровне выше (например, w или 2w). Он просто поднимается на уровень выше.
            return OrdinalIndex(this->omegaPart + 1, this->finitePart);
        }
    }

    // Сдвиг на ординальное смещение (используется при конкатенации)
    OrdinalIndex ShiftBy(const OrdinalIndex& shift) const {
        return OrdinalIndex(
            this->omegaPart + shift.omegaPart,
            this->finitePart + shift.finitePart
        );
    }

    bool operator==(const OrdinalIndex& other) const {
        return omegaPart == other.omegaPart && finitePart == other.finitePart;
    }
    bool operator!=(const OrdinalIndex& other) const { return !(*this == other); }
    
    bool operator<(const OrdinalIndex& other) const {
        if (omegaPart != other.omegaPart) return omegaPart < other.omegaPart;
        return finitePart < other.finitePart;
    }
    bool operator>(const OrdinalIndex& other) const { return other < *this; }
    bool operator<=(const OrdinalIndex& other) const { return !(*this > other); }
    bool operator>=(const OrdinalIndex& other) const { return !(*this < other); }

    std::string ToString() const {
        if (omegaPart == 0) return std::to_string(finitePart);
        std::string result = (omegaPart == 1) ? "ω" : std::to_string(omegaPart) + "ω";
        if (finitePart > 0) result += "+" + std::to_string(finitePart);
        return result;
    }
};

#endif