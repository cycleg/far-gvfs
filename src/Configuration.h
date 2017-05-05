#pragma once

#include <string>
#include <windows.h>

class Configuration
{
  public:
    static Configuration* Instance(const std::wstring& registryFolder);

    inline static Configuration* Instance() { return Configuration::instance; }
    inline bool unmountAtExit() const { return m_unmountAtExit; }
    inline void setUnmountAtExit(bool u) { m_unmountAtExit = u; }

    void save() const;

  private:
    static Configuration* instance;

    Configuration(const std::wstring& registryFolder);

    void load();

    bool SetValue(HKEY folder, const std::wstring& field,
                   const DWORD value) const;
    bool GetValue(HKEY folder, const std::wstring& field,
                   DWORD& value) const;

    std::wstring m_registryFolder;
    bool m_unmountAtExit;
};
