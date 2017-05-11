#include <iostream> // debug output
#include <utils.h>
#include "Configuration.h"
#include "gvfsdlg.h"
#include "GvfsService.h"
#include "LngStringIDs.h"
#include "MountPointStorage.h"
#include "UiCallbacks.h"
#include "Plugin.h"

#define UNUSED(x) (void)x;

// чтобы TEXT из WinCompat.h работал с макросом
#define MACRO_TEXT(s) TEXT(s)

Plugin& Plugin::getInstance()
{
    static Plugin instance;
    return instance;
}

Plugin::Plugin():
  m_firstDemand(true)
{
    Opt.AddToDisksMenu = true;
    Opt.AddToPluginsMenu = true;
}

Plugin::~Plugin()
{
    clearPanelItems();
}

int Plugin::getVersion()
{
    return 1;
}

void Plugin::setStartupInfo(const PluginStartupInfo* psi)
{
    static const wchar_t* emptyHint = L"";
    m_pPsi = *psi;

    // key bar
    // all blocked in processKey()
    m_keyBar.setNormalKey(2, emptyHint)
            .setNormalKey(4, emptyHint)
            .setNormalKey(5, emptyHint);
    for (int i = 0; i < 3; i++) m_keyBar.setShiftKey(i, emptyHint);
    m_keyBar.setShiftKey(4, emptyHint)
            .setShiftKey(5, emptyHint);
    for (int i = 2; i < 6; i++) m_keyBar.setAltKey(i, emptyHint);

    m_keyBar.setNormalKey(6, m_pPsi.GetMsg(m_pPsi.ModuleNumber, MF7Bar))
            .setShiftKey(3, m_pPsi.GetMsg(m_pPsi.ModuleNumber, MF7Bar))
            .setShiftKey(7, m_pPsi.GetMsg(m_pPsi.ModuleNumber, MShiftF8Bar));
    m_registryRoot.append(m_pPsi.RootKey);
    m_registryRoot.append(WGOOD_SLASH);
    m_registryRoot.append(MACRO_TEXT(PLUGIN_NAME));

    // load configuration from registry
    Configuration::Instance(m_registryRoot);
    // load mount points from registry
    MountPointStorage storage(m_registryRoot);
    storage.LoadAll(m_mountPoints);
}

void Plugin::exitFar()
{
    if (Configuration::Instance()->unmountAtExit())
    {
        // unmount all VFS, mounted in current session
        for (auto& mntPoint : m_mountPoints)
        {
            if (mntPoint.second.isMounted())
                try
                {
                    GvfsService service;
                    mntPoint.second.unmount(&service);
                }
                catch (const GvfsServiceException& error)
                {
                    // ignore error here
                }
        }
    }
}

void Plugin::getPluginInfo(PluginInfo* info)
{
    info->StructSize = sizeof(*info);
    info->Flags = PF_PRELOAD;

    static const wchar_t* DiskMenuStrings[1];
    DiskMenuStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MDiskMenuString);
    info->DiskMenuStrings = DiskMenuStrings;
    info->DiskMenuStringsNumber = Opt.AddToDisksMenu ? ARRAYSIZE(DiskMenuStrings) : 0;

    static const wchar_t* PluginMenuStrings[1];
    PluginMenuStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    info->PluginMenuStrings = Opt.AddToPluginsMenu ? PluginMenuStrings : NULL;
    info->PluginMenuStringsNumber = Opt.AddToPluginsMenu ? ARRAYSIZE(PluginMenuStrings) : 0;

    static const wchar_t* PluginCfgStrings[1];
    PluginCfgStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    info->PluginConfigStrings = PluginCfgStrings;
    info->PluginConfigStringsNumber = ARRAYSIZE(PluginCfgStrings);
    info->CommandPrefix = Opt.Prefix;
}

int Plugin::configure(int item)
{
    UNUSED(item)

    bool ret = ConfigurationEditDlg(m_pPsi);
    if (ret) Configuration::Instance()->save();
    return ret;
}

HANDLE Plugin::openPlugin(int openFrom, intptr_t item)
{
    UNUSED(openFrom)
    UNUSED(item)

    return static_cast<HANDLE>(this);
}

