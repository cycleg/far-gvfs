#include <string>
#include <vector>
#include <iostream>
#include "Configuration.h"
#include "LngStringIDs.h"
#include "MountPoint.h"
#include "plugin.hpp"
#include "dialogs.h"

#define DLG_GET_TEXTPTR(info, hDlg, item) reinterpret_cast<const wchar_t*>(info.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, item, 0))
#define DLG_GET_CHECKBOX(info, hDlg, item) (info.SendDlgMessage(hDlg, DM_GETCHECK, item, 0) == BSTATE_CHECKED)
#define DLG_GET_COMBOBOXPOS(info, hDlg, item) info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, item, 0)

namespace {

// for DlgProc functions
PluginStartupInfo* startupInfo = nullptr;

} // anonymous namespace

///
/// @author invy, cycleg
///
void InitDialogItems(PluginStartupInfo& info,
                     const std::vector<InitDialogItem>& initItems,
                     std::vector<FarDialogItem>& dlgItems)
{
    dlgItems.clear();
    for (const auto &item : initItems)
    {
        FarDialogItem farDlgItem;
        farDlgItem.Type = item.Type;
        farDlgItem.X1 = item.X1;
        farDlgItem.Y1 = item.Y1;
        farDlgItem.X2 = item.X2;
        farDlgItem.Y2 = item.Y2;
        farDlgItem.Focus = item.Focus;
        switch (farDlgItem.Type)
        {
            case DI_CHECKBOX:
                farDlgItem.Selected = item.Selected;
                break;
            case DI_COMBOBOX:
                farDlgItem.ListItems = item.ListItems;
                break;
            default:
                farDlgItem.Reserved = item.Reserved;
                break;
        }
        farDlgItem.Flags = item.Flags;
        farDlgItem.DefaultButton = item.DefaultButton;

        // TODO: refactor
        farDlgItem.PtrData = ((item.lngIdx >= 0) && (item.lngIdx < __LAST_LNG_ENTRY__)) ?
                             info.GetMsg(info.ModuleNumber, (unsigned int)item.lngIdx) :
                             item.text.c_str();
        farDlgItem.MaxLen = item.maxLen;
        dlgItems.emplace_back(farDlgItem);
    }
}

enum EEditResourceDlg
{
  ResourcePathLabel = 1,
  ResourcePathInput,
  UserLabel,
  UserInput,
  PasswordLabel,
  PasswordInput,
  AskPasswordInput,
  OkButton,
  CancelButton
};

LONG_PTR WINAPI EditResourceDlgProc(HANDLE hDlg, int msg, int param1, LONG_PTR param2)
{
    if ((msg == DN_BTNCLICK) && (param1 == EEditResourceDlg::AskPasswordInput))
    {
        // password input enable/disable if checkbox unset/set 
        startupInfo->SendDlgMessage(hDlg, DM_ENABLE, EEditResourceDlg::PasswordLabel, param2 == 0);
        startupInfo->SendDlgMessage(hDlg, DM_ENABLE, EEditResourceDlg::PasswordInput, param2 == 0);
        if (param2)
        {
          // clear password input
          FarDialogItemData data;
          data.PtrLength = 0;
          data.PtrData = nullptr;
          startupInfo->SendDlgMessage(hDlg, DM_SETTEXT, EEditResourceDlg::PasswordInput, (LONG_PTR)&data);
        }
    }
    return startupInfo->DefDlgProc(hDlg, msg, param1, param2);
}

