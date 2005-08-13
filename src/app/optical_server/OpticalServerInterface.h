
#include "Tcl_Linkvar.h"
#include <windows.h>
#include <vrpn_Types.h>
#include "nmm_Microscope_SEM_optical.h"

#ifndef OPTICALSERVERINTERFACE_H
#define OPTICALSERVERINTERFACE_H

// an OpenGL + Tcl/Tk windows
class OpticalServerInterface
{
public:
	static OpticalServerInterface* getInterface( );

	void setImage( vrpn_uint8* buffer, int width, int height );
	void setMicroscope( nmm_Microscope_SEM_optical* m );

	int getBinning( ) { return binning; };
	int getResolutionIndex( ) { return resolutionIndex; };
	int getContrast( ) { return contrast; };
	int getExposure( ) { return (int) d_exposure; }
	void setBinning( int bin );
	void setResolutionIndex( int idx );
	void setContrast( int contrast );
	void setExposure( int exposure );

protected:

	int image_window;
	HANDLE ifaceThread;
	vrpn_uint8* lastImage;
	int lastImageWidth, lastImageHeight;
	nmm_Microscope_SEM_optical* microscope;

	static OpticalServerInterface* instance;
	static bool interfaceShutdown;

	Tclvar_list_of_strings  *d_binningList;
	Tclvar_list_of_strings  *d_resolutionList;
	Tclvar_list_of_strings  *d_contrastList;
	Tclvar_selector	*d_binningSelector;
	Tclvar_selector *d_resolutionSelector;
	Tclvar_selector *d_contrastSelector;
	Tclvar_float* d_exposure;  // in ms
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
	static void handle_exposure_changed( float new_value, void* userdata );

	bool threadReady;
};

#endif // OPTICALSERVERINTERFACE_H