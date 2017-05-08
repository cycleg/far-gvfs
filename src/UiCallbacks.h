#pragma once

#include <plugin.hpp>

class UiCallbacks
{
  public:
    UiCallbacks(PluginStartupInfo& info);

    void onAskQuestion(char* message, char** choices, int& choice) const;

  private:
    PluginStartupInfo& m_pStartupInfo;
};
