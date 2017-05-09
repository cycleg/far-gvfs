#pragma once

#include <string>
#include <vector>
#include "plugin.hpp"
#include "MountPoint.h"

///
/// Структура-описание элемента диалога.
///
/// Преобразуется в структуру FarDialogItem.
///
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
        struct FarList* ListItems;
    };
    unsigned int Flags;
    int DefaultButton;
    int lngIdx;
    std::wstring text;
    int maxLen;
};

///
/// Редактирование ресурса для монтирования.
///
bool EditResourceDlg(PluginStartupInfo& info, MountPoint& mountPoint);
///
/// Запрос пароля перед монтированием ресурса.
///
bool AskPasswordDlg(PluginStartupInfo& info, MountPoint& mountPoint);
///
/// Редактирование общих настроек плагина.
///
bool ConfigurationEditDlg(PluginStartupInfo& info);
///
/// Ответ на вопрос в ходе монтирования ресурса.
///
bool AskQuestionDlg(PluginStartupInfo& info,
                    const int DIALOG_WIDTH,
                    const std::vector<std::wstring>& message,
                    const std::vector<std::wstring>& choices,
                    unsigned int& choice);