bool EditResourceDlg(PluginStartupInfo& info, MountPoint& mountPoint)
{
    const int DIALOG_WIDTH = 68;
    const int DIALOG_HEIGHT = 13;
    std::vector<InitDialogItem> initItems = {
        { DI_DOUBLEBOX, 2, 1, DIALOG_WIDTH - 3, DIALOG_HEIGHT - 2, 0, 0, 0, 0,
          MResourceTitle, L"", 0 },

        { DI_TEXT, 4, 2, 0, 2, 0, 0, 0, 0,
          MResourceUrl, L"", 0 },
        { DI_EDIT, 4, 3, DIALOG_WIDTH - 5, 3, 1, 0, 0, 0,
          -1, mountPoint.getUrl().c_str(), 0 },

        { DI_TEXT, 4, 4, 0, 4, 0, 0, 0, 0,
          MUser, L"", 0 },
        { DI_EDIT, 4, 5, DIALOG_WIDTH - 5, 5, 1, 0, 0, 0,
          -1, mountPoint.getUser().c_str(), 0 },

        { DI_TEXT, 4, 6, 0, 6, 0, 0,
          mountPoint.getAskPassword() ? DIF_DISABLE : 0, 0,
          MPassword, L"", 0 },
        { DI_PSWEDIT, 4, 7, DIALOG_WIDTH - 5, 7, 1, 0,
          mountPoint.getAskPassword() ? DIF_DISABLE : 0, 0, -1,
          mountPoint.getPassword().c_str(), 0 },

        { DI_CHECKBOX, 4, 8, 0, 8, 0, mountPoint.getAskPassword(), 0, 0,
          MAskPasswordEveryTime, L"", 0 },

        { DI_BUTTON, 0, 10, 0, 10, 0, 0, DIF_CENTERGROUP, 1,
          MOk, L"", 0 },
        { DI_BUTTON, 0, 10, 0, 10, 0, 0, DIF_CENTERGROUP, 0,
          MCancel, L"", 0 }
    };
    std::vector<FarDialogItem> dialogItems;
    InitDialogItems(info, initItems, dialogItems);
    startupInfo = &info;

    HANDLE hDlg = info.DialogInit(info.ModuleNumber, -1, -1, DIALOG_WIDTH,
                                  DIALOG_HEIGHT, L"Config", dialogItems.data(),
                                  dialogItems.size(), 0, 0, EditResourceDlgProc, 0);
    int ret = info.DialogRun(hDlg);
    // get user input
    std::wstring l_url = DLG_GET_TEXTPTR(info, hDlg, EEditResourceDlg::ResourcePathInput);
    std::wstring l_user = DLG_GET_TEXTPTR(info, hDlg, EEditResourceDlg::UserInput);
    std::wstring l_password = DLG_GET_TEXTPTR(info, hDlg, EEditResourceDlg::PasswordInput);
    bool l_askPassword = DLG_GET_CHECKBOX(info, hDlg, EEditResourceDlg::AskPasswordInput);
    info.DialogFree(hDlg);
    startupInfo = nullptr;
    // check user input
    if ((ret == -1) || (ret == EEditResourceDlg::CancelButton))
    {
        return false;
    }
    if (l_url.empty())
    {
        const wchar_t* msgItems[2];
        msgItems[0] = info.GetMsg(info.ModuleNumber, MError);
        msgItems[1] = info.GetMsg(info.ModuleNumber, MResourceUrlEmptyError);
        info.Message(info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, nullptr,
                     msgItems, ARRAYSIZE(msgItems), 0);
        return false;
    }
    if (l_user.empty() && !l_password.empty())
    {
        const wchar_t* msgItems[2];
        msgItems[0] = info.GetMsg(info.ModuleNumber, MError);
        msgItems[1] = info.GetMsg(info.ModuleNumber, MPasswordWithoutUserError);
        info.Message(info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, nullptr,
                     msgItems, ARRAYSIZE(msgItems), 0);
        return false;
    }
    if (l_user.empty() && l_password.empty() && l_askPassword)
    {
        const wchar_t* msgItems[2];
        msgItems[0] = info.GetMsg(info.ModuleNumber, MError);
        msgItems[1] = info.GetMsg(info.ModuleNumber, MAnonymousConnectionPassword);
        info.Message(info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, nullptr,
                     msgItems, ARRAYSIZE(msgItems), 0);
        return false;
    }
    mountPoint.setUrl(l_url);
    mountPoint.setUser(l_user);
    mountPoint.setAskPassword(l_askPassword);
    if (mountPoint.getAskPassword()) l_password.clear();
    mountPoint.setPassword(l_password);
    return true;
}

