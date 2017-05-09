#include <string>
#include <vector>
#include <utils.h>
#include "gvfsdlg.h"
#include "TextFormatter.h"
#include "UiCallbacks.h"

UiCallbacks::UiCallbacks(PluginStartupInfo& info):
  m_pStartupInfo(info)
{
}

void UiCallbacks::onAskQuestion(char* message, char** choices, int& choice) const
{
  const int DIALOG_WIDTH = 78;
  int answer = choice;
  char** l_choice = choices;
  std::vector<std::wstring> messageLines, answers;
  TextFormatter formatter;
  // 3 columns - frame, 1 column - pad
  formatter.TextWidth(DIALOG_WIDTH - (3 + 1) * 2)
           .WordWrap(true)
           .IgnoreEmptyLines(false)
           .DropRemain(true);
  formatter.FitToLines(MB2Wide(message), messageLines);
  while (*l_choice)
  {
    answers.push_back(MB2Wide(*l_choice));
    l_choice++;
  }
  formatter.TextWidth(DIALOG_WIDTH - 4 * 2 - 2); // another 2 columns in combobox
  for (auto& buf : answers) buf = formatter.FitToLine(buf);
  if (!AskQuestionDlg(m_pStartupInfo, DIALOG_WIDTH, messageLines, answers,
                      (unsigned int&)answer))
    answer = -1;
  choice = answer;
}