void Plugin::closePlugin(HANDLE Plugin)
{
    UNUSED(Plugin)
}

void Plugin::getOpenPluginInfo(HANDLE Plugin, OpenPluginInfo* pluginInfo)
{
    UNUSED(Plugin)

    static const wchar_t* pluginPanelTitle = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    pluginInfo->StructSize = sizeof(*pluginInfo);
    pluginInfo->PanelTitle = pluginPanelTitle;
    // panel modes
    static struct PanelMode PanelModesArray[10];
    static const wchar_t* ColumnTitles[3] = { NULL };
    memset(&PanelModesArray, 0, sizeof(PanelModesArray));
    ColumnTitles[1] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MResourceTitle);
    ColumnTitles[2] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MUser);
    PanelModesArray[0].ColumnTypes = L"C0,C1,C2";
    PanelModesArray[0].ColumnWidths = L"1,0,0";
    PanelModesArray[0].ColumnTitles = ColumnTitles;
    PanelModesArray[0].StatusColumnTypes = PanelModesArray[0].ColumnTypes;
    PanelModesArray[0].StatusColumnWidths = PanelModesArray[0].ColumnWidths;
    pluginInfo->PanelModesArray = PanelModesArray;
    pluginInfo->PanelModesNumber = ARRAYSIZE(PanelModesArray);
    pluginInfo->StartPanelMode = _T('0');
    pluginInfo->KeyBar = &(m_keyBar.getKeyBar());
}

int Plugin::getFindData(HANDLE Plugin, PluginPanelItem** PanelItem, int* itemsNumber, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(OpMode)

    if (m_firstDemand)
    {
      m_firstDemand = false;
      checkResourcesStatus();
    }
    updatePanelItems();
    *PanelItem = m_items.empty() ? nullptr : m_items.data();
    *itemsNumber = m_items.size();
    return 1;
}

void Plugin::freeFindData(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber)
{
    UNUSED(Plugin)
    UNUSED(PanelItem)
    UNUSED(itemsNumber)
}

int Plugin::processHostFile(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(PanelItem)
    UNUSED(itemsNumber)
    UNUSED(OpMode)

    return 0;
}

int Plugin::processKey(HANDLE Plugin, int key, unsigned int controlState)
{
std::cerr << "Plugin::processKey() key = " << key << std::endl;
    if (((controlState == 0) && ((key == VK_F3) || (key == VK_F5) || (key == VK_F6))) ||
        ((controlState == PKF_SHIFT) && 
         (((key >= VK_F1) && (key <= VK_F3)) || (key == VK_F5) || (key == VK_F6))) ||
        ((controlState == PKF_ALT) && (key >= VK_F3) && (key <= VK_F6)))
    {
        // block keys
        return 1;
    }
    else if ((controlState == 0) && (key == VK_F4))
    {
        // edit resource
        PluginPanelItem* item = getPanelCurrentItem(Plugin);
        if (!item) return 1; // no item, drop key
        std::wstring name = item->CustomColumnData[1];
        free(item);
        auto it = m_mountPoints.find(name);
        if (it != m_mountPoints.end())
        {
            if (it->second.isMounted())
            {
                const wchar_t* msgItems[2] = { NULL };
                msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MResourceTitle);
                msgItems[1] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MFirstUnmountResource);
                m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_OK,
                               NULL, msgItems, ARRAYSIZE(msgItems), 0);
                return 1;
            }
            if (EditResourceDlg(m_pPsi, it->second))
            {
                MountPointStorage storage(m_registryRoot);
                MountPoint changedMountPt = it->second;
                m_mountPoints.erase(it);
                m_mountPoints.insert(std::pair<std::wstring, MountPoint>(
                    changedMountPt.getResPath(), changedMountPt
                ));
                // TODO: save error
                storage.Save(changedMountPt);
                m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
            }
        }
        return 1; // return 1: far should not handle this key
    }
    else if ((controlState == PKF_SHIFT) && (key == VK_F4))
    {
        // add new resource
        MountPoint point(MountPointStorage::PointFactory());
        if (EditResourceDlg(m_pPsi, point))
        {
            MountPointStorage storage(m_registryRoot);
            m_mountPoints.insert(std::pair<std::wstring, MountPoint>(
                point.getResPath(), point
            ));
            // TODO: save error
            storage.Save(point);
            m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
        }
        return 1;
    }
    else if ((controlState == PKF_SHIFT) && (key == VK_F8))
    {
        // unmount selected resource
        PluginPanelItem* item = getPanelCurrentItem(Plugin);
        if (!item) return 1; // no item, drop key
        std::wstring name = item->CustomColumnData[1];
        free(item);
        auto it = m_mountPoints.find(name);
        if ((it != m_mountPoints.end()) && it->second.isMounted())
        {
            unmountResource(it->second);
            m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
            // redraw screen
            m_pPsi.AdvControl(m_pPsi.ModuleNumber, ACTL_REDRAWALL, NULL);
        }
        return 1;
    }
    else if ((controlState == PKF_CONTROL) && (key == 'R'))
    {
        // refresh resources status
        checkResourcesStatus();
        m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
        // redraw screen
        m_pPsi.AdvControl(m_pPsi.ModuleNumber, ACTL_REDRAWALL, NULL);
        return 1;
    }
    return 0; // all other keys left to far
}

int Plugin::processEvent(HANDLE Plugin, int Event, void* Param)
{
    UNUSED(Plugin)
    UNUSED(Event)
    UNUSED(Param)

    return 0;
}

int Plugin::setDirectory(HANDLE Plugin, const wchar_t* Dir, int OpMode)
{
    if(OpMode == 0)
    {
        auto it = m_mountPoints.find(std::wstring(Dir));
        if (it != m_mountPoints.end())
        {
            if (!it->second.isMounted())
            {
                const wchar_t* msgItems[2] = { NULL };
                bool isMount = false;
                HANDLE hScreen = nullptr;
                if (it->second.getAskPassword())
                {
                  if (!AskPasswordDlg(m_pPsi, it->second)) return 0;
                }
                hScreen = m_pPsi.SaveScreen(0, 0, -1, -1);
                try
                {
                    UiCallbacks callbacks(m_pPsi);
                    GvfsService service(&callbacks);
                    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MResourceMount);
                    msgItems[1] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MPleaseWait);
                    m_pPsi.Message(m_pPsi.ModuleNumber, 0, NULL, msgItems,
                                   ARRAYSIZE(msgItems), 0);
                    // для сообщения об ошибке
                    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MMountError);
                    isMount = it->second.mount(&service);
                }
                catch (const GvfsServiceException& error)
                {
                    std::wstring buf(StrMB2Wide(error.what().raw()));
                    msgItems[1] = buf.c_str();
                    m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_OK,
                                   NULL, msgItems, ARRAYSIZE(msgItems), 0);
                }
                m_pPsi.RestoreScreen(hScreen);
                if (!isMount) return 0;
            }
            // change directory to:
            std::wstring dir = it->second.getFsPath();
            if (!dir.empty())
            {
                m_pPsi.Control(Plugin, FCTL_SETPANELDIR, 0, (LONG_PTR)(dir.c_str()));
            }
        }
    }
    return 0;
}

int Plugin::makeDirectory(HANDLE Plugin, const wchar_t** Name, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(Name)
    UNUSED(OpMode)

    // add new resource
    MountPoint point(MountPointStorage::PointFactory());
    if (EditResourceDlg(m_pPsi, point))
    {
        MountPointStorage storage(m_registryRoot);
        m_mountPoints.insert(std::pair<std::wstring, MountPoint>(point.getResPath(), point));
        // TODO: save error
        storage.Save(point);
    }
    return 1;
}

int Plugin::deleteFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(OpMode)

    const wchar_t* msgItems[2] =
    {
        L"Delete selected mounts",
        L"Do you really want to delete mount point?"
    };

    auto msgCode = m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_YESNO,
                                  NULL, msgItems, ARRAYSIZE(msgItems), 0);
    // if no or canceled msg box, do nothing
    if ((msgCode == -1) || (msgCode == 1))
    {
        return -1;
    }

    for (int i = 0; i < itemsNumber; ++i)
    {
        auto name = PanelItem[i].CustomColumnData[1];
        auto it = m_mountPoints.find(name);
        if (it != m_mountPoints.end())
        {
            MountPointStorage storage(m_registryRoot);
            if (it->second.isMounted())
            {
                unmountResource(it->second);
            }
            storage.Delete(it->second);
            m_mountPoints.erase(it);
        }
    }
    return 0;
}

int Plugin::getFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int Move, const wchar_t** dstPath, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(PanelItem)
    UNUSED(itemsNumber)
    UNUSED(Move)
    UNUSED(dstPath)
    UNUSED(OpMode)

    return 0;
}

int Plugin::putFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int Move, const wchar_t* srcPath, int OpMode)
{
    UNUSED(Plugin)
    UNUSED(PanelItem)
    UNUSED(itemsNumber)
    UNUSED(Move)
    UNUSED(srcPath)
    UNUSED(OpMode)

    return 0;
}

int Plugin::processEditorEvent(int Event, void* Param)
{
    UNUSED(Event)
    UNUSED(Param)

    return 0;
}

int Plugin::processEditorInput(const INPUT_RECORD* Rec)
{
    UNUSED(Rec)

    return 0;
}

void Plugin::clearPanelItems()
{
    for (PluginPanelItem& item : m_items)
    {
        free((void*)item.FindData.lpwszFileName);
        item.FindData.lpwszFileName = nullptr;
        for (int i = 0; i < item.CustomColumnNumber; i++)
            free((void*)item.CustomColumnData[i]);
        delete[] item.CustomColumnData;
        item.CustomColumnData = nullptr;
    }
    m_items.clear();
}

void Plugin::updatePanelItems()
{
    clearPanelItems();
    for (const auto& mountPoint : m_mountPoints)
    {
        PluginPanelItem item;
        memset(&item, 0, sizeof(item));
        item.FindData.lpwszFileName = wcsdup(mountPoint.second.getResPath().c_str());
        item.FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        item.CustomColumnNumber = 3;
        wchar_t** data = new wchar_t*[3];
        data[0] = wcsdup(mountPoint.second.isMounted() ? TEXT("*") : TEXT(" ")); //C0
        data[1] = wcsdup(mountPoint.second.getResPath().c_str()); //C1
        data[2] = wcsdup(mountPoint.second.getUser().c_str()); //C2
        item.CustomColumnData = data;
        m_items.push_back(item);
    }
}

void Plugin::unmountResource(MountPoint& point)
{
    const wchar_t* msgItems[2] = { NULL };
    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MUnmountError);
    try
    {
        GvfsService service;
        point.unmount(&service);
    }
    catch (const GvfsServiceException& error)
    {
        std::wstring buf(StrMB2Wide(error.what().raw()));
        msgItems[1] = buf.c_str();
        m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_OK,
                       NULL, msgItems, ARRAYSIZE(msgItems), 0);
    }
}

PluginPanelItem* Plugin::getPanelCurrentItem(HANDLE Plugin)
{
    PluginPanelItem* PPI = nullptr;
    struct PanelInfo pInfo;
    m_pPsi.Control(Plugin, FCTL_GETPANELINFO, 0, (LONG_PTR)&pInfo);
    if (pInfo.ItemsNumber < 1) return PPI; // no items
    PPI = (PluginPanelItem*) malloc(
        m_pPsi.Control(Plugin, FCTL_GETPANELITEM, pInfo.CurrentItem, 0)
    );
    if (PPI)
    {
        m_pPsi.Control(Plugin, FCTL_GETPANELITEM, pInfo.CurrentItem,
                       (LONG_PTR)PPI);
    }
    return PPI;
}

void Plugin::checkResourcesStatus()
{
    HANDLE hScreen = nullptr;
    const wchar_t* msgItems[2] = { NULL };
    hScreen = m_pPsi.SaveScreen(0, 0, -1, -1);
    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MResourceStatus);
    msgItems[1] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MPleaseWait);
    m_pPsi.Message(m_pPsi.ModuleNumber, 0, NULL, msgItems,
                   ARRAYSIZE(msgItems), 0);
    for (auto& mountPoint : m_mountPoints)
    {
        GvfsService service;
        mountPoint.second.mountCheck(&service);
    }
    m_pPsi.RestoreScreen(hScreen);
}
