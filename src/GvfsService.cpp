#include <iostream>
#include <functional>
#include "GvfsService.h"

// "Объезд" ошибки в glibmm v2.50.0: проблемы с управлением памятью из-за
// использования Glib::StringArrayHandle в сигнале/слоте ask_question класса
// Gio::MountOperation. Приходится использовать оригинальный C-интерфейс GIO
// и глобальную переменную ask_question_callback. См. GvfsService::mount() и
// слот GvfsService::on_ask_question().
//
// TODO
// В старших версиях glibmm тип Glib::StringArrayHandle полностью заменен на
// std::vector<Glib::ustring>. После обновления версии в Debian можно будет
// проверить работоспособность Gio::MountOperation, а затем перейти к условной
// компиляции GvfsService в зависимости от версии glibmm
namespace {

std::function<void(GMountOperation*, char*, char**, gpointer)>
    ask_question_callback;

extern "C" void ask_question_wrapper(GMountOperation* op,
                                     char* message, char** choices,
                                     gpointer user_data)
{
    ask_question_callback(op, message, choices, user_data);
}

} // anonymous namespace

GvfsService::GvfsService() :
    m_mountCount(0)
{
}

bool GvfsService::mount(const std::string &resPath, const std::string &userName,
                        const std::string &password)
    throw(GvfsServiceException)
{
    using namespace std::placeholders;

std::cerr << "GvfsService::mount() " << resPath << std::endl;
    m_exception.reset();
    m_mountPath.clear();
    m_mountName.clear();

    Gio::init();

    m_mainLoop = Glib::MainLoop::create(false);

    m_file = Gio::File::create_for_parse_name(resPath);
    Glib::RefPtr<Gio::MountOperation> mount_operation = Gio::MountOperation::create();

    bool l_anonymous = userName.empty() && password.empty();

    if (!userName.empty()) mount_operation->set_username(userName);
    if (!password.empty()) mount_operation->set_password(password);

    // connect mount_operation slots
#if 0
    mount_operation->signal_ask_question().connect(
        std::bind(&GvfsService::on_ask_question, this, mount_operation, _1, _2)
    );
#endif
    ask_question_callback = std::bind(&GvfsService::on_ask_question, this,
                                      _1, _2, _3, _4);
    g_signal_connect(mount_operation->gobj(), "ask_question",
                     G_CALLBACK(ask_question_wrapper), nullptr);
    mount_operation->signal_ask_password().connect(
        std::bind(&GvfsService::on_ask_password, this, mount_operation, l_anonymous,
                  _1, _2, _3, _4)
    );
    mount_operation->signal_aborted().connect(
        std::bind(&GvfsService::on_aborted, this, mount_operation)
    );

    // do mount
    bool l_mounted = false;
    try
    {
        m_file->mount_enclosing_volume(mount_operation,
                                       [this] (Glib::RefPtr<Gio::AsyncResult>& result)
                                       {
                                          this->mount_cb(result);
                                       });
        m_mountCount++;
std::cerr << "GvfsService::mount() inc m_mountCount: " << m_mountCount << std::endl;
        if (m_mountCount > 0)
        {
            m_mainLoop->run();
        }
        m_mountName = m_file->find_enclosing_mount()->get_name();
        m_mountPath = m_file->find_enclosing_mount()->get_default_location()->get_path();
        std::cout << "GvfsService::mount() name: " << m_mountName << std::endl;
        std::cout << "GvfsService::mount() path: " << m_mountPath << std::endl;
        l_mounted = true;
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::mount() Glib::Error: " << ex.what().raw() << std::endl
                  << "m_mountCount: " << m_mountCount << std::endl;
        if (m_exception.get() == nullptr)
        {
          m_exception = std::make_shared<GvfsServiceException>(ex.domain(),
                                         ex.code(), ex.what());
        }
        throw *m_exception;
    }
    return l_mounted;
}

