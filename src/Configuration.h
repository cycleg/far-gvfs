#pragma once

#include "RegistryStorage.h"

class Configuration: public RegistryStorage
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

    bool m_unmountAtExit;
};
