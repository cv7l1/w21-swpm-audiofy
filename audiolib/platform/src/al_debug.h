//
// Created by Jonathan on 02.12.2021.
//

#ifndef AUDIOFY_LIB_AL_DEBUG_H
#define AUDIOFY_LIB_AL_DEBUG_H
#include<string>

class Console {
public:
    static void setup();
    //THIS IS ALL STUPID! REPLACE REPLACE REPLACE
    inline static void writeInfo(const char* message, const char * file, const int line, const char* function) {
        printf("%S[file: %s, func: %s, l: %S] : %s%S", info_start, file, function, std::to_wstring(line).c_str(), message, end);
    }

    inline static void writeWarn(const char* message, const char* file, const int line, const char* function)
    {
        printf("%S[file: %s, func: %s, l: %S] : %s%S", warn_start, file, function, std::to_wstring(line).c_str(), message, end);
    }

    inline static void writeCritical(const char* message, const char* file, const int line, const char* function){
        printf("%S[file: %s, func: %s, l: %S] : %s%S", critical_start, file, function, std::to_wstring(line).c_str(), message, end);
    }

private:
    static constexpr const wchar_t* info_start = L"\x1b[32m[INFO]";
    static constexpr const wchar_t* warn_start = L"\x1b[33m[ERROR]";
    static constexpr const wchar_t* critical_start = L"\x1b[31m[CRITICAL]";
    static constexpr const wchar_t* end = L"\x1b[0m\n";

};
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


///Write Info Message
#define al_ErrorInfo(message) \
	Console::writeInfo((message), __FILENAME__, __LINE__, __func__)

///Write Warn Message
#define al_ErrorWarn(message) \
	Console::writeWarn((message), __FILENAME__, __LINE__, __func__)

///rite Critical Message
#define al_ErrorCritical(message) \
	Console::writeCritical((message), __FILENAME__, __LINE__, __func__)

#define WarnOnError(hr, message) \
	if(FAILED(hr)) {	\
	al_ErrorWarn(message); \
	}

#define WarnOnNull(val, message) \
	if(val == 0) { \
	al_ErrorWarn(message); \
	}

#define CriticalOnError(hr, message) \
	if(FAILED(hr)) { \
	al_ErrorCritical(message); \
	} \

#define CriticalOnNull(val, message) \
	if(val == 0) { \
	al_ErrorCritical(message); \
	}\

#endif //AUDIOFY_LIB_AL_DEBUG_H
