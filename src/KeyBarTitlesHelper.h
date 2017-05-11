#pragma once

#include "plugin.hpp"

#include <string>

///
/// Вспомогательный класс для назначения подсказок в меню функциональных кнопок

/// Назначает подсказки в меню функциональных кнопок клавиатуры для различных
/// комбинаций кнопок с клавиатурными модификаторами.
///
/// Нумерация кнопок (индексы) ведется от 0, который соответствует кнопке F1.
/// Недопустимые индексы игнорируются.
///
/// Переданные в методы setKey() строки копируются во внутренние буферы.
///
/// @author invy, cycleg
///
class KeyBarTitlesHelper {
public:
    ///
    /// Типы комбинаций кнопок с модификаторами.
    ///
    enum class KeyType {
        NORMAL, ///< нажатие без модификаторов
        CTRL,
        ALT,
        SHIFT,
        CTRL_SHIFT,
        ALT_SHIFT,
        CTRL_ALT
    };

public:
    KeyBarTitlesHelper();
    ~KeyBarTitlesHelper();

    ///
    /// @return Ссылка на набор подсказок
    ///
    KeyBarTitles& getKeyBar();
    ///
    /// Назначить подсказку для кнопки с модификатором.
    ///
    /// @param index Номер кнопки
    /// @param type Комбинация модификаторов
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setKey(int index, KeyType type, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки без модификатора.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setNormalKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Ctrl.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setCtrlKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Alt.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setAltKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Shift.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Ctrl+Shift.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setCtrlShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Alt+Shift.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setAltShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Ctrl+Alt.
    ///
    /// @param index Номер кнопки
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setCtrlAltKey(int index, const std::wstring& hint);

private:
    ///
    /// Назначить подсказку для кнопки в массиве подсказок.
    ///
    /// @param index Номер кнопки
    /// @param key массив подсказок
    /// @param hint Текст подсказки
    /// @return Ссылка на экземпляр класса
    ///
    /// Массив из структуры KeyBarTitles соответствует одной из комбинаций
    /// модификаторов.
    ///
    KeyBarTitlesHelper& setKeyGeneric(int index, wchar_t* key[],
                                      const std::wstring& hint);

    KeyBarTitles m_keyBar; ///< набор подсказок
};
