#pragma once

#include <string>
#include <map>
#include <vector>
#include <WinCompat.h>
#include "MountPoint.h"

class MountPointStorage
{
  public:
    MountPointStorage(const std::wstring& registryFolder);

    static MountPoint PointFactory();

    void LoadAll(std::map<std::wstring, MountPoint>& storage) const;
    bool Save(const MountPoint& point) const;
    void Delete(const MountPoint& point) const;

  private:
    static const wchar_t* StoragePath;

    static void GenerateId(std::wstring& id);
    static void Encrypt(const std::wstring& in, std::vector<BYTE>& out);
    static void Decrypt(const std::vector<BYTE>& in, std::wstring& out);

    bool Load(MountPoint& point) const;

    bool SetRegKey(HKEY folder, const std::wstring& field,
                   const std::vector<BYTE>& value) const;
    bool SetRegKey(HKEY folder, const std::wstring& field,
                   const std::wstring& value) const;
    bool SetRegKey(HKEY folder, const std::wstring& field,
                   const DWORD value) const;
    bool GetRegKey(HKEY folder, const std::wstring& field,
                   std::vector<BYTE>& value) const;
    bool GetRegKey(HKEY folder, const std::wstring& field,
                   std::wstring& value) const;
    bool GetRegKey(HKEY folder, const std::wstring& field,
                   DWORD& value) const;

    std::wstring m_registryFolder;
};
