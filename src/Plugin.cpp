#include <iostream> // debug output
#include <codecvt> // debug output
#include <locale> // debug output
#include "gvfsdlg.h"
#include "LngStringIDs.h"
#include "Plugin.h"

Plugin& Plugin::getInstance()
{
    static Plugin instance;
    return instance;
}

Plugin::Plugin()
{
    Opt.AddToDisksMenu = true;
    Opt.AddToPluginsMenu = true;
//    m_keyBar.setNormalKey(7, L"MkMount");
//    m_keyBar.setNormalKey(4, L"EdMount");
}

Plugin::~Plugin()
{
    clearPanelItems();
}

int Plugin::getVersion()
{
    return 1;
}

void Plugin::setStartupInfo(const PluginStartupInfo *psi)
{
    this->m_pPsi = *psi;
}

void Plugin::exitFar()
{
    // unmount all VFS, mounted in current session
    for (auto& mntPoint : m_mountPoints)
    {
        if (mntPoint.second.isMounted())
            try
            {
                mntPoint.second.unmount();
            }
            catch (const GvfsServiceException& error)
            {
                // ignore error here
            }
    }
}

void Plugin::getPluginInfo(PluginInfo *info)
{

    info->StructSize = sizeof(*info);
    info->Flags = PF_PRELOAD;

    static const wchar_t *DiskMenuStrings[1];
    DiskMenuStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MDiskMenuString);
    info->DiskMenuStrings = DiskMenuStrings;
    info->DiskMenuStringsNumber = Opt.AddToDisksMenu ? ARRAYSIZE(DiskMenuStrings) : 0;

    static const wchar_t *PluginMenuStrings[1];
    PluginMenuStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    info->PluginMenuStrings = Opt.AddToPluginsMenu ? PluginMenuStrings : NULL;
    info->PluginMenuStringsNumber = Opt.AddToPluginsMenu ? ARRAYSIZE(PluginMenuStrings) : 0;

    static const wchar_t *PluginCfgStrings[1];
    PluginCfgStrings[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    info->PluginConfigStrings = PluginCfgStrings;
    info->PluginConfigStringsNumber = ARRAYSIZE(PluginCfgStrings);
    info->CommandPrefix = Opt.Prefix;
}

int Plugin::configure(int item)
{
  (void)item;
  return FALSE;
}

HANDLE Plugin::openPlugin(int openFrom, intptr_t item)
{
    return static_cast<HANDLE>(this);
}

void Plugin::closePlugin(HANDLE Plugin)
{

}

void Plugin::getOpenPluginInfo(HANDLE Plugin, OpenPluginInfo *pluginInfo)
{
//    pluginInfo->KeyBar = &(m_keyBar.getKeyBar());

    static const wchar_t *pluginPanelTitle = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MGvfsPanel);
    pluginInfo->PanelTitle = pluginPanelTitle;
}

int Plugin::getFindData(HANDLE Plugin, PluginPanelItem **PanelItem, int *itemsNumber, int OpMode)
{
    updatePanelItems();
    *PanelItem = m_items.empty() ? nullptr : &(m_items[0]);
    *itemsNumber = m_items.size();
    return 1;
}

void Plugin::freeFindData(HANDLE Plugin, PluginPanelItem *PanelItem, int itemsNumber)
{

}

int Plugin::processHostFile(HANDLE Plugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode)
{
    return 0;
}

int Plugin::processKey(HANDLE Plugin, int key, unsigned int controlState)
{
fprintf(stderr, "Gvfs processKey(): key = %d controlState = %d\n", key, controlState);
fflush(stderr);
    if ((key == VK_F3) || (key == VK_F5) || (key == VK_F6))
    {
        // block keys
        return 1;
    }
    else if ((key == VK_F4) && (controlState == 0))
    {
        // edit mount record
        if (m_items.empty()) return 1; // no items, drop key
        struct PanelInfo pInfo;
        m_pPsi.Control(Plugin, FCTL_GETPANELINFO, 0, (LONG_PTR)&pInfo);
        auto currentItem = pInfo.CurrentItem;
        PluginPanelItem item = m_items[currentItem];
        std::wstring name = item.FindData.lpwszFileName;
        auto it = m_mountPoints.find(name);
        if (it != m_mountPoints.end())
        {
            if (GetLoginData(m_pPsi, it->second))
            {
                MountPoint changedMountPt = it->second;
                m_mountPoints.erase(it);
                m_mountPoints.insert(std::pair<std::wstring, MountPoint>(
                    changedMountPt.getResPath(), changedMountPt
                ));
                m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
            }
        }
        return 1; // return 1: far should not handel this key
    }
    else if ((key == VK_F4) && (controlState & PKF_SHIFT))
    {
        // add new mount record
        MountPoint mntPoint;
        if (GetLoginData(m_pPsi, mntPoint))
        {
            m_mountPoints.insert(std::pair<std::wstring, MountPoint>(
                mntPoint.getResPath(), mntPoint
            ));
            m_pPsi.Control(Plugin, FCTL_UPDATEPANEL, 0, 0);
        }
        return 1;
    }
    return 0; // all other keys left to far
}

