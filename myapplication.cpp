#include "myapplication.h"

#ifdef SINGLE_INSTANCE
MyApplication::MyApplication ( const QString & appId, int & argc, char ** argv ) 
	: QtSingleApplication(appId, argc, argv)
{

}

#endif

