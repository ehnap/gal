#ifndef GALYD_GLOBAL_H
#define GALYD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GALYD_LIBRARY)
#  define GALYDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GALYDSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GALLIVE_GLOBAL_H
