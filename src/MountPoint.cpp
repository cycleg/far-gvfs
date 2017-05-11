#include <string>
#include <utils.h>
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

bool MountPoint::mount(GvfsService* service) throw(GvfsServiceException)
{
    std::string resPath(StrWide2MB(m_resPath));
    std::string userName(StrWide2MB(m_user));
    std::string password(StrWide2MB(m_password));

    // пароль спрашивают перед монтированием, зачищаем его
    if (m_askPassword) m_password.clear();
    if (m_bMounted) return true;
    if (resPath.empty()) return false;

    bool success = service->mount(resPath, userName, password);
    if (success)
    {
        m_bMounted = true;
        StrMB2Wide(service->getMountName(), m_shareName);
        StrMB2Wide(service->getMountPath(), m_mountPointPath);
    }
    return success;
}

bool MountPoint::unmount(GvfsService* service) throw(GvfsServiceException)
{
    if (!m_bMounted) return true;

    std::string resPath(StrWide2MB(this->m_resPath));
    if (resPath.empty()) return false;

    bool success = false;
    try
    {
        success = service->umount(resPath);
    }
    catch (const GvfsServiceException& e)
    {
        // exception equal to unmount
        m_mountPointPath.clear();
        m_shareName.clear();
        m_bMounted = false;
        throw; // escalate error
    }
    if (success)
    {
        m_mountPointPath.clear();
        m_shareName.clear();
        m_bMounted = false;
    }
    return success;
}

void MountPoint::mountCheck(GvfsService* service)
{
    std::string resPath(StrWide2MB(m_resPath));
    if (resPath.empty())
    {
        m_mountPointPath.clear();
        m_shareName.clear();
        m_bMounted = false;
        return;
    }
    if (service->mounted(resPath))
        {
            m_bMounted = true;
            StrMB2Wide(service->getMountPath(), m_mountPointPath);
            StrMB2Wide(service->getMountName(), m_shareName);
        }
        else
        {
            m_shareName.clear();
            m_mountPointPath.clear();
            m_bMounted = false;
        }
}
