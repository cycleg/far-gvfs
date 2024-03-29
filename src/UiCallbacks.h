#pragma once

#include <vector>
#include <glibmm/ustring.h>
#include <farplug-wide.h>

/// 
/// @brief Обратные вызовы в интерфейс пользователя (UI) из операций с ресурсами.

/// Класс реализует шаблон "Стратегия" для класса-интерфейса к GVFS.
///
/// @author cycleg
///
class UiCallbacks
{
  public:
    ///
    /// Кнструктор.
    ///
    /// @param [in] info
    ///
    UiCallbacks(PluginStartupInfo& info);

    ///
    /// Ответ пользователя в сигнале "ask question" при монтировании ресурса.
    ///
    /// @param [in] message Сообщение (вопрос) пользователю.
    /// @param [in] choices Варианты ответа.
    /// @param [in,out] choice На входе - вариант по умолчанию, на выходе --
    ///                        выбранный пользователем.
    ///
    void onAskQuestion(const Glib::ustring& message,
                       const std::vector<Glib::ustring>& choices,
                       int& choice) const;

    ///
    /// Ответ пользователя в сигнале "ask question" при монтировании ресурса.
    ///
    /// @param [in] message Сообщение (вопрос) пользователю.
    /// @param [in] choices Варианты ответа.
    /// @param [in,out] choice На входе - вариант по умолчанию, на выходе --
    ///                        выбранный пользователем.
    ///
    void onAskQuestion(char* message, char** choices, int& choice) const;

  private:
    static const int AskQuestionDlgWidth = 78; ///< Макс. ширина диалога
                                               ///< "Ask question".

    PluginStartupInfo& m_pStartupInfo; ///< Интерактивность черех UI far2l.
};
