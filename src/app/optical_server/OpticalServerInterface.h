
#include <windows.h>
#include <vrpn_Types.h>


// an OpenGL + GLUI window
class OpticalServerInterface
{
public:
	static OpticalServerInterface* getInterface( );

protected:

	int main_window;
	HANDLE ifaceThread;

	static OpticalServerInterface* instance;

private:
	OpticalServerInterface( );
	~OpticalServerInterface( );

	friend void OpticalServerInterface_myGlutRedisplay( );
	friend void OpticalServerInterface_myGlutKeyboard( unsigned char key, int x, int y );
	friend void OpticalServerInterface_myGlutSpecial( int, int, int );
	friend void OpticalServerInterface_myGlutMouse( int button, int state, int x, int y );
	friend void OpticalServerInterface_myGlutMotion( int x, int y );
	friend void OpticalServerInterface_myGlutReshape( int w, int h );
	friend void OpticalServerInterface_myGlutIdle( );
	friend DWORD WINAPI OpticalServerInterface_mainloop( LPVOID lpParameter );
};
