#ifndef _FUNCS_FOR_MM_
#define _FUNCS_FOR_MM_

int StrLen(const char* str) {
    if (!str) return 0;
    int len = 0;
    while (str[len]) len++;
    return len;
}

bool StrEq(const char* a, const char* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

int StrFind(const char* text, const char* pattern, int startPos = 0) {
    if (!text || !pattern) return -1;
    int textLen = StrLen(text);
    int patLen = StrLen(pattern);
    if (patLen == 0) return startPos;
    if (startPos < 0) startPos = 0;
    
    for (int i = startPos; i <= textLen - patLen; i++) {
        bool match = true;
        for (int j = 0; j < patLen; j++) {
            if (text[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

char* StrDup(const char* src) {
    if (!src) return nullptr;
    int len = StrLen(src);
    char* dup = new char[len + 1];
    for (int i = 0; i <= len; i++) dup[i] = src[i];
    return dup;
}


#endif /*_FUNCS_FOR_MM_*/