bool GvfsService::umount(const std::string &resPath)
    throw(GvfsServiceException)
{
std::cerr << "GvfsService::umount() " << resPath << std::endl;
    m_exception.reset();

    Gio::init();

    m_mainLoop = Glib::MainLoop::create(false);

    m_file = Gio::File::create_for_parse_name(resPath);
    Glib::RefPtr<Gio::MountOperation> mount_operation = Gio::MountOperation::create();

    bool l_unmounted = false;
    try
    {
        Glib::RefPtr<Gio::Mount> mount = m_file->find_enclosing_mount();
        mount->unmount(mount_operation,
                       [&l_unmounted, this] (Glib::RefPtr<Gio::AsyncResult>& result)
                       {
                           l_unmounted = this->unmount_cb(result);
                       });
        m_mountCount++;
std::cerr << "GvfsService::umount() inc m_mountCount: " << m_mountCount << std::endl;
        if (m_mountCount > 0)
        {
            m_mainLoop->run();
        }
        
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::umount() Glib::Error: "<< ex.what().raw() << std::endl
                  << "m_mountCount: " << m_mountCount << std::endl;
        if (m_exception.get() == nullptr)
        {
          m_exception = std::make_shared<GvfsServiceException>(ex.domain(),
                                         ex.code(), ex.what());
        }
        m_mountPath.clear();
        m_mountName.clear();
        throw *m_exception;
    }
    if (l_unmounted)
        {
            m_mountPath.clear();
            m_mountName.clear();
        }
        else
        {
            if (m_exception.get() != nullptr)
            {
                m_mountPath.clear();
                m_mountName.clear();
                throw *m_exception;
            }
        }
    return l_unmounted;
}

bool GvfsService::mounted(const std::string& resPath)
{
std::cerr << "GvfsService::mounted() " << resPath << std::endl;
    m_exception.reset();
    m_mountPath.clear();
    m_mountName.clear();

    Gio::init();

    m_mainLoop = Glib::MainLoop::create(false);

    m_file = Gio::File::create_for_parse_name(resPath);
    Glib::RefPtr<Gio::Mount> l_mount;
    try
    {
        m_file->find_enclosing_mount_async([&l_mount, this] (Glib::RefPtr<Gio::AsyncResult>& result)
                                           {
                                               l_mount = this->find_mount_cb(result);
                                           });
        m_mountCount++;
std::cerr << "GvfsService::mounted() inc m_mountCount: " << m_mountCount << std::endl;
        if (m_mountCount > 0)
        {
            m_mainLoop->run();
        }
        if (l_mount.operator->() != nullptr)
        {
            m_mountName = l_mount->get_name();
            m_mountPath = l_mount->get_default_location()->get_path();
        }
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::mounted() Glib::Error: "<< ex.what().raw()
                  << std::endl;
        l_mount.reset();
    }
    // don't escalate error here
    m_exception.reset();
    return (l_mount.operator->() != nullptr);
}

