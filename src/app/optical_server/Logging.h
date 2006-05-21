
#ifndef OPTICAL_SERVER_LOGGING_H
#define OPTICAL_SERVER_LOGGING_H

#include <nmm_Microscope_SEM_Remote.h>

class Logging
{
public:
	static Logging* getInstance();

	// returns the file name that it will log to
	const char* startLogging( );

	// returns the file name that was logged to, or NULL
	// if no logging was going on.
	const char* stopLogging( );

	void mainloop( );

protected:
	static Logging* instance;
	static CRITICAL_SECTION cslogging;
	static char* connName;

	nmm_Microscope_SEM_Remote* sem;
	vrpn_Connection* conn;

	char* logfileName;

	void makeNewLogfileName( );

private:
	Logging( );
	Logging( const Logging& );
	Logging& operator=( const Logging& );
	~Logging( );
	
	bool isLogging;
	static void SEM_handler( void*, const nmm_Microscope_SEM_ChangeHandlerData& d );

	static int staticInitializer;
	static int staticInitialize( );
};





#endif // OPTICAL_SERVER_LOGGING_H