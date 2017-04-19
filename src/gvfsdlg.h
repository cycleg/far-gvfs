#pragma once

#include <string>
#include <vector>
#include "plugin.hpp"
#include "MountPoint.h"

struct InitDialogItem
{
    int Type;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Focus;
    DWORD_PTR Selected;
    unsigned int Flags;
    int DefaultButton;
    int lngIdx;
    std::wstring text;
};

bool GetLoginData(PluginStartupInfo &info, MountPoint& mountPoint);
