#include <atomic>
#include <iostream>
#include <functional>
#include <mutex>
#include <unordered_map>
#include "UiCallbacks.h"
#include "GvfsService.h"

// "Объезд" ошибки в glibmm v2.50.0: проблемы с управлением памятью из-за
// использования Glib::StringArrayHandle в сигнале/слоте ask_question класса
// Gio::MountOperation. Приходится использовать оригинальный C-интерфейс GIO
// и глобальную переменную ask_question_callback. См. GvfsService::mount() и
// слот GvfsService::on_ask_question().
//
// Объезд с ask_password пришлось сделать после введения
// g_main_context_push_thread_default(). Без него приложение аварийно
// завершалось.
//
// TODO
// В старших версиях glibmm тип Glib::StringArrayHandle полностью заменен на
// std::vector<Glib::ustring>. После обновления версии в Debian можно будет
// проверить работоспособность Gio::MountOperation, а затем перейти к условной
// компиляции GvfsService в зависимости от версии glibmm.

extern "C" void ask_question_wrapper(GMountOperation* op, char* message,
                                     char** choices, gpointer user_data);
extern "C" void ask_password_wrapper(GMountOperation* op, const char* message,
                                     const char* default_user,
                                     const char* default_domain,
                                     GAskPasswordFlags flags);

namespace {

///
/// @brief Таблица слотов для обработки сигналов процедуры монтирования ресурса.

/// Здесь слоты -- обращения к методам класса GvfsService при обработке
/// сигналов от GMountOperation через C-интерфейс (не C++!). Перед запуском
/// монтирования экземпляр GMountOperation регистрируется в этой таблице, по
/// завершению -- удаляется из нее.
///
/// При манипуляциях с таблицей потокобезопасность обеспечивается стратегией
/// "один писатель, много читателей". Попытки зарегистрировать объект в таблице
/// или удалить из нее задерживаются до тех пор, пока не завершатся все
/// запущенные слоты. Эти операции выполняются за фиксированное время, в
/// отличие от слотов. Слоты могут запускаться параллельно.
///
/// @author cycleg
///
class MountCallbacks
{
    friend class ::GvfsService; ///< Только экземпляры GvfsService могут
                                ///< манипулировать таблицей.

  public:
    MountCallbacks(): m_readers(0), m_lock(m_mapMutex, std::defer_lock)
    {
    }

    void onAskQuestion(GMountOperation* op, char* message, char** choices,
                       gpointer user_data)
    {
        if (m_readers++ == 0) m_lock.lock();
        auto i = m_slots.find(op);
        if (i != m_slots.end())
            i->second.onAskQuestion(op, message, choices, user_data);
        if (--m_readers == 0) m_lock.unlock();
    }

    void onAskPassword(GMountOperation* op, const char* message,
                       const char* default_user, const char* default_domain,
                       GAskPasswordFlags flags)
    {
        if (m_readers++ == 0) m_lock.lock();
        auto i = m_slots.find(op);
        if (i != m_slots.end())
            i->second.onAskPassword(op, message, default_user, default_domain,
                                    flags);
        if (--m_readers == 0) m_lock.unlock();
    }

  private:
    struct slots
    {
        std::function<void(GMountOperation*, char*, char**, gpointer)>
            onAskQuestion; ///< слот для сигнала "ask_question"
        std::function<void(GMountOperation* op, const char* message,
                           const char* default_user,
                           const char* default_domain,
                           GAskPasswordFlags flags)>
            onAskPassword; ///< слот для сигнала "ask_password"
    };

    void connect(GMountOperation* op, slots& cb)
    {
        // ожидаем, когда отработают все вызванные слоты
        std::lock_guard<std::mutex> lck(m_mapMutex);
        m_slots.emplace(op, cb);
        g_signal_connect(op, "ask_question", G_CALLBACK(ask_question_wrapper),
                         nullptr);
        g_signal_connect(op, "ask_password", G_CALLBACK(ask_password_wrapper),
                         nullptr);
    }

    void disconnect(GMountOperation* op)
    {
        std::lock_guard<std::mutex> lck(m_mapMutex);
        auto i = m_slots.find(op);
        if (i != m_slots.end()) m_slots.erase(i);
    }

