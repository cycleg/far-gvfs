#pragma once

#include <glibmm/error.h>

/// 
/// @brief Класс-исключение для ошибок GvfsService

/// @author cycleg
///
class GvfsServiceException: public Glib::Error
{
  public:
    ///
    /// Конструктор.
    ///
    GvfsServiceException(): Error() {}
    ///
    /// Конструктор.
    ///
    /// @param [in] error_domain
    /// @param [in] error_code
    /// @param [in] message
    ///
    GvfsServiceException(GQuark error_domain, int error_code, const Glib::ustring& message):
      Error(error_domain, error_code, message) {}
    ///
    /// Копирующий конструктор.
    ///
    /// @param [in] other Копируемый экземпляр класса.
    ///
    GvfsServiceException(const GvfsServiceException& other): Error(other) {}
};
