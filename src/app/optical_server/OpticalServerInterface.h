
#include <windows.h>
#include <vrpn_Types.h>
#include <glui.h>


// an OpenGL + GLUI window
class OpticalServerInterface
{
public:
	static OpticalServerInterface* getInterface( );

	void setImage( vrpn_uint8* buffer, int width, int height );

protected:

	int image_window;
	GLUI* glui_window;
	HANDLE ifaceThread;
	vrpn_uint8* lastImage;
	int lastImageWidth, lastImageHeight;

	static OpticalServerInterface* instance;
	static bool interfaceShutdown;

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

	bool threadReady;
};
