#pragma once

#include <map>
#include <memory>
#include <vector>
#include <windows.h>
#include "plugin.hpp"
#include "KeyBarTitlesHelper.h"
#include "MountPoint.h"

///
/// Параметры FAR-плагина
///
struct Options
{
    bool AddToDisksMenu;
    bool AddToPluginsMenu;
    int CommonPanel;
    int SafeModePanel;
    int AnyInPanel;
    int CopyContents;
    int Mode;
    int MenuForFilelist;
    int NewPanelForSearchResults;
    int FullScreenPanel;
    int LastSearchResultsPanel;
    int SelectedCopyContents;
    wchar_t ColumnTypes[64];
    wchar_t ColumnWidths[64];
    wchar_t StatusColumnTypes[64];
    wchar_t StatusColumnWidths[64];
    wchar_t Mask[512];
    wchar_t Prefix[16];
};

/// 
/// Класс-реализация плагина.

/// Реализован как синглетон.
/// 
/// @author cycleg
///
class Plugin
{
public:
    static Plugin& getInstance();

    Plugin();
    ~Plugin();

    int getVersion();
    void setStartupInfo(const struct PluginStartupInfo* m_pPsi);
    void exitFar();
    void getPluginInfo(PluginInfo* pi);
    int configure(int item);
    HANDLE openPlugin(int openFrom, intptr_t item);
    void closePlugin(HANDLE Plugin);
    void getOpenPluginInfo(HANDLE Plugin, OpenPluginInfo* pluginInfo);
    int getFindData(HANDLE Plugin, PluginPanelItem** PanelItem, int* itemsNumber, int OpMode);
    void freeFindData(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber);
    int processHostFile(HANDLE Plugin, struct PluginPanelItem* PanelItem, int itemsNumber, int OpMode);
    int processKey(HANDLE Plugin, int key, unsigned int controlState);
    int processEvent(HANDLE Plugin, int Event, void* Param);
    int setDirectory(HANDLE Plugin, const wchar_t* Dir, int OpMode);
    int makeDirectory(HANDLE Plugin, const wchar_t** Name, int OpMode);
    int deleteFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int OpMode);
    int getFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int Move, const wchar_t** dstPath, int OpMode);
    int putFiles(HANDLE Plugin, PluginPanelItem* PanelItem, int itemsNumber, int Move, const wchar_t* srcPath, int OpMode);
    int processEditorEvent(int Event, void* Param);
    int processEditorInput(const INPUT_RECORD* Rec);

private:
    ///
    /// Очистить набор отображаемых в панели элементов.
    ///
    void clearPanelItems();
    ///
    /// Заполнить набор отображаемых в панели элементов данными из набора
    /// ресурсов.
    ///
    void updatePanelItems();
    ///
    /// Отсоединить указанный ресурс.
    ///
    /// @param point Класс-описание ресурса
    ///
    void unmountResource(MountPoint& point);
    ///
    /// Извлечь выбранный элемент панели.
    ///
    /// @param Plugin Указатель на структуру плагина в FAR
    /// @return указатель на выбранный лемент или nullptr, если элементов нет
    ///
    PluginPanelItem* getPanelCurrentItem(HANDLE Plugin);
    ///
    /// Преверить статус соединения со всеми ресурсами.
    ///
    void checkResourcesStatus();

    Options Opt;
    KeyBarTitlesHelper m_keyBar; ///< элемент управления посказками о
                                 ///< функциональных кнопках клавиатуры
    PluginStartupInfo m_pPsi;
    std::wstring m_registryRoot; ///< имя корневой папки плагина в реестре FAR
    std::vector<PluginPanelItem> m_items; ///< набор элементов, отображаемых в
                                          ///< панели плагина
    std::map<std::wstring, MountPoint> m_mountPoints; ///< Набор ресурсов для
                                                      ///< монтирования. Ключ -
                                                      ///< URL ресурса.
    bool m_firstDemand; ///< флаг того, что панель ранее не открывали
};
