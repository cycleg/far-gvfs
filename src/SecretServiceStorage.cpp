/*
 * SecretServiceStorage.cpp
 *
 *  Created on: 26.05.2017
 *      Author: cycleg
 */
#include <atomic>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <WideMB.h> // far2l/utils
#include "SecretServiceStorage.h"

#define UNUSED(x) (void)x;

using namespace std::placeholders;

const char* RecordLabel = "Far-gvfs password record";

namespace {

///
/// @brief Таблица обратных вызовов для асинхронных операций с безопасным хранилищем.

/// Обратные вызовы -- обращения к методам класса SecretServiceStorage для
/// конкретного экземпляра этого класса. В конструкторе SecretServiceStorage
/// экземпляр регистрируется в этой таблице, в деструкторе -- удаляется из нее.
///
/// При манипуляциях с таблицей потокобезопасность обеспечивается стратегией
/// "один писатель, много читателей". Попытки зарегистрировать объект в таблице
/// или удалить из нее задерживаются до тех пор, пока не завершатся все
/// текущие обратные вызовы. Эти операции выполняются за фиксированное время,
/// в отличие от обратных вызовов. Обратные вызовы могут выполняться
/// параллельно.
///
/// @author cycleg
///
class PasswordCallbacks
{
  friend class ::SecretServiceStorage; ///< Только экземпляры
                                       ///< SecretServiceStorage могут
                                       ///< манипулировать таблицей.

  public:
    PasswordCallbacks(): m_readers(0), m_lock(m_mapMutex, std::defer_lock)
    {
    }

    void stored(GObject* source, GAsyncResult* result, gpointer user_data)
    {
      if (m_readers++ == 0) m_lock.lock();
      auto i = m_callbacks.find(static_cast<SecretServiceStorage*>(user_data));
      if (i != m_callbacks.end()) i->second.stored(source, result, user_data);
      if (--m_readers == 0) m_lock.unlock();
    }

    void lookup(GObject* source, GAsyncResult* result, gpointer user_data)
    {
      if (m_readers++ == 0) m_lock.lock();
      auto i = m_callbacks.find(static_cast<SecretServiceStorage*>(user_data));
      if (i != m_callbacks.end()) i->second.lookup(source, result, user_data);
      if (--m_readers == 0) m_lock.unlock();
    }

    void cleared(GObject* source, GAsyncResult* result, gpointer user_data)
    {
      if (m_readers++ == 0) m_lock.lock();
      auto i = m_callbacks.find(static_cast<SecretServiceStorage*>(user_data));
      if (i != m_callbacks.end()) i->second.cleared(source, result, user_data);
      if (--m_readers == 0) m_lock.unlock();
    }

  private:
    struct callbacks
    {
      std::function<void(GObject*, GAsyncResult*, gpointer)>
        stored, ///< обратный вызов для сохранения пароля
        lookup, ///< обратный вызов для поиска пароля
        cleared; ///< обратный вызов для удаления пароля
    };

    void registerObj(SecretServiceStorage* _this, callbacks& cb)
    {
      // ожидаем, когда завершатся все текущие обратные вызовы
      std::lock_guard<std::mutex> lck(m_mapMutex);
      m_callbacks.emplace(_this, cb);
    }

    void unregisterObj(SecretServiceStorage* _this)
    {
      std::lock_guard<std::mutex> lck(m_mapMutex);
      auto i = m_callbacks.find(_this);
      if (i != m_callbacks.end()) m_callbacks.erase(i);
    }

