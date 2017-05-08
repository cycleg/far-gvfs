#pragma once

#include <string>
#include "GvfsServiceException.h"

class GvfsService;

class MountPointStorage;

class MountPoint
{
  friend class MountPointStorage;

  public:
    enum class FileSystem {
        NoFs,
        DiskFs,
        Scp,
        Nfs,
        Samba,
        WebDav
    };

    MountPoint();
    MountPoint(const std::wstring& resPath, const std::wstring& user, const std::wstring& password);
    MountPoint(const MountPoint& other);

    MountPoint& operator=(const MountPoint& other);

    inline bool isMounted() const { return m_bMounted; }
    inline FileSystem getFsType() const { return m_type; }
    inline const std::wstring& getFsPath() const { return m_mountPointPath; }
    inline const std::wstring& getMountName() const { return m_shareName; }
    inline const std::wstring& getResPath() const { return m_resPath; }
    inline const std::wstring& getUser() const { return m_user; }
    inline const std::wstring& getPassword() const { return m_password; }
    inline bool getAskPassword() const { return m_askPassword; }

    inline MountPoint& setResPath(const std::wstring& s)
    { m_resPath = s; return *this; }
    inline MountPoint& setUser(const std::wstring& s) { m_user = s; return *this; }
    inline MountPoint& setPassword(const std::wstring& s)
    { m_password = s; return *this; }
    inline MountPoint& setAskPassword(bool ask)
    { m_askPassword = ask; return *this; }

    bool mount(GvfsService* service) throw(GvfsServiceException);
    bool unmount(GvfsService* service) throw(GvfsServiceException);
    void mountCheck(GvfsService* service);

  private:
    bool m_bMounted;
    FileSystem m_type;
    std::wstring m_resPath;
    std::wstring m_user;
    std::wstring m_password;
    std::wstring m_mountPointPath;
    std::wstring m_shareName;
    std::wstring m_storageId;
    bool m_askPassword;
};
