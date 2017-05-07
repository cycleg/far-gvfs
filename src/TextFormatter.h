#pragma once

#include <string>
#include <vector>

class TextFormatter
{
  public:
    TextFormatter();

    inline TextFormatter& TextWidth(int w) { m_textWidth = w; return *this; }
    inline TextFormatter& WordWrap(bool w) { m_wordWrap = w; return *this; }
    inline TextFormatter& IgnoreEmptyLines(bool i)
    { m_ignoreEmptyLines = i; return *this; }
    inline TextFormatter& DropRemain(bool d) { m_dropRemain = d; return *this; }

    void FitToLines(const std::wstring& text, std::vector<std::wstring>& lines) const;
    std::wstring FitToLine(const std::wstring& text) const;

  private:
    static const wchar_t LinesDelimiter = '\n';
    static const wchar_t* WordDelimiters;
    static const std::wstring dropMark;

    void SplitLines(const std::wstring& text, std::vector<std::wstring>& lines) const;
    void SplitWords(const std::wstring& text, std::vector<std::wstring>& words) const;

    int m_textWidth;
    bool m_wordWrap;
    bool m_ignoreEmptyLines;
    bool m_dropRemain;
};
