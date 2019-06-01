#ifndef GALLIVE_GLOBAL_H
#define GALLIVE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GALLIVE_LIBRARY)
#  define GALLIVESHARED_EXPORT Q_DECL_EXPORT
#else
#  define GALLIVESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GALLIVE_GLOBAL_H
