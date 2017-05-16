///
/// @file gvfsdlg.h
///
#pragma once

#include <string>
#include <vector>
#include "plugin.hpp"
#include "MountPoint.h"

///
/// @brief Структура-описание элемента диалога.

/// Преобразуется в структуру FarDialogItem.
///
/// @authors invy, cycleg
///
struct InitDialogItem
{
    int Type; ///< Тип элемента диалога.
    int X1; ///< X-координата левого верхнего угла.
    int Y1; ///< Y-координата левого верхнего угла.
    int X2; ///< X-координата правого нижнего угла.
    int Y2; ///< Y-координата правого нижнего угла.
    int Focus; ///< В фокусе элемент илли нет.
    /// Разная интерпретация для разных типов элементов диалога.
    union
    {
        DWORD_PTR Reserved; ///< Для будущего использования.
        int Selected; ///< Для CheckBox включен он или выключен.
        struct FarList* ListItems; ///< Список вариантов для ListBox или ComboBox.
    };
    unsigned int Flags; ///< Флаги элемента диалога.
    int DefaultButton; ///< Элемент является кнопкой по умолчанию.
    int lngIdx; ///< Индекс строки в языковом файле, подставляемого в
                ///< качестве текста элемента. Может быть равен -1, в этом
                ///< случае заменяется на поле #text.
    std::wstring text; ///< Текст элемента. Игнорируется, если указан
                       ///< существующий lngIdx.
    int maxLen; ///< Максимальная длина значения элемента, например, поля
                ///< ввода. 0 -- без ограничений.
};

///
/// Редактирование ресурса для монтирования.
///
/// @param [in] info
/// @param [in, out] mountPoint Редактируемый ресурс.
/// @return false, если диалог прерван.
///
/// @authors invy, cycleg
///
bool EditResourceDlg(PluginStartupInfo& info, MountPoint& mountPoint);
///
/// Запрос пароля перед монтированием ресурса.
///
/// @param [in] info
/// @param [in, out] mountPoint Монтируемый ресурс.
/// @return false, если диалог прерван.
///
/// @author cycleg
///
bool AskPasswordDlg(PluginStartupInfo& info, MountPoint& mountPoint);
///
/// Редактирование общих настроек плагина.
///
/// @param [in] info
/// @return false, если диалог прерван.
///
/// @author cycleg
///
bool ConfigurationEditDlg(PluginStartupInfo& info);
///
/// Ответ на вопрос в ходе монтирования ресурса.
///
/// @param [in] info
/// @param [in] DIALOG_WIDTH Ширина диалога.
/// @param [in] message Запрос пользователю.
/// @param [in] choices Варианты ответа
/// @param [in, out] choice
/// @return false, если диалог прерван.
///
/// @author cycleg
///
bool AskQuestionDlg(PluginStartupInfo& info,
                    const int DIALOG_WIDTH,
                    const std::vector<std::wstring>& message,
                    const std::vector<std::wstring>& choices,
                    unsigned int& choice);
