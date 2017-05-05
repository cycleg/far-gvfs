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
    union
    {
        DWORD_PTR Reserved;
        int Selected;
    };
    unsigned int Flags;
    int DefaultButton;
    int lngIdx;
    std::wstring text;
    int maxLen;
};

bool EditResourceDlg(PluginStartupInfo &info, MountPoint& mountPoint);
bool AskPasswordDlg(PluginStartupInfo &info, MountPoint& mountPoint);
bool ConfigurationEditDlg(PluginStartupInfo &info);
