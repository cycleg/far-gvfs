#pragma once

#include <plugin.hpp>

/// 
/// Обратные вызовы в интерфейс пользователя (UI) из операций с ресурсами.

/// Класс реализует шаблон "Стратегия" для класса-интерфейса к GVFS.
///
/// @author cycleg
///
class UiCallbacks
{
  public:
    UiCallbacks(PluginStartupInfo& info);

    ///
    /// Ответ пользователя в сигнале "ask question" при монтировании ресурса
    ///
    /// @param message Сообщение (вопрос) пользователю
    /// @param choices Варианты ответа
    /// @param choice На входе - вариант по умолчанию, на выходе - выбранный
    ///               пользователем
    ///
    void onAskQuestion(char* message, char** choices, int& choice) const;

  private:
    PluginStartupInfo& m_pStartupInfo; ///< Интерактивность черех UI far2l.
};
