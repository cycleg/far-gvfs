#include <string>
#include <WideMB.h> // far2l/utils
#include "dialogs.h"
#include "TextFormatter.h"
#include "UiCallbacks.h"

UiCallbacks::UiCallbacks(PluginStartupInfo& info):
  m_pStartupInfo(info)
{
}

void UiCallbacks::onAskQuestion(const Glib::ustring& message,
                                const std::vector<Glib::ustring>& choices,
                                int& choice) const
{
  int answer = choice;
  std::vector<std::wstring> messageLines, answers;
  TextFormatter formatter;
  // 3 columns - frame, 1 column - pad
  formatter.TextWidth(AskQuestionDlgWidth - (3 + 1) * 2)
           .WordWrap(true)
           .IgnoreEmptyLines(false)
           .DropRemain(true);
  formatter.FitToLines(MB2Wide(message.raw().c_str()), messageLines);
  for (auto l_choice: choices)
    answers.push_back(MB2Wide(l_choice.raw().c_str()));
  formatter.TextWidth(AskQuestionDlgWidth - 4 * 2 - 2); // another 2 columns in combobox
  for (auto& buf : answers) buf = formatter.FitToLine(buf);
  if (!AskQuestionDlg(m_pStartupInfo, AskQuestionDlgWidth, messageLines, answers,
                      (unsigned int&)answer))
    answer = -1;
  choice = answer;
}

void UiCallbacks::onAskQuestion(char* message, char** choices, int& choice) const
{
  int answer = choice;
  char** l_choice = choices;
  std::vector<std::wstring> messageLines, answers;
  TextFormatter formatter;
  // 3 columns - frame, 1 column - pad
  formatter.TextWidth(AskQuestionDlgWidth - (3 + 1) * 2)
           .WordWrap(true)
           .IgnoreEmptyLines(false)
           .DropRemain(true);
  formatter.FitToLines(MB2Wide(message), messageLines);
  while (*l_choice)
  {
    answers.push_back(MB2Wide(*l_choice));
    l_choice++;
  }
  formatter.TextWidth(AskQuestionDlgWidth - 4 * 2 - 2); // another 2 columns in combobox
  for (auto& buf : answers) buf = formatter.FitToLine(buf);
  if (!AskQuestionDlg(m_pStartupInfo, AskQuestionDlgWidth, messageLines, answers,
                      (unsigned int&)answer))
    answer = -1;
  choice = answer;
}