bool AskPasswordDlg(PluginStartupInfo& info, MountPoint& mountPoint)
{
    const int DIALOG_WIDTH = 48;
    const int DIALOG_HEIGHT = 7;
    std::vector<InitDialogItem> initItems = {
        { DI_DOUBLEBOX, 2, 1, DIALOG_WIDTH - 3, DIALOG_HEIGHT - 2, 0, 0, 0, 0,
          MResourceMount, L"", 0 },

        { DI_TEXT, 4, 2, 0, 2, 0, 0, 0, 0,
          MPassword, L"", 0 },
        { DI_PSWEDIT, 4, 3, DIALOG_WIDTH - 5, 3, 1, 0, 0, 0,
          -1, L"", 0 },

        { DI_BUTTON, 0, 5, 0, 5, 0, 0, DIF_CENTERGROUP, 1,
          MOk, L"", 0 },
        { DI_BUTTON, 0, 5, 0, 5, 0, 0, DIF_CENTERGROUP, 0,
          MCancel, L"", 0 }
    };
    std::vector<FarDialogItem> dialogItems;
    InitDialogItems(info, initItems, dialogItems);
    startupInfo = &info;

    HANDLE hDlg = info.DialogInit(info.ModuleNumber, -1, -1, DIALOG_WIDTH,
                                  DIALOG_HEIGHT, L"Config", dialogItems.data(),
                                  dialogItems.size(), 0, 0, nullptr, 0);
    int ret = info.DialogRun(hDlg);
    // get user input
    std::wstring l_password = DLG_GET_TEXTPTR(info, hDlg, 2);
    info.DialogFree(hDlg);
    startupInfo = nullptr;
    // check user input
    if ((ret == -1) || (ret == int(initItems.size()) - 1))
    {
        return false;
    }
    mountPoint.setPassword(l_password);
    return true;
}

bool ConfigurationEditDlg(PluginStartupInfo& info)
{
#ifdef USE_SECRET_STORAGE
    const int DIALOG_WIDTH = 56;
    const int DIALOG_HEIGHT = 8;
    std::vector<InitDialogItem> initItems = {
        { DI_DOUBLEBOX, 2, 1, DIALOG_WIDTH - 3, DIALOG_HEIGHT - 2, 0, 0, 0, 0,
          MConfigTitle, L"", 0 },

        { DI_CHECKBOX, 4, 2, 0, 2, 0, Configuration::Instance()->unmountAtExit(),
          0, 0, MConfigUnmountAtExit, L"", 0 },
        { DI_CHECKBOX, 4, 3, 0, 3, 0, Configuration::Instance()->useSecretStorage(),
          0, 0, MConfigUseSecretService, L"", 0 },

        { DI_BUTTON, 0, 5, 0, 5, 0, 0, DIF_CENTERGROUP, 1,
          MOk, L"", 0 },
        { DI_BUTTON, 0, 5, 0, 5, 0, 0, DIF_CENTERGROUP, 0,
          MCancel, L"", 0 }
    };
#else
    const int DIALOG_WIDTH = 50;
    const int DIALOG_HEIGHT = 7;
    std::vector<InitDialogItem> initItems = {
        { DI_DOUBLEBOX, 2, 1, DIALOG_WIDTH - 3, DIALOG_HEIGHT - 2, 0, 0, 0, 0,
          MConfigTitle, L"", 0 },

        { DI_CHECKBOX, 4, 2, 0, 2, 0, Configuration::Instance()->unmountAtExit(),
          0, 0, MConfigUnmountAtExit, L"", 0 },

        { DI_BUTTON, 0, 4, 0, 4, 0, 0, DIF_CENTERGROUP, 1,
          MOk, L"", 0 },
        { DI_BUTTON, 0, 4, 0, 4, 0, 0, DIF_CENTERGROUP, 0,
          MCancel, L"", 0 }
    };
#endif
    std::vector<FarDialogItem> dialogItems;
    InitDialogItems(info, initItems, dialogItems);
    startupInfo = &info;

    HANDLE hDlg = info.DialogInit(info.ModuleNumber, -1, -1, DIALOG_WIDTH,
                                  DIALOG_HEIGHT, L"Config", dialogItems.data(),
                                  dialogItems.size(), 0, 0, nullptr, 0);
    int ret = info.DialogRun(hDlg);
    // get user input
    bool l_unmountAtExit = DLG_GET_CHECKBOX(info, hDlg, 1);
#ifdef USE_SECRET_STORAGE
    bool l_useSecretStorage = DLG_GET_CHECKBOX(info, hDlg, 2);
#endif
    info.DialogFree(hDlg);
    startupInfo = nullptr;
    // check user input
    if ((ret == -1) || (ret == int(initItems.size()) - 1))
    {
        return false;
    }
    Configuration::Instance()->setUnmountAtExit(l_unmountAtExit);
#ifdef USE_SECRET_STORAGE
    Configuration::Instance()->setUseSecretStorage(l_useSecretStorage);
#endif
    return true;
}