    std::atomic_int m_readers; ///< Число потоков-читателей таблицы (извлекших
                               ///< указатели на слоты).
    std::mutex m_mapMutex; ///< Мутекс на запись таблицы слотов.
                           ///< Заперт, если есть хотя бы один читатель таблицы
                           ///< или имеется писатель в таблицу.
    std::unique_lock<std::mutex> m_lock; ///< Закрыт, если есть хотя бы один
                                         ///< читатель таблицы.
    std::unordered_map<GMountOperation*, slots>
        m_slots; ///< Таблица слотов: ключ -- указатель на целевой объект,
                 ///< который будет обрабатываться.
                                                                  
};

MountCallbacks mountCallbacksRegistry;

extern "C" void ask_question_wrapper(GMountOperation* op,
                                     char* message, char** choices,
                                     gpointer user_data)
{
    mountCallbacksRegistry.onAskQuestion(op, message, choices, user_data);
}


extern "C" void ask_password_wrapper(GMountOperation* op,
                                     const char* message,
                                     const char* default_user,
                                     const char* default_domain,
                                     GAskPasswordFlags flags)
{
    mountCallbacksRegistry.onAskPassword(op, message, default_user,
                                         default_domain, flags);
}

} // anonymous namespace

GvfsService::GvfsService(UiCallbacks* uic) :
    m_uiCallbacks(uic)
{
}

bool GvfsService::mount(const std::string &resPath, const std::string &userName,
                        const std::string &password)
    throw(GvfsServiceException)
{
    using namespace std::placeholders;

    // какая-то операция в данном экземпляре уже запущена
    if (m_mainLoop && m_mainLoop->is_running()) return false;
std::cerr << "GvfsService::mount() " << resPath << std::endl;
    m_exception.reset();
    m_mountScheme.clear();
    m_mountPath.clear();
    m_mountName.clear();

    Gio::init();

    Glib::RefPtr< Glib::MainContext > main_context = Glib::MainContext::create();
    // Чтобы контекст главного цикла Glib::MainLoop не отслеживал ничего,
    // кроме операций с ресурсом! Иначе блокируется пользовательский ввод Far.
    // Из руководства:
    // This will cause certain asynchronous operations (such as most GIO-based
    // I/O) which are started in this thread to run under context and deliver
    // their results to its main loop, rather than running under the global
    // default context in the main thread.
    g_main_context_push_thread_default(main_context->gobj());
    m_mainLoop = Glib::MainLoop::create(main_context, false);

    m_file = Gio::File::create_for_parse_name(resPath);
    Glib::RefPtr<Gio::MountOperation> mount_operation = Gio::MountOperation::create();

    if (!userName.empty()) mount_operation->set_username(userName);
    if (!password.empty()) mount_operation->set_password(password);

    // connect mount_operation slots
#if 0
    mount_operation->signal_ask_question().connect(
        std::bind(&GvfsService::on_ask_question, this, mount_operation, _1, _2)
    );
    mount_operation->signal_ask_password().connect(
        std::bind(&GvfsService::on_ask_password, this, mount_operation, _1, _2,
                  _3, _4)
    );
#endif
    MountCallbacks::slots sl;
    sl.onAskQuestion = std::bind(&GvfsService::on_ask_question, this, _1, _2,
                                 _3, _4);
    sl.onAskPassword = std::bind(&GvfsService::on_ask_password, this, _1, _2,
                                 _3, _4, _5);
    mountCallbacksRegistry.connect(mount_operation->gobj(), sl);
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
        m_mainLoop->run();
        mountCallbacksRegistry.disconnect(mount_operation->gobj());
        // Из руководства:
        // In some cases however, you may want to schedule a single operation
        // in a non-default context, or temporarily use a non-default context
        // in the main thread. In that case, you can wrap the call to the
        // asynchronous operation inside a
        // g_main_context_push_thread_default() / g_main_context_pop_thread_default()
        // pair...
        // Второй вариант, видимо, наш случай. Без этого вызова Glib выдает
        // assert.
        g_main_context_pop_thread_default(main_context->gobj());
        m_mountName = m_file->find_enclosing_mount()->get_name();
        m_mountPath = m_file->get_path();
        m_mountScheme = m_file->get_uri_scheme();
        std::cout << "GvfsService::mount() name: " << m_mountName << std::endl;
        std::cout << "GvfsService::mount() path: " << m_mountPath << std::endl;
        std::cout << "GvfsService::mount() scheme: " << m_mountScheme << std::endl;
        l_mounted = true;
    }
    catch (const Glib::Error& ex)
    {
        // А здесь контекст потока восстанавливать не получается, и снова
        // из-за assert в Glib. Вероятно, это связано с "разматыванием" стека.
        // Сплошные загадки...
        std::cerr << "GvfsService::mount() Glib::Error: " << ex.what().raw()
                  << std::endl;
        mountCallbacksRegistry.disconnect(mount_operation->gobj());
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
    // какая-то операция в данном экземпляре уже запущена
    if (m_mainLoop && m_mainLoop->is_running()) return false;
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
        m_mainLoop->run();
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "GvfsService::umount() Glib::Error: "<< ex.what().raw()
                  << std::endl;
        if (m_exception.get() == nullptr)
        {
          m_exception = std::make_shared<GvfsServiceException>(ex.domain(),
                                         ex.code(), ex.what());
        }
        m_mountScheme.clear();
        m_mountPath.clear();
        m_mountName.clear();
        throw *m_exception;
    }
    if (l_unmounted)
        {
            m_mountScheme.clear();
            m_mountPath.clear();
            m_mountName.clear();
        }
        else
        {
            if (m_exception.get() != nullptr)
            {
                m_mountScheme.clear();
                m_mountPath.clear();
                m_mountName.clear();
                throw *m_exception;
            }
        }
    return l_unmounted;
}

