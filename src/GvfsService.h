#pragma once

#include <memory>
#include <string>
#include <gtkmm.h>
#include "GvfsServiceException.h"

// Класс не потокобезопасный из-за уродливого "объезда" ошибки в glibmm
// v2.50.0. Подробнее см. в GvfsService.cpp.
class GvfsService
{
public:
    GvfsService();

    inline const std::string& getMountName() const { return m_mountName; }
    inline const std::string& getMountPath() const { return m_mountPath; }

    bool mount(const std::string& resPath, const std::string &userName,
               const std::string &password) throw(GvfsServiceException);
    bool umount(const std::string& resPath) throw(GvfsServiceException);
    bool mounted(const std::string& resPath);

private:
#if 0
    void on_ask_question(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                         const Glib::ustring& msg,
                         const Glib::StringArrayHandle& choices);
#endif
    void on_ask_question(GMountOperation* op, char* message, char** choices,
                         gpointer user_data);
    void on_ask_password(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                         bool l_anonymous, const Glib::ustring& msg,
                         const Glib::ustring& defaultUser,
                         const Glib::ustring& defaultdomain,
                         Gio::AskPasswordFlags flags);
    void on_aborted(Glib::RefPtr<Gio::MountOperation>& mount_operation);

    void mount_cb(Glib::RefPtr<Gio::AsyncResult>& result);
    bool unmount_cb(Glib::RefPtr<Gio::AsyncResult>& result);
    Glib::RefPtr<Gio::Mount> find_mount_cb(Glib::RefPtr<Gio::AsyncResult>& result);

    int m_mountCount;
    std::string m_mountName;
    std::string m_mountPath;
    Glib::RefPtr<Gio::File> m_file;
    Glib::RefPtr<Glib::MainLoop> m_mainLoop;
    std::shared_ptr<GvfsServiceException> m_exception;
};
