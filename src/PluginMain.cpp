#include "Plugin.h"

#include <windows.h>

#include <string>
#include <algorithm>
#include <vector>

extern "C"
{

SHAREDSYMBOL int WINAPI _export GetMinFarVersionW()
{
    return Plugin::getInstance().getVersion();
}

SHAREDSYMBOL void WINAPI _export SetStartupInfoW(const struct PluginStartupInfo * psi)
{
    Plugin::getInstance().setStartupInfo(psi);
}

SHAREDSYMBOL void WINAPI _export ExitFARW()
{
    Plugin::getInstance().exitFar();
}

SHAREDSYMBOL void WINAPI _export GetPluginInfoW(struct PluginInfo * pi)
{
    Plugin::getInstance().getPluginInfo(pi);
}

SHAREDSYMBOL int WINAPI _export ConfigureW(int item)
{
    return Plugin::getInstance().configure(item);
}

SHAREDSYMBOL HANDLE WINAPI _export OpenPluginW(int openFrom, INT_PTR item)
{
    return Plugin::getInstance().openPlugin(openFrom, item);
}

SHAREDSYMBOL void WINAPI _export ClosePluginW(HANDLE Plugin)
{
    Plugin::getInstance().closePlugin(Plugin);
}

SHAREDSYMBOL void WINAPI _export GetOpenPluginInfoW(HANDLE Plugin, struct OpenPluginInfo * pluginInfo)
{
    Plugin::getInstance().getOpenPluginInfo(Plugin, pluginInfo);
}

SHAREDSYMBOL int WINAPI _export GetFindDataW(HANDLE Plugin, struct PluginPanelItem ** PanelItem, int * itemsNumber, int OpMode)
{
    return Plugin::getInstance().getFindData(Plugin, PanelItem, itemsNumber, OpMode);
}

SHAREDSYMBOL void WINAPI _export FreeFindDataW(HANDLE Plugin, struct PluginPanelItem * PanelItem, int itemsNumber)
{
    Plugin::getInstance().freeFindData(Plugin, PanelItem, itemsNumber);
}

SHAREDSYMBOL int WINAPI _export ProcessHostFileW(HANDLE Plugin,
                            struct PluginPanelItem * PanelItem, int ItemsNumber, int OpMode)
{
    return Plugin::getInstance().processHostFile(Plugin, PanelItem, ItemsNumber, OpMode);
}

SHAREDSYMBOL int WINAPI _export ProcessKeyW(HANDLE Plugin, int key, unsigned int controlState)
{
    return Plugin::getInstance().processKey(Plugin, key, controlState);
}

SHAREDSYMBOL int WINAPI _export ProcessEventW(HANDLE Plugin, int Event, void * Param)
{
    return Plugin::getInstance().processEvent(Plugin, Event, Param);
}

SHAREDSYMBOL int WINAPI _export SetDirectoryW(HANDLE Plugin, const wchar_t * Dir, int OpMode)
{
    return Plugin::getInstance().setDirectory(Plugin, Dir, OpMode);
}

SHAREDSYMBOL int WINAPI _export MakeDirectoryW(HANDLE Plugin, const wchar_t ** Name, int OpMode)
{
    return Plugin::getInstance().makeDirectory(Plugin, Name, OpMode);
}

SHAREDSYMBOL int WINAPI _export DeleteFilesW(HANDLE Plugin, struct PluginPanelItem * PanelItem, int itemsNumber, int OpMode)
{
    return Plugin::getInstance().deleteFiles(Plugin, PanelItem, itemsNumber, OpMode);
}

SHAREDSYMBOL int WINAPI _export GetFilesW(HANDLE Plugin, struct PluginPanelItem * PanelItem, int itemsNumber,
                     int Move, const wchar_t ** destPath, int OpMode)
{
    return Plugin::getInstance().getFiles(Plugin, PanelItem, itemsNumber,
                                          Move, destPath, OpMode);
}

SHAREDSYMBOL int WINAPI _export PutFilesW(HANDLE Plugin, struct PluginPanelItem * PanelItem, int itemsNumber, int Move, const wchar_t * SrcPath, int OpMode)
{
    return Plugin::getInstance().putFiles(Plugin, PanelItem, itemsNumber,
                                          Move, SrcPath, OpMode);
}

SHAREDSYMBOL int WINAPI _export ProcessEditorEventW(int Event, void * Param)
{
    return Plugin::getInstance().processEditorEvent(Event, Param);
}

SHAREDSYMBOL int WINAPI _export ProcessEditorInputW(const INPUT_RECORD * Rec)
{
    return Plugin::getInstance().processEditorInput(Rec);
}

SHAREDSYMBOL HANDLE WINAPI _export OpenFilePluginW(const wchar_t * fileName, const unsigned char * fileHeader, int fileHeaderSize, int OpMode)
{
    (void)fileName;
    (void)fileHeader;
    (void)fileHeaderSize;
    (void)OpMode;

    return INVALID_HANDLE_VALUE;
#if 0
    if (fileName == nullptr)
    {
        return INVALID_HANDLE_VALUE;
    }
    std::wstring name(fileName);
    std::wstring ext(L".gvfsmounts");
    if(!std::equal(name.rbegin(), name.rbegin()+ext.size(), ext.rbegin(), ext.rend()))
    {
        return INVALID_HANDLE_VALUE;
    }
    std::vector<uint8_t> fHdr(fileHeader, fileHeader+fileHeaderSize);
    std::vector<uint8_t> xmlHdr{'<', '?', 'x', 'm', 'l'};
    if (!std::equal(xmlHdr.begin(), xmlHdr.end(), fHdr.begin(), fHdr.begin() + xmlHdr.size()))
    {
        return INVALID_HANDLE_VALUE;
    }
    return Plugin::getInstance().openPlugin(OPEN_ANALYSE,
                                            reinterpret_cast<intptr_t>(fileName));
#endif
}

__attribute__((constructor)) void so_init(void)
{
#if 0
    FarPlugin = CreateFarPlugin(0);
#endif
}

} // extern "C"
