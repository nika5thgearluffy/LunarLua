#ifndef ClipboardSystem_hhh
#define ClipboardSystem_hhh

#include <windows.h>
#include <string>

namespace ClipboardSystem
{
    std::string GetText();
    bool SetText(std::string text);
    bool HasText();
    void Clear();

    void Init();
    void StartMonitoring();
    void StopMonitoring();

    void History_Add(std::string text);
    std::string History_Get(int index);  // 0 = most recent
    int History_Count();
    void History_Clear();
}

#endif