bool AskQuestionDlg(PluginStartupInfo& info,
                    const int DIALOG_WIDTH,
                    const std::vector<std::wstring>& message,
                    const std::vector<std::wstring>& choices,
                    unsigned int& choice)
{
    // 2 rows - frame, 2 rows - buttons, total: 2 * 2 + 2
    // first line in message - title
    int DIALOG_HEIGHT = 6 + message.size();
    std::vector<InitDialogItem> initItems = {
        { DI_DOUBLEBOX, 2, 1, DIALOG_WIDTH - 3, DIALOG_HEIGHT - 2, 0, 0, 0, 0,
          -1, message[0], 0 },

        { DI_BUTTON, 0, int(message.size() + 3), 0, int(message.size() + 1),
          0, 0, DIF_CENTERGROUP, 1, MOk, L"", 0 },
        { DI_BUTTON, 0, int(message.size() + 3), 0, int(message.size() + 1),
          0, 0, DIF_CENTERGROUP, 0, MCancel, L"", 0 }
    };
    // add message lines to dialog
    std::vector<InitDialogItem>::iterator pos = initItems.begin();
    ++pos;
    for (unsigned int i = 1; i < message.size(); i++)
    {
        InitDialogItem item = {
            DI_TEXT, 4, int(1 + i), 0, int(1 + i), 0, 0, 0, 0, -1,
            message[i], 0
        };
        pos = initItems.insert(pos, item);
        ++pos;
    }
    // add choices list
    std::vector<FarListItem> choicesList;
    unsigned int choicesWidth = 0;
    // max width limited in UiCallbacks::onAskQuestion()
    for (unsigned int i = 0; i < choices.size(); i++)
    {
        FarListItem item = {
          ((i == choice) ? LIF_SELECTED : 0), choices[i].c_str(), { 0 }
        };
        choicesWidth = (choicesWidth < choices[i].size()) ?
                       choices[i].size() : choicesWidth;
        choicesList.push_back(item);
    }
    FarList listInfo = { int(choicesList.size()), choicesList.data() };
    {
        InitDialogItem item = {
             DI_COMBOBOX,
             4, int(message.size() + 1),
             int(4 + choicesWidth), int(message.size() + 1),
             0, 0, DIF_DROPDOWNLIST | DIF_SHOWAMPERSAND, 0, -1, L"", 0
        };
        item.ListItems = &listInfo;
        pos = initItems.insert(pos, item);
    }
    ++pos; // for future use
    std::vector<FarDialogItem> dialogItems;
    InitDialogItems(info, initItems, dialogItems);
    startupInfo = &info;
    HANDLE hDlg = info.DialogInit(info.ModuleNumber, -1, -1, DIALOG_WIDTH,
                                  DIALOG_HEIGHT, L"Config", dialogItems.data(),
                                  dialogItems.size(), 0, 0, nullptr, 0);
    int ret = info.DialogRun(hDlg);
    // get user input
    choice = DLG_GET_COMBOBOXPOS(info, hDlg, message.size());
    info.DialogFree(hDlg);
    startupInfo = nullptr;
    // check user input
    if ((ret == -1) || (ret == int(initItems.size()) - 1))
    {
        return false;
    }
    return true;
}