// TODO
// Реализовать выбор варианта ответа пользователем.
#if 0
void GvfsService::on_ask_question(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                                  const Glib::ustring& msg,
                                  const Glib::StringArrayHandle& choices)
{
std::cerr << "on signal_ask_question: " << msg.raw() << std::endl;
std::cerr << "choices:" << std::endl;
int i = 0;
for (const auto& choice : choices) std::cerr << i++ << " " << choice.raw() << std::endl;
    mount_operation->reply(Gio::MOUNT_OPERATION_HANDLED);
}
#endif
void GvfsService::on_ask_question(GMountOperation* op, char* message,
                                  char** choices, gpointer user_data)
{
    (void)user_data;
std::cerr << "on signal_ask_question: " << message << std::endl;
std::cerr << "choices:" << std::endl;
int i = 0;
char** choice = choices;
while (*choice)
{
std::cerr << i << " " << *choice << std::endl;
i++;
choice++;
}
    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

void GvfsService::on_ask_password(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                                  bool l_anonymous, const Glib::ustring& msg,
                                  const Glib::ustring& defaultUser,
                                  const Glib::ustring& defaultdomain,
                                  Gio::AskPasswordFlags flags)
{
std::cerr << "Gvfs on signal_ask_password ask password: " << msg.raw() << std::endl
          << "Gvfs on signal_ask_password default user: " << defaultUser.raw() << std::endl
          << "Gvfs on signal_ask_password default domain: " << defaultdomain.raw() << std::endl;

    if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) && l_anonymous)
    {
std::cerr << "Gvfs on signal_ask_password set anonymous" << std::endl;
        mount_operation->set_anonymous(true);
    }
    else
    {
        // trigger functor for entering user credentials
        if (flags & G_ASK_PASSWORD_NEED_USERNAME)
        {
std::cerr << "Gvfs on signal_ask_password NEED USERNAME" << std::endl;
            // trigger user name enter callback, call passwd functor
        }
        if (flags & G_ASK_PASSWORD_NEED_DOMAIN)
        {
std::cerr << "Gvfs on signal_ask_password NEED DOMAIN" << std::endl;
            // trigger domain name enter callback, call passwd functor
        }
        if (flags & G_ASK_PASSWORD_NEED_PASSWORD)
        {
std::cerr << "Gvfs on signal_ask_password NEED PASSWORD" << std::endl;
            // trigger password name enter callback, call passwd functor
        }
    }
    mount_operation->reply(Gio::MOUNT_OPERATION_HANDLED);
}

void GvfsService::on_aborted(Glib::RefPtr<Gio::MountOperation>& mount_operation)
{
std::cerr << "on signal_aborted" << std::endl;
}

void GvfsService::mount_cb(Glib::RefPtr<Gio::AsyncResult>& result)
{
std::cerr << "GvfsService::mount_cb()" << std::endl;
    try
    {
        m_file->mount_enclosing_volume_finish(result);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::mount_cb() Glib::Error: "<< ex.what().raw() << std::endl;
        // fill exception
        m_exception = std::make_shared<GvfsServiceException>(ex.domain(), ex.code(), ex.what());
    }

    m_mountCount--;
std::cerr << "GvfsService::mount_cb() dec m_mountCount: " << m_mountCount << std::endl;

    if (m_mountCount == 0)
    {
        m_mainLoop->quit();
    }
}

bool GvfsService::unmount_cb(Glib::RefPtr<Gio::AsyncResult> &result)
{
    Glib::RefPtr<Gio::Mount> mount = Glib::RefPtr<Gio::Mount>::cast_dynamic(result->get_source_object_base());
    bool success = false;
    try
    {
        success = mount->unmount_finish(result);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::unmount_cb() Glib::Error: "<< ex.what().raw() << std::endl;
        // fill exception
        m_exception = std::make_shared<GvfsServiceException>(ex.domain(), ex.code(), ex.what());
    }

    m_mountCount--;
std::cerr << "GvfsService::unmount_cb() dec m_mountCount: " << m_mountCount << std::endl;

    if (m_mountCount == 0)
    {
        m_mainLoop->quit();
    }
    return success;
}

Glib::RefPtr<Gio::Mount> GvfsService::find_mount_cb(Glib::RefPtr<Gio::AsyncResult>& result)
{
std::cerr << "GvfsService::find_mount_cb()" << std::endl;
    Glib::RefPtr<Gio::Mount> l_mount;
    try
    {
        l_mount = m_file->find_enclosing_mount_finish(result);
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::find_mount_cb() Glib::Error: "<< ex.what().raw() << std::endl;
        // fill exception
        m_exception = std::make_shared<GvfsServiceException>(ex.domain(), ex.code(), ex.what());
    }

    m_mountCount--;
std::cerr << "GvfsService::find_mount_cb() dec m_mountCount: " << m_mountCount << std::endl;

    if (m_mountCount == 0)
    {
        m_mainLoop->quit();
    }
    return l_mount;
}
