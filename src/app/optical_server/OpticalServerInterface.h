
// Horrible hack: Code that uses this function must #include Tcl_Linkvar.h
// BEFORE windows.h and before this file is included.  If it doesn't then
// there is a fight between Tk and STL, and Tk loses.

#include <windows.h>
#include <vrpn_Types.h>
#include "nmm_Microscope_SEM_diaginc.h"

#ifndef OPTICALSERVERINTERFACE_H
#define OPTICALSERVERINTERFACE_H

// an OpenGL + Tcl/Tk windows
class OpticalServerInterface
{
public:
	static OpticalServerInterface* getInterface( );

	void setImage( vrpn_uint8* buffer, int width, int height );
	void setMicroscope( nmm_Microscope_SEM_diaginc* m );

	int getBinning( ) { return binning; };
	int getResolutionIndex( ) { return resolutionIndex; };
	int getContrast( ) { return contrast; };
	void setBinning( int bin );
	void setResolutionIndex( int idx );
	void setContrast( int contrast );

protected:

	int image_window;
	HANDLE ifaceThread;
	vrpn_uint8* lastImage;
	int lastImageWidth, lastImageHeight;
	nmm_Microscope_SEM_diaginc* microscope;

	static OpticalServerInterface* instance;
	static bool interfaceShutdown;

	Tclvar_list_of_strings  *d_binningList;
	Tclvar_list_of_strings  *d_resolutionList;
	Tclvar_list_of_strings  *d_contrastList;
	Tclvar_selector	*d_binningSelector;
	Tclvar_selector *d_resolutionSelector;
	Tclvar_selector *d_contrastSelector;
	int binning;
	int resolutionIndex;
	int contrast;

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

	static void handle_binning_changed(char *new_value, void *userdata);
	static void handle_resolution_changed(char *new_value, void *userdata);
	static void handle_contrast_changed(char *new_value, void *userdata);

	bool threadReady;
};

#endif // OPTICALSERVERINTERFACE_H