int Plugin::processEvent(HANDLE Plugin, int Event, void *Param)
{
    return 0;
}

int Plugin::setDirectory(HANDLE Plugin, const wchar_t *Dir, int OpMode)
{
    if(OpMode == 0)
    {
        auto it = m_mountPoints.find(std::wstring(Dir));
        if (it != m_mountPoints.end())
        {
            if (!it->second.isMounted())
                try
                {
                    if(!it->second.mount())
                        return 0;
                }
                catch (const GvfsServiceException& error)
                {
                    const wchar_t *msgItems[2];
                    std::wstring buf =
                        std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(error.what().raw());
                    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MMountError);
                    msgItems[1] = buf.c_str();
                    m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_OK,
                                   NULL, msgItems, 2, 0);
                    return 0;
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

int Plugin::makeDirectory(HANDLE Plugin, const wchar_t **Name, int OpMode)
{
    // add new mount record
    MountPoint mountPoint;
    if(GetLoginData(this->m_pPsi, mountPoint))
    {
        m_mountPoints.insert(std::pair<std::wstring, MountPoint>(mountPoint.getResPath(), mountPoint));
        return 1;
    }
    return 0;
}

int Plugin::deleteFiles(HANDLE Plugin, PluginPanelItem *PanelItem, int itemsNumber, int OpMode)
{
    const unsigned N = 2;
    const wchar_t *msgItems[N] =
    {
        L"Delete selected mounts",
        L"Do you really want to delete mount point?"
    };

    auto msgCode = m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_YESNO, NULL, msgItems, N, 0);
    // if no or canceled msg box, do nothing
    if ((msgCode == -1) || (msgCode == 1))
    {
        return -1;
    }

    for (int i = 0; i < itemsNumber; ++i)
    {
        auto name = PanelItem[i].FindData.lpwszFileName;
        auto it = m_mountPoints.find(name);
        if (it != m_mountPoints.end())
        {
            if (it->second.isMounted())
                try
                {
                    it->second.unmount();
                }
                catch (const GvfsServiceException& error)
                {
                    const wchar_t *msgItems[2];
                    std::wstring buf =
                        std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(error.what().raw());
                    msgItems[0] = m_pPsi.GetMsg(m_pPsi.ModuleNumber, MUnmountError);
                    msgItems[1] = buf.c_str();
                    m_pPsi.Message(m_pPsi.ModuleNumber, FMSG_WARNING | FMSG_MB_OK,
                                   NULL, msgItems, 2, 0);
                }
            m_mountPoints.erase(it);
        }
    }
    return 0;
}

int Plugin::getFiles(HANDLE Plugin, PluginPanelItem *PanelItem, int itemsNumber, int Move, const wchar_t **destPath, int OpMode)
{
    return 0;
}

int Plugin::putFiles(HANDLE Plugin, PluginPanelItem *PanelItem, int itemsNumber, int Move, const wchar_t *SrcPath, int OpMode)
{
    return 0;
}

int Plugin::processEditorEvent(int Event, void *Param)
{
    return 0;
}

int Plugin::processEditorInput(const INPUT_RECORD *Rec)
{
    return 0;
}

void Plugin::clearPanelItems()
{
    for (PluginPanelItem& item : m_items)
    {
        free((void*)item.FindData.lpwszFileName);
    }
    m_items.clear();
}

void Plugin::updatePanelItems()
{
    clearPanelItems();
    for (const auto& mountPt : m_mountPoints)
    {
        PluginPanelItem item { };
        item.FindData.lpwszFileName = wcsdup(mountPt.second.getResPath().c_str());
        item.FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_ARCHIVE;
        item.CustomColumnNumber = 0;
        m_items.push_back(item);
    }
}