bool GvfsService::mounted(const std::string& resPath)
{
    // какая-то операция в данном экземпляре уже запущена
    if (m_mainLoop && m_mainLoop->is_running()) return false;
std::cerr << "GvfsService::mounted() " << resPath << std::endl;
    m_exception.reset();
    m_mountScheme.clear();
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
        m_mainLoop->run();
        if (l_mount.operator->() != nullptr)
        {
            m_mountName = l_mount->get_name();
            m_mountPath = m_file->get_path();
            m_mountScheme = m_file->get_uri_scheme();
            std::cout << "GvfsService::mounted() name: " << m_mountName
                      << std::endl;
            std::cout << "GvfsService::mounted() path: " << m_mountPath
                      << std::endl;
            std::cout << "GvfsService::mounted() scheme: " << m_mountScheme
                      << std::endl;
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

#if 0
void GvfsService::on_ask_question(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                                  const Glib::ustring& msg,
                                  const Glib::StringArrayHandle& choices)
{
std::cerr << "on signal_ask_question: " << msg.raw() << std::endl;
std::cerr << "choices:" << std::endl;
int i = 0;
for (const auto& choice : choices) std::cerr << i++ << " " << choice.raw() << std::endl;
    if (m_uiCallbacks)
        {
            int answer = mount_operation->get_choice();
            m_uiCallbacks->onAskQuestion(message, choices, answer);
            if (answer != -1)
                {
                    mount_operation->set_choice(answer);
                    mount_operation->reply(Gio::MOUNT_OPERATION_HANDLED);
                }
                else mount_operation->reply(Gio::MOUNT_OPERATION_ABORTED);
        }
        else mount_operation->reply(Gio::MOUNT_OPERATION_HANDLED);
}

void GvfsService::on_ask_password(Glib::RefPtr<Gio::MountOperation>& mount_operation,
                                  const Glib::ustring& msg,
                                  const Glib::ustring& defaultUser,
                                  const Glib::ustring& defaultdomain,
                                  Gio::AskPasswordFlags flags)
{
std::cerr << "Gvfs on signal_ask_password ask password: " << msg.raw() << std::endl
          << "Gvfs on signal_ask_password default user: " << defaultUser.raw() << std::endl
          << "Gvfs on signal_ask_password default domain: " << defaultdomain.raw() << std::endl;

    if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) &&
        mount_operation->get_username().empty() &&
        mount_operation->get_password().empty())
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
    if (m_uiCallbacks)
        {
            int answer = g_mount_operation_get_choice(op);
            m_uiCallbacks->onAskQuestion(message, choices, answer);
            if (answer != -1)
                {
                    g_mount_operation_set_choice(op, answer);
                    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
                }
                else g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        }
        else g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
}

void GvfsService::on_ask_password(GMountOperation* op, const char* message,
                                  const char* default_user,
                                  const char* default_domain,
                                  GAskPasswordFlags flags)
{
std::cerr << "Gvfs on signal_ask_password ask password: " << message << std::endl
          << "Gvfs on signal_ask_password default user: " << default_user << std::endl
          << "Gvfs on signal_ask_password default domain: " << default_domain << std::endl;
    if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) &&
        (g_mount_operation_get_username(op) == nullptr) &&
        (g_mount_operation_get_password(op) == nullptr))
    {
std::cerr << "Gvfs on signal_ask_password set anonymous" << std::endl;
        g_mount_operation_set_anonymous(op, true);
    }
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
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
    m_mainLoop->quit();
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
    m_mainLoop->quit();
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
    m_mainLoop->quit();
    return l_mount;
}
