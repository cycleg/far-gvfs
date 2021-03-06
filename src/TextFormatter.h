#pragma once

#include <string>
#include <vector>

/// 
/// @brief Вспомогательный класс для форматирования текстовых строк.

/// Реализует форматирование текста по нескольким параметрам:
///
/// * длина строки;
/// * перенос по словам (да/нет);
/// * пропуск пустых строк (да/нет);
/// * сброс не помещающегося текста и отметка этого строкой #dropMark (да/нет).
///
/// Два основных действия:
///
/// * обработать по параметрам текст из нескольких строк (абзацев) и
///   вернуть отформатированный текст;
/// * обработать текст и вернуть одну (первую) строку.
///
/// При включенном переносе по словам, если длина слова оказывается больше
/// длины строки и отбрасывание текста отключено, то слово вставляется в
/// выходную строку целиком.
///
/// Все разделители строк заменяются на пробел. Разделители строк в выходной
/// текст не включаются.
///
/// @author cycleg
///
class TextFormatter
{
  public:
    ///
    /// Конструктор.
    ///
    TextFormatter();

    ///
    /// Задать ширину строки текста.
    ///
    /// @param [in] w Новая ширина строки.
    /// @return Ссылка на объект.
    ///
    inline TextFormatter& TextWidth(unsigned int w)
    { m_textWidth = w; return *this; }
    ///
    /// Включить/выключить перенос текста по словам.
    ///
    /// @param [in] w true -- перенос включен, false -- строка разбивается
    ///               произвольно.
    /// @return Ссылка на объект.
    ///
    inline TextFormatter& WordWrap(bool w) { m_wordWrap = w; return *this; }
    ///
    /// Включить/выключить пропуск пустых строк в оригинальном тексте.
    ///
    /// @param [in] i true -- строки пропускаются, false -- строки включаются
    ///               в выходной текст.
    /// @return Ссылка на объект.
    ///
    inline TextFormatter& IgnoreEmptyLines(bool i)
    { m_ignoreEmptyLines = i; return *this; }
    ///
    /// Включить/выключить отбрасывание части оригинального текста, не
    /// укладывающейся в формат.
    ///
    /// @param [in] d true -- текст отбрасывается, false -- нет.
    /// @return Ссылка на объект.
    ///
    /// Отброшенный текст заменяется на #dropMark.
    ///
    inline TextFormatter& DropRemain(bool d) { m_dropRemain = d; return *this; }

    ///
    /// Форматирование текста с возвратом набора строк.
    ///
    /// @param [in] text Оригинальный текст в виде одной большой строки.
    /// @param [out] lines Набор переформатированных строк.
    ///
    /// Если в оригинале длина строки меньше заданной, то строка не
    /// переформатируется.
    ///
    void FitToLines(const std::wstring& text, std::vector<std::wstring>& lines) const;
    ///
    /// Форматирование первой строки оригинального текста.
    ///
    /// @param [in] text Оригинальный текст в виде одной большой строки.
    /// @return Отформатированная строка.
    ///
    /// Метод может вернуть пустую строку, если игнорирование их выключено.
    ///
    std::wstring FitToLine(const std::wstring& text) const;

  private:
    static const wchar_t LinesDelimiter = '\n'; ///< Разделитель строк в исходном тексте.
    static const wchar_t* WordDelimiters; ///< Разделители слов в исходном тексте.
    static const std::wstring dropMark; ///< Метка отброшенного исходного текста.

    ///
    /// Разбить оригинальный текст на строки.
    ///
    /// @param [in] text Оригинальный текст в виде одной большой строки.
    /// @param [out] lines Набор строк.
    ///
    /// Символ "\r" в выходной текст не включается.
    ///
    void SplitLines(const std::wstring& text, std::vector<std::wstring>& lines) const;
    ///
    /// Разбить строку на слова.
    ///
    /// @param [in] text Оригинальный текст в виде строки.
    /// @param [in] words Набор строк.
    ///
    /// Несколько разделителей слов подряд трактуются как один.
    ///
    void SplitWords(const std::wstring& text, std::vector<std::wstring>& words) const;

    unsigned int m_textWidth; ///< Ширина строки.
    bool m_wordWrap; ///< Флаг переноса по словам.
    bool m_ignoreEmptyLines; ///< Флаг игнорирования пустых строк.
    bool m_dropRemain; ///< Флаг отбрасывания оригинального текста.
};
