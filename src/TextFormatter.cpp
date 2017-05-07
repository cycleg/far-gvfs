#include "TextFormatter.h"

const std::wstring TextFormatter::dropMark = L"...";
const wchar_t* TextFormatter::WordDelimiters = L" \t\n\r";

TextFormatter::TextFormatter():
  m_textWidth(0),
  m_wordWrap(true),
  m_ignoreEmptyLines(true),
  m_dropRemain(false)
{
}

void TextFormatter::FitToLines(const std::wstring& text,
                            std::vector<std::wstring>& lines) const
{
  lines.clear();
  if (!m_textWidth) return;
  std::wstring buffer;
  std::vector<std::wstring> linesBuffer;
  SplitLines(text, linesBuffer);
  for (std::wstring line : linesBuffer)
  {
    if (line.empty())
    {
      // only if not ignore empty lines
      lines.push_back(line);
      continue;
    }
    if (m_wordWrap)
      {
        std::vector<std::wstring> wordsBuffer;
        SplitWords(line, wordsBuffer);
        for (std::wstring word : wordsBuffer)
        {
          if (buffer.size() + word.size() + 1 > m_textWidth)
            {
              if (buffer.empty())
              {
                // безвыходная ситуация: слово длиннее ширины строки
                if (m_dropRemain)
                  {
                    buffer = word.substr(0, m_textWidth - dropMark.size());
                    buffer.append(dropMark);
                  }
                  else buffer = word;
                lines.push_back(buffer);
                buffer.clear();
              }
              else
              {
                lines.push_back(buffer);
                buffer.clear();
                if (word.size() > m_textWidth)
                  {
                    // безвыходная ситуация: слово длиннее ширины строки
                    if (m_dropRemain)
                      {
                        buffer += word.substr(0, m_textWidth - dropMark.size());
                        buffer.append(dropMark);
                      }
                      else buffer = word;
                  }
                  else buffer.append(word);
              }
            }
            else
            {
              if (!buffer.empty()) buffer.push_back(' ');
              buffer.append(word);
            }
        }
        if (!buffer.empty())
        {
          lines.push_back(buffer);
          buffer.clear();
        }
      }
      else
      {
        // lines fit to width "as is"
        while (!line.empty())
        {
          if (line.size() > m_textWidth)
            {
              lines.push_back(line.substr(0, m_textWidth));
              line.erase(0, m_textWidth);
            }
            else 
            {
              lines.push_back(line);
              line.clear();
            }
        }
      }
  }
}

std::wstring TextFormatter::FitToLine(const std::wstring& text) const
{
  std::wstring buffer;
  if (!m_textWidth) return buffer;
  std::vector<std::wstring> linesBuffer;
  SplitLines(text, linesBuffer);
  if (linesBuffer.empty()) return buffer;
  std::wstring line = linesBuffer[0];
  if (!line.empty())
  {
    if (m_wordWrap)
      {
        std::vector<std::wstring> wordsBuffer;
        SplitWords(line, wordsBuffer);
        for (std::wstring word : wordsBuffer)
        {
          if (buffer.size() + word.size() + 1 > m_textWidth)
            {
              if (buffer.empty())
                {
                  // безвыходная ситуация: слово длиннее ширины строки
                  if (m_dropRemain)
                    {
                      buffer += word.substr(0, m_textWidth - dropMark.size());
                      buffer.append(dropMark);
                    }
                    else buffer = word;
                }
                else
                {
                  if (m_dropRemain)
                    {
                      buffer.push_back(' ');
                      buffer.append(word);
                      buffer.erase(m_textWidth - dropMark.size());
                      buffer.append(dropMark);
                    }
                    else if (m_textWidth - buffer.size() > 1)
                    {
                      buffer.push_back(' ');
                      buffer.append(word.substr(0, m_textWidth - buffer.size()));
                    }
                }
              break;
            }
            else
            {
              if (!buffer.empty()) buffer.push_back(' ');
              buffer.append(word);
            }
        }
      }
      else
      {
        if (line.size() > m_textWidth)
          {
            if (m_dropRemain)
              {
                buffer = line.substr(0, m_textWidth - dropMark.size());
                buffer.append(dropMark);
              }
              else buffer = line.substr(0, m_textWidth);
          }
          else buffer = line;
      }
  }
  if ((linesBuffer.size() > 1) && m_dropRemain)
  {
    if (m_textWidth - buffer.size() < dropMark.size())
      buffer.erase(m_textWidth - dropMark.size());
    buffer.append(dropMark);
  }
  return buffer;
}

void TextFormatter::SplitLines(const std::wstring& text,
                            std::vector<std::wstring>& lines) const
{
  lines.clear();
  std::wstring::size_type pos1 = text.find_first_not_of(LinesDelimiter),
                          pos2 = (pos1 != std::wstring::npos) ?
                                 text.find_first_of(LinesDelimiter, pos1) :
                                 std::wstring::npos;
  if ((pos1 == std::wstring::npos) && (text.size() > 0))
  {
    // only empty line(s)
    if (m_ignoreEmptyLines) return;
    for (int i = 0; i < text.size(); i++) lines.push_back(std::wstring());
  }
  if ((pos1 > 0) && !m_ignoreEmptyLines)
  {
    // leading empty lines
    for (int i = 0; i < pos1; i++) lines.push_back(std::wstring());
  }
  while (pos2 != std::wstring::npos)
  {
    std::wstring::size_type pos3 = pos2;
    if (text[pos3 - 1] == '\r') pos3--;
    if ((pos3 != pos1) || !m_ignoreEmptyLines)
      lines.push_back(text.substr(pos1, pos3 - pos1));
    if (m_ignoreEmptyLines)
      {
        // skip repeated delimiters
        pos1 = text.find_first_not_of(LinesDelimiter, pos2);
        if (pos1 == std::wstring::npos) break;
        pos2 = text.find_first_of(LinesDelimiter, pos1);
      }
      else
      {
        pos1 = pos2 + 1;
        if (pos1 >= text.size())
          pos1 = pos2 = std::wstring::npos;
          else pos2 = text.find_first_of(LinesDelimiter, pos1);
      }
  }
  if (pos1 != std::wstring::npos)
  {
    // last (single) line
    std::wstring::size_type pos3 = text.size();
    if (text[pos3 - 1] == '\r') pos3--;
    if ((pos3 != pos1) || !m_ignoreEmptyLines)
      lines.push_back(text.substr(pos1, pos3 - pos1));
  }
}

void TextFormatter::SplitWords(const std::wstring& text,
                            std::vector<std::wstring>& words) const
{
  words.clear();
  std::wstring::size_type pos1 = text.find_first_not_of(WordDelimiters),
                          pos2 = (pos1 != std::wstring::npos) ?
                                 text.find_first_of(WordDelimiters, pos1) :
                                 std::wstring::npos;
  while (pos2 != std::wstring::npos)
  {
    words.push_back(text.substr(pos1, pos2 - pos1));
    // skip repeated delimiters
    pos1 = text.find_first_not_of(WordDelimiters, pos2);
    if (pos1 == std::wstring::npos) break;
    pos2 = text.find_first_of(WordDelimiters, pos1);
  }
  if (pos1 != std::wstring::npos)
  {
    // last (single) word
    words.push_back(text.substr(pos1));
  }
}
