#ifndef GLIBMMCONF_H
#define GLIBMMCONF_H

#if ((GLIBMM_MAJOR_VERSION > 2) || ((GLIBMM_MAJOR_VERSION == 2) && (GLIBMM_MINOR_VERSION >= 66) && (GLIBMM_MICRO_VERSION > 6)))
#define USE_GIO_MOUNTOPERATION_ONLY
#endif

#endif
