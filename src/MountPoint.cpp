#include <string>
#include <locale>
#include <codecvt>
#include "GvfsService.h"
#include "MountPoint.h"

MountPoint::MountPoint():
    m_bMounted(false),
    m_type(FileSystem::NoFs),
    m_askPassword(false)
{
}

MountPoint::MountPoint(const std::wstring &resPath, const std::wstring &user, const std::wstring &password) :
    m_bMounted(false),
    m_type(FileSystem::NoFs),
    m_resPath(resPath),
    m_user(user),
    m_password(password),
    m_askPassword(false)
{
}

MountPoint::MountPoint(const MountPoint& other)
{
    m_bMounted = other.m_bMounted;
    m_type = other.m_type;
    m_resPath = other.m_resPath;
    m_user = other.m_user;
    m_password = other.m_password;
    m_mountPointPath = other.m_mountPointPath;
    m_shareName = other.m_shareName;
    m_storageId = other.m_storageId;
    m_askPassword = other.m_askPassword;
}

MountPoint& MountPoint::operator=(const MountPoint& other)
{
    m_bMounted = other.m_bMounted;
    m_type = other.m_type;
    m_resPath = other.m_resPath;
    m_user = other.m_user;
    m_password = other.m_password;
    m_mountPointPath = other.m_mountPointPath;
    m_shareName = other.m_shareName;
    m_storageId = other.m_storageId;
    m_askPassword = other.m_askPassword;
    return *this;
}

bool MountPoint::mount() throw(GvfsServiceException)
{
    std::string resPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(m_resPath);
    std::string userName = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(m_user);
    std::string password = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(m_password);

    // пароль спрашивают перед монтированием, зачищаем его
    if (m_askPassword) m_password.clear();
    if (m_bMounted) return true;
    if (resPath.empty()) return false;

    GvfsService service;
    bool success = service.mount(resPath, userName, password);
    if (success)
    {
        m_bMounted = true;
        m_shareName = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                      .from_bytes(service.getMountName());
        m_mountPointPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                           .from_bytes(service.getMountPath());
    }
    return success;
}

bool MountPoint::unmount() throw(GvfsServiceException)
{
    if (!m_bMounted) return true;

    std::string resPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(this->m_resPath);
    if (resPath.empty()) return false;

    GvfsService service;
    bool success = service.umount(resPath);
    if (success)
    {
        m_mountPointPath.clear();
        m_shareName.clear();
        m_bMounted = false;
    }
    return success;
}

void MountPoint::mountCheck()
{
    if (m_bMounted) return;

    std::string resPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(m_resPath);
    if (resPath.empty()) return;

    GvfsService service;
    if (service.mounted(resPath))
    {
        m_bMounted = true;
        m_shareName = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                      .from_bytes(service.getMountName());
        m_mountPointPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                           .from_bytes(service.getMountPath());
    }
}
