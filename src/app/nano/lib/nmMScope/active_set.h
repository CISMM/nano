#ifndef	ACTIVE_SET_H
#define	ACTIVE_SET_H

#include <stm_cmd.h>  // for STM_NAME_LENGTH

class BCGrid;  // from BCGrid.h
class BCPlane;  // from BCPlane.h
class Point_results;  // from Point.h
class Point_value;
class Point_list;
class nmb_Dataset;  // from nmb_Dataset.h

//#include <nmm_Globals.h>  // was for USE_VRPN_MICROSCOPE

//---------------------------------------------------------------------------
// Class to keep track of which data sets are currently being sent by the
// microscope when it is in scan mode.
// Also provides a user interface to allow selections of new
// types.  Sends messages to the microscope to request changes when the
// user enables them with buttons.  Modifies the buttons to follow when
// the microscope sends a message telling which it is sending.

//XXX Need to encapsulate read/write routines to microscope.  When we do,
// the object that handles the microscope should be passed to the constructor
// as a parameter, to be used by Update_microscope().

#define	MAX_CHANNELS		(100)

#ifndef USE_VRPN_MICROSCOPE
class Microscope;
#else
class nmm_Microscope_Remote;	// Added by Tiger
#endif

class	Channel_selector
{
    friend void tcl_update_callback(const char *name, int val, void *udata);
    public:
    Channel_selector (nmb_ListOfStrings *, nmb_Dataset *);
	virtual ~Channel_selector (void);

	// User-interface side of things, response to changes by the user
	int	Set(const char *channel);
	int	Unset(const char *channel);

    int    Is_set(const char *channel) { return (checklist->Is_set(channel)); }
	// If changes, request new list
	// Changed to allow instantiation of this as a generic
        // channel selector that doesn't contain references to data; 
#ifndef USE_VRPN_MICROSCOPE
	virtual int Update_microscope (Microscope *);
#else
	virtual int Update_microscope (nmm_Microscope_Remote *);
#endif
	void	Changed(int c) { change_from_tcl = c; }
	int	Changed (void) const { return change_from_tcl; }

	// Commands called in response to microscope reports
	int	Clear_channels(void);		// Clears out all channels
    // Add channel, and set checklist button associated with channel.
	int Add_channel(char *name, char *units, 
		float offset, float scale);
    // Add channel, and set checklist button and entry associated with channel.
        int Add_channel(char *name, char *units,
		float offset,float scale, int num_samples);
	inline float DAC_to_units(int channel, float value)
	    {return value*channels[channel].scale + channels[channel].offset;}

	// for aesthetics:
	inline float units_to_DAC(int channel, float value)
	    {return (value - channels[channel].offset)/channels[channel].scale;}

	int	Num_channels (void) const { return numchannels; }
	const Tclvar_checklist_with_entry * Checklist (void) const;

    protected:
	struct {
		char	name [STM_NAME_LENGTH];		// Name of the data set
		char	units [STM_NAME_LENGTH];	// Name of the data set
		float	offset;		// plane data is (offset + data*scale)
		float	scale;
	} channels[MAX_CHANNELS];
	int	numchannels;		// Channels ready for from scope
	int	change_from_tcl;	// Values changed in tcl, usually
                                        // requires update to microscope
        int     change_from_microscope;    //change made by microscope, 
                                        // so we can ignore changes from tcl. 

	Tclvar_checklist_with_entry	*checklist;
	nmb_ListOfStrings	* namelist;

        nmb_Dataset * d_dataset;
};

// "Scan" means the AFM is rastering back and forth, collecting a grid
// of data sets. This is used to specify which data sets.
class	Scan_channel_selector : public Channel_selector
{
    public:
	Scan_channel_selector (BCGrid * grid_to_track,
                               nmb_ListOfStrings *, nmb_Dataset *);
	virtual ~Scan_channel_selector (void);  // avoid compiler warnings

	// User-interface side of things, response to changes by the user
#ifndef USE_VRPN_MICROSCOPE
	virtual	int Update_microscope (Microscope *);
#else
	virtual int Update_microscope (nmm_Microscope_Remote *);  // Tiger
#endif

	// Commands called in response to microscope reports
	int Add_channel(char *name, char *units,
		float offset, float scale);

	int	Handle_old_report(int x, int y, long sec, long usec,
			      float val, float std);
	int	Handle_report(int x, int y, long sec, long usec, float *values,
			int numvalues);

    protected:
	BCGrid	*mygrid;
	BCPlane	*planes[MAX_CHANNELS];	// Planes to store data in
};

// "Point" means the AFM is going to a specific spot, and collecting
// a set of data. This is used to specify which data sets. 
class	Point_channel_selector : public Channel_selector
{
    public:
	Point_channel_selector (Point_results * point,
                                nmb_ListOfStrings *, nmb_Dataset *,
                                Point_list * pointList = NULL);
	~Point_channel_selector (void);  // avoid compiler warnings

	// User-interface side of things, response to changes by the user

#ifndef USE_VRPN_MICROSCOPE
	virtual	int Update_microscope (Microscope *);
#else
	virtual int Update_microscope (nmm_Microscope_Remote *);  // Tiger
#endif

	// Commands called in response to microscope reports
	int Add_channel(char *name, char *units,
		float offset, float scale, int num_samples);

	int	Handle_old_report(float x, float y, long sec, long usec,
			      float val, float std);
	int	Handle_report(float x, float y, long sec, long usec,
			float *vals, int numvalues, vrpn_bool accumulatePoints);
          ///< If accumulatePoints is true, stashes a copy of myresult
          ///< in d_pointList.

    protected:
	Point_results	*myresult;
	Point_value	*values[MAX_CHANNELS];	// Where to store the values
        Point_list * d_pointList;
};

// "Forcecurve" means the AFM goes to a specific spot, and then does
// a special withdraw, press-into-surface, widthdraw motion, all the while
// taking data.
class	ForceCurve_channel_selector : public Channel_selector
{
    public:
	ForceCurve_channel_selector (Point_results * point,
				     nmb_ListOfStrings *, nmb_Dataset *);
	virtual ~ForceCurve_channel_selector (void);  // avoid compiler warnings

	// User-interface side of things, response to changes by the user
#ifndef USE_VRPN_MICROSCOPE
	virtual int Update_microscope (Microscope *);
#else
	virtual int Update_microscope (nmm_Microscope_Remote *);  // Tiger
#endif

	// Commands called in response to microscope reports
	int Add_channel(char *name, char *units,
		float offset, float scale);

	int	Handle_report(float x, float y, float z, long sec, long usec,
			float *vals, int numvalues);

    protected:
	Point_results * myresult;
	Point_value * values [MAX_CHANNELS];
};

#endif
