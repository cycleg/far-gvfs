#pragma once

#include <string>

///
/// Вспомогательный класс для назначения подсказок в меню функциональных кнопок

/// Назначает подсказки в меню функциональных кнопок клавиатуры для различных
/// комбинаций кнопок с клавиатурными модификаторами.
///
/// Нумерация кнопок (индексирование) ведется от 0, который соответствует
/// кнопке F1. Недопустимые индексы игнорируются.
///
/// Переданные в методы семейства %setKey() строки копируются во внутренние
/// буферы.
///
/// @authors invy, cycleg
///
class KeyBarTitlesHelper {
public:
    ///
    /// Типы комбинаций кнопок с модификаторами.
    ///
    enum class KeyType {
        NORMAL, ///< Нажатие без модификаторов.
        CTRL, ///< Нажатие с Ctrl.
        ALT, ///< Нажатие с Alt.
        SHIFT, ///< Нажатие с Shift.
        CTRL_SHIFT, ///< Нажатие с Ctrl+Shift.
        ALT_SHIFT, ///< Нажатие с Alt+Shift.
        CTRL_ALT ///< Нажатие с Ctrl+Alt.
    };

public:
    ///
    /// Конструктор.
    ///
    KeyBarTitlesHelper();
    ///
    /// Деструктор.
    ///
    ~KeyBarTitlesHelper();

    ///
    /// @return Ссылка на набор подсказок.
    ///
    KeyBarTitles& getKeyBar();
    ///
    /// Назначить подсказку для кнопки с модификатором.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] type Комбинация модификаторов.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setKey(int index, KeyType type, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки без модификатора.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setNormalKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Ctrl.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setCtrlKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Alt.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setAltKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификатором Shift.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Ctrl+Shift.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setCtrlShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Alt+Shift.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    KeyBarTitlesHelper& setAltShiftKey(int index, const std::wstring& hint);
    ///
    /// Назначить подсказку для кнопки с модификаторами Ctrl+Alt.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса
    ///
    KeyBarTitlesHelper& setCtrlAltKey(int index, const std::wstring& hint);

private:
    ///
    /// Назначить подсказку для кнопки в массиве подсказок.
    ///
    /// @param [in] index Номер кнопки.
    /// @param [in] key массив подсказок.
    /// @param [in] hint Текст подсказки.
    /// @return Ссылка на экземпляр класса.
    ///
    /// Массив из структуры KeyBarTitles соответствует одной из комбинаций
    /// модификаторов.
    ///
    KeyBarTitlesHelper& setKeyGeneric(int index, wchar_t* key[],
                                      const std::wstring& hint);

    KeyBarTitles m_keyBar; ///< Набор подсказок.
};
