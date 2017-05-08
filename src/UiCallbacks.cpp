#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include "gvfsdlg.h"
#include "TextFormatter.h"
#include "UiCallbacks.h"

UiCallbacks::UiCallbacks(PluginStartupInfo& info):
  m_pStartupInfo(info)
{
}

void UiCallbacks::onAskQuestion(char* message, char** choices, int& choice) const
{
  int answer = choice;
  char** l_choice = choices;
  std::vector<std::wstring> messageLines, answers;
  TextFormatter formatter;
  formatter.TextWidth(70) // look at AskQuestionDlg() in gvfsdlg.cpp
           .WordWrap(true)
           .IgnoreEmptyLines(false)
           .DropRemain(true);
  formatter.FitToLines(
    std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(message),
    messageLines
  );
  while (*l_choice)
  {
    answers.push_back(
      std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(*l_choice)
    );
    l_choice++;
  }
  for (auto& buf : answers) buf = formatter.FitToLine(buf);
  if (!AskQuestionDlg(m_pStartupInfo, messageLines, answers, (unsigned int&)answer))
    answer = -1;
  choice = answer;
}