    std::atomic_int m_readers; ///< Число потоков-читателей таблицы (извлекших
                               ///< указатели на вызовы).
    std::mutex m_mapMutex; ///< Мутекс на запись таблицы обратных вызовов.
                           ///< Заперт, если есть хотя бы один читатель таблицы
                           ///< или имеется писатель в таблицу.
    std::unique_lock<std::mutex> m_lock; ///< Закрыт, если есть хотя бы один
                                         ///< читатель таблицы.
    std::unordered_map<SecretServiceStorage*, callbacks>
      m_callbacks; ///< Таблица обратных вызовов: ключ -- указатель на целевой
                   ///< объект, методы которого будут вызываться.
                                                                  
};

PasswordCallbacks callbacksRegistry;

extern "C" void password_stored_wrapper(GObject* source, GAsyncResult* result,
                                        gpointer user_data)
{
  callbacksRegistry.stored(source, result, user_data);
}

extern "C" void password_lookup_wrapper(GObject* source, GAsyncResult* result,
                                        gpointer user_data)
{
  callbacksRegistry.lookup(source, result, user_data);
}

extern "C" void password_cleared_wrapper(GObject* source, GAsyncResult* result,
                                         gpointer user_data)
{
  callbacksRegistry.cleared(source, result, user_data);
}

///
/// @brief Схема сохранения паролей для libsecret.

/// @author cycleg
///
const SecretSchema* SecretServiceStorageSchema() G_GNUC_CONST;

const SecretSchema* SecretServiceStorageSchema()
{
    static const SecretSchema the_schema = {
        "org.far2l.gvfspanel.secure.storage.password", SECRET_SCHEMA_NONE,
        {
            {  "id", SECRET_SCHEMA_ATTRIBUTE_STRING }, ///< MountPoint storage ID
            {  "NULL", SECRET_SCHEMA_ATTRIBUTE_STRING },
        }
    };
    return &the_schema;
}

} // anonymous namespace

#define SECRET_SERVICE_STORAGE_SCHEMA SecretServiceStorageSchema()

SecretServiceStorage::SecretServiceStorage(): m_result(false)
{
  // регистрируем наши обратные вызовы
  PasswordCallbacks::callbacks cb;
  cb.stored = std::bind(&SecretServiceStorage::onPasswordStored, this, _1, _2,
                        _3);
  cb.lookup = std::bind(&SecretServiceStorage::onPasswordFound, this, _1, _2,
                        _3);
  cb.cleared = std::bind(&SecretServiceStorage::onPasswordRemoved, this, _1,
                         _2, _3);
  callbacksRegistry.registerObj(this, cb);
}

SecretServiceStorage::~SecretServiceStorage()
{
  callbacksRegistry.unregisterObj(this);
}

bool SecretServiceStorage::SavePassword(const std::wstring& id,
                                       const std::wstring& password)
{
  std::string idBuf(StrWide2MB(id)),
              passwordBuf(StrWide2MB(password));

  m_password.clear();
  m_result = false;

  Glib::init();
  Glib::RefPtr< Glib::MainContext > main_context = Glib::MainContext::create();
  // Чтобы контекст главного цикла Glib::MainLoop не отслеживал ничего, кроме
  // операций с ресурсом! Иначе блокируется пользовательский ввод Far.
  // Из руководства:
  // This will cause certain asynchronous operations (such as most GIO-based
  // I/O) which are started in this thread to run under context and deliver
  // their results to its main loop, rather than running under the global
  // default context in the main thread.
  g_main_context_push_thread_default(main_context->gobj());
  m_mainLoop = Glib::MainLoop::create(main_context, false);

  secret_password_store(SECRET_SERVICE_STORAGE_SCHEMA, // The password type.
                        SECRET_COLLECTION_DEFAULT, // Where to save it: on disk.
                        RecordLabel,
                        passwordBuf.c_str(), // The password itself.
                        nullptr, // Cancellation object.
                        password_stored_wrapper, // Callback
                        this, // User data for callback.

                        // These are the attributes.
                        "id", idBuf.c_str(),

                        nullptr); // Always end with NULL.

  m_mainLoop->run();
  // Из руководства:
  // In some cases however, you may want to schedule a single operation
  // in a non-default context, or temporarily use a non-default context
  // in the main thread. In that case, you can wrap the call to the
  // asynchronous operation inside a
  // g_main_context_push_thread_default() / g_main_context_pop_thread_default()
  // pair...
  // Второй вариант, видимо, наш случай. Без этого вызова Glib выдает assert.
  g_main_context_pop_thread_default(main_context->gobj());
  return m_result;
}

