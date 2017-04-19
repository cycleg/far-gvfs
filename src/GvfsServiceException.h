#pragma once

#include <glibmm/error.h>

class GvfsServiceException: public Glib::Error
{
  public:
    GvfsServiceException(): Error() {}
    GvfsServiceException(GQuark error_domain, int error_code, const Glib::ustring& message):
      Error(error_domain, error_code, message) {}
    GvfsServiceException(const GvfsServiceException& other): Error(other) {}
};
