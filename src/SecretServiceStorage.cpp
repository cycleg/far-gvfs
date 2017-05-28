/*
 * SecretServiceStorage.cpp
 *
 *  Created on: 26.05.2017
 *      Author: cycleg
 */
#include <iostream>
#include <functional>
#include <utils.h> // far2l/utils
#include "SecretServiceStorage.h"

#define UNUSED(x) (void)x;

using namespace std::placeholders;

const char* RecordLabel = "Far-gvfs password record";

namespace {

std::function<void(GObject*, GAsyncResult*, gpointer)>
  password_stored_callback, password_lookup_callback,
  password_cleared_callback;

extern "C" void password_stored_wrapper(GObject* source, GAsyncResult* result,
                                        gpointer user_data)
{
  password_stored_callback(source, result, user_data);
}

extern "C" void password_lookup_wrapper(GObject* source, GAsyncResult* result,
                                        gpointer user_data)
{
  password_lookup_callback(source, result, user_data);
}

extern "C" void password_cleared_wrapper(GObject* source, GAsyncResult* result,
                                         gpointer user_data)
{
  password_cleared_callback(source, result, user_data);
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
  password_stored_callback = std::bind(&SecretServiceStorage::onPasswordStored,
                                       this, _1, _2, _3);
  password_lookup_callback = std::bind(&SecretServiceStorage::onPasswordFound,
                                       this, _1, _2, _3);
  password_cleared_callback = std::bind(&SecretServiceStorage::onPasswordRemoved,
                                       this, _1, _2, _3);
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
                        nullptr, // User data for callback.

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
                         nullptr, // User data for callback.

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
                        nullptr, // User data for callback.

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

  GError* error = NULL;
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