bool SecretServiceStorage::LoadPassword(const std::wstring& id,
                                       std::wstring& password)
{
  std::string idBuf(StrWide2MB(id));

  m_password.clear();
  m_result = false;
  password.clear();

  Glib::init();
  Glib::RefPtr< Glib::MainContext > main_context = Glib::MainContext::create();
  g_main_context_push_thread_default(main_context->gobj());
  m_mainLoop = Glib::MainLoop::create(main_context, false);

  secret_password_lookup(SECRET_SERVICE_STORAGE_SCHEMA,
                         nullptr, // Cancellation object.
                         password_lookup_wrapper, // Callback
                         this, // User data for callback.

                         // These are the attributes.
                         "id", idBuf.c_str(),

                         nullptr); // Always end with NULL.
  m_mainLoop->run();
  g_main_context_pop_thread_default(main_context->gobj());
  if (m_result)
  {
    StrMB2Wide(m_password, password);
  }
  return m_result;
}

void SecretServiceStorage::RemovePassword(const std::wstring& id)
{
  std::string idBuf(StrWide2MB(id));

  m_password.clear();
  m_result = false;

  Glib::init();
  Glib::RefPtr< Glib::MainContext > main_context = Glib::MainContext::create();
  g_main_context_push_thread_default(main_context->gobj());
  m_mainLoop = Glib::MainLoop::create(main_context, false);

  secret_password_clear(SECRET_SERVICE_STORAGE_SCHEMA,
                        nullptr, // Cancellation object.
                        password_cleared_wrapper, // Callback
                        this, // User data for callback.

                        // These are the attributes.
                        "id", idBuf.c_str(),

                        nullptr); // Always end with NULL.
  m_mainLoop->run();
  g_main_context_pop_thread_default(main_context->gobj());
}

void SecretServiceStorage::onPasswordStored(GObject* source,
                                           GAsyncResult* result,
                                           gpointer user_data)
{
  UNUSED(source);
  UNUSED(user_data);

  GError* error = nullptr;
  secret_password_store_finish(result, &error);
  m_result = (error == nullptr);
  if (!m_result)
  {
    std::cerr << "SecretServiceStorage couldn't save password: "
              << error->message << std::endl;
    g_error_free (error);
  }
  m_mainLoop->quit();
}

void SecretServiceStorage::onPasswordFound(GObject* source,
                                          GAsyncResult* result,
                                          gpointer user_data)
{
  UNUSED(source);
  UNUSED(user_data);

  GError* error = nullptr;
  gchar* password = secret_password_lookup_finish(result, &error);
  m_result = (error == nullptr);
  if (m_result)
    {
      if (password != nullptr)
        {
          m_password = password;
          secret_password_free(password);
        }
        else
        {
          std::cerr << "SecretServiceStorage couldn't find password" << std::endl;
          m_result = false;
        }
    }
    else
    {
      std::cerr << "SecretServiceStorage couldn't find password: "
                << error->message << std::endl;
      g_error_free (error);
    }
  m_mainLoop->quit();
}

void SecretServiceStorage::onPasswordRemoved(GObject* source,
                                            GAsyncResult* result,
                                            gpointer user_data)
{
  UNUSED(source);
  UNUSED(user_data);

  GError* error = nullptr;
  secret_password_clear_finish(result, &error);
  // отсутствие пароля -- не ошибка
  m_result = (error == nullptr);
  if (!m_result)
  {
    std::cerr << "SecretServiceStorage couldn't remove password: "
              << error->message << std::endl;
    g_error_free (error);
  }
  m_mainLoop->quit();
}
