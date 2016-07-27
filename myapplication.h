#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QtGlobal>

#ifdef SINGLE_INSTANCE
#include "qtsingleapplication.h"

class MyApplication : public QtSingleApplication
{
	Q_OBJECT

public:
	MyApplication ( const QString & appId, int & argc, char ** argv );

	virtual void commitData ( QSessionManager & /*manager*/ ) {
		// Nothing to do, let the application to close
	}

	inline static MyApplication * instance() {
		return qobject_cast<MyApplication*>(QApplication::instance());
	}
};

#endif

#endif

