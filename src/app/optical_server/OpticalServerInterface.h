
#include <windows.h>
#include <vrpn_Types.h>
#include <glui.h>
#include "nmm_Microscope_SEM_diaginc.h"

#ifndef OPTICALSERVERINTERFACE_H
#define OPTICALSERVERINTERFACE_H

// an OpenGL + GLUI window
class OpticalServerInterface
{
public:
	static OpticalServerInterface* getInterface( );

	void setImage( vrpn_uint8* buffer, int width, int height );
	void setMicroscope( nmm_Microscope_SEM_diaginc* m );

	int getBinning( );
	int getResolutionIndex( );
	void setBinning( int bin );
	void setResolutionIndex( int idx );

protected:

	int image_window;
	GLUI* glui_window;
	GLUI_Listbox* binningListbox;
	GLUI_RadioGroup* resRadioGroup;
	HANDLE ifaceThread;
	vrpn_uint8* lastImage;
	int lastImageWidth, lastImageHeight;
	 nmm_Microscope_SEM_diaginc* microscope;

	static OpticalServerInterface* instance;
	static bool interfaceShutdown;

private:
	OpticalServerInterface( );
	~OpticalServerInterface( );

	friend void OpticalServerInterface_gluiChangedResolution( int id );
	friend void OpticalServerInterface_gluiChangedBinning( int id );

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

#endif // OPTICALSERVERINTERFACE_H