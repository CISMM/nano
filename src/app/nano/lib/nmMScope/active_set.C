#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	"Tcl_Linkvar.h"
#include	"Tcl_Netvar.h"
#include	"active_set.h"
#include	<BCPlane.h>
#include	<nmb_Globals.h>

//#include	"stm_cmd.h"

//#include <nmm_Globals.h>  // was for USE_VRPN_MICROSCOPE,
                            // which is now controlled by the Makefile

#ifndef USE_VRPN_MICROSCOPE
#include "Microscope.h"
#else
#include "nmm_MicroscopeRemote.h"
#endif

void tcl_update_callback(const char */*name*/, int /*val*/, void *udata)
{
	Channel_selector	*me = (Channel_selector*)(udata);

	//fprintf(stderr, "tcl_update_callback: val = %d\n", val);
	// Status has changed
	me->change_from_tcl =1;


	// Set or unset the variable that changed in the variable that changed.
	/*	if (val) {
		me->Set(name);
	} else {
		me->Unset(name);
	}
	*/
}

Channel_selector::Channel_selector (nmb_ListOfStrings * namelist,
                                    nmb_Dataset * dataset) :
    numchannels (0),
    change_from_tcl (0),
    change_from_microscope (0),
    checklist (NULL),
    namelist (namelist),
    d_dataset (dataset) {

}

// Moved here because virtual functions can't be inlined but
// declaration in the header implies inlining request for
// some compilers - TCH 1 Oct 97

Channel_selector::~Channel_selector (void) {
  if (checklist) {
    delete checklist;
    checklist = NULL;
  }
}

int	Channel_selector::Set(const char *channel)
{
	// Set the channel in the checklist.  If this fails, add a new
	// checkbox in (Set_checkbox returns -1 if it can't find the
	// checkbox).
	if (checklist->Set_checkbox(channel)) {
		if (checklist->Add_checkbox(channel,1)) {
			fprintf(stderr,
			  "Channel_selector::Set(): Can't add %s\n",
			  channel);
			return -1;
		}
	}
    return 0;
}

int	Channel_selector::Unset(const char *channel)
{
	// Unset the channel in the checklist.  If this fails, add a new
	// checkbox in (Set_checkbox returns -1 if it can't find the
	// checkbox).
	if (checklist->Unset_checkbox(channel)) {
		if (checklist->Add_checkbox(channel,0)) {
			fprintf(stderr,
			  "Channel_selector::Unset(): Can't add %s\n",
			  channel);
			return -1;
		}
	}
    return 0;
}

#ifndef USE_VRPN_MICROSCOPE

// virtual
int Channel_selector::Update_microscope (Microscope *) {
  return 0;
}

#else

// virtual
int Channel_selector::Update_microscope (nmm_Microscope_Remote *) {
  return 0;
}

#endif


int	Channel_selector::Clear_channels(void)
{
	int	ret = 0;
	int	i;

	// Mark this a being a change from the microscope.
	change_from_microscope = 1;

	// Clear all of the channels that are currently getting data
	numchannels = 0;

	// Clear all of the checkboxes
	for (i = 0; i < checklist->Num_checkboxes(); i++) {
	  if (checklist->Unset_checkbox(checklist->Checkbox_name(i))) {
	    fprintf(stderr,
	      "Channel_selector::Clear_channels(): Can't unset checkbox %s\n",
		channels[i].name);
	    ret = -1;
	  }
	}
	return ret;
}

int	Channel_selector::Add_channel(char *name, char *units,
		float offset,float scale)
{
	// Make sure we have room for another one
	if (numchannels >= MAX_CHANNELS) {
	    fprintf(stderr,"Channel_selector::Add_channel(): Too many\n");
	    return -1;
	}

	// See if we can set this checkbox in the checklist.  If not, add it
	if (checklist->Set_checkbox(name)) {
	    if (checklist->Add_checkbox(name,1)) {
		fprintf(stderr,"Channel_selector::Add_channel(): Can't add %s to checklist\n",name);
		return -1;
	    }
	}

	// Fill in the rest and increment the count
	strncpy(channels[numchannels].units,units,
		sizeof(channels[numchannels].units));
	channels[numchannels].offset = offset;
	channels[numchannels].scale = scale;
	numchannels++;

	// DEBUG
// 	fprintf(stderr,"Channel_selector::Add_channel, adding %s, which %d\n",
// 		name, numchannels-1);
	return numchannels-1;		// Tell which channel
}

int	Channel_selector::Add_channel(char *name, char *units,
		float offset,float scale, int num_samples)
{
	// Make sure we have room for another one
	if (numchannels >= MAX_CHANNELS) {
	    fprintf(stderr,"Channel_selector::Add_channel(): Too many\n");
	    return -1;
	}

	// See if we can set this checkbox in the checklist.  If not, add it
	if (checklist->Set_checkbox(name)) {
	    if (checklist->Add_checkbox_entry(name,1, num_samples)) {
		fprintf(stderr,"Channel_selector::Add_channel(): Can't add %s to checklist\n",name);
		return -1;
	    }
	} else { // if set succeeds, make sure we set entry field, too.
	   checklist->Set_checkbox_entry(name, num_samples);
	}

	// Fill in the rest and increment the count
	strncpy(channels[numchannels].units,units,
		sizeof(channels[numchannels].units));
	channels[numchannels].offset = offset;
	channels[numchannels].scale = scale;
	numchannels++;

	return numchannels-1;		// Tell which channel
}

const Tclvar_checklist_with_entry * Channel_selector::Checklist (void) const {
  return checklist;
}


Scan_channel_selector::Scan_channel_selector(BCGrid *grid_to_track,
    nmb_ListOfStrings * namelist, nmb_Dataset * dataset) :
		Channel_selector (namelist, dataset)
{
	change_from_tcl = 1;	// Report to the microscope what we want
	numchannels = 0;	// No channels mapped yet
	mygrid = grid_to_track;

	// Get a new checklist.
	checklist = new Tclvar_checklist_with_entry(".data_sets.scan");
	if (checklist == NULL) {
	    fprintf(stderr,"Scan_channel_selector(): Can't make checklist\n");
	    return;
	}

	// Put the known types into the checklist, but don't require planes for
	// them.
	checklist->Add_checkbox("Topography-Forward",1);
	checklist->Add_checkbox("Topography-Reverse",0);
	checklist->Add_checkbox("Internal Sensor-Forward",0);
	checklist->Add_checkbox("Internal Sensor-Reverse",0);
	checklist->Add_checkbox("Z Modulation-Forward",0);
	checklist->Add_checkbox("Z Modulation-Reverse",0);
	checklist->Add_checkbox("Lateral Force-Forward",0);
	checklist->Add_checkbox("Lateral Force-Reverse",0);
	checklist->Add_checkbox("IN 1-Forward",0);
	checklist->Add_checkbox("IN 1-Reverse",0);
	checklist->Add_checkbox("IN 2-Forward",0);
	checklist->Add_checkbox("IN 2-Reverse",0);
	checklist->Add_checkbox("FastTrack-Forward",0);
	checklist->Add_checkbox("FastTrack-Reverse",0);
	checklist->Add_checkbox("Z Piezo-Forward",0);
	checklist->Add_checkbox("Z Piezo-Reverse",0);

	// Set up a callback from the checklist
	checklist->addCallback(tcl_update_callback,this);

	// Set up for old default, which was Topography and standard deviation
	if ((d_dataset->inputGrid->readMode() == READ_DEVICE) ||
            (d_dataset->inputGrid->readMode() == READ_STREAM)) {
            if (Add_channel("Topography-Forward","nm",0,1)) {
		fprintf(stderr,"Scan_channel_selector(): Can't get height\n");
	  }
	}
}

Scan_channel_selector::~Scan_channel_selector (void) {

}

#ifndef USE_VRPN_MICROSCOPE
int	Scan_channel_selector::Update_microscope (Microscope * microscope)
#else
int     Scan_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
#endif
{
	// If we have changed since the last time we did this, send a
	// message to the microscope requesting a new set of data
	// channels.
    if (change_from_tcl) {
	change_from_tcl = 0;
	if (change_from_microscope) {
	    change_from_microscope = 0;
	    //fprintf(stderr, "Update_microscope Scan: ignoring update req.\n");
	    return 0;
	} else {
	    if (microscope->GetNewScanDatasets(checklist) == -1)
		return -1;
	}
    }
    return 0;
}

int	Scan_channel_selector::Add_channel(char *name, char *units,
		float offset,float scale)
{
	int	which;

	// Call the base class method to set up the generic channel
	// information.
	// The base class method returns the index of the new channel
	// or -1.  Derived class method returns 0 on success, -1 on failure.
	which = Channel_selector::Add_channel(name, units, offset, scale);
	if (which == -1) {
	    fprintf(stderr,"Scan_channel_selector::Add_channel(): Failed\n");
	    return -1;
	}

	// See if we can lookup the plane for this checkbox.  If not, make one
	if ( (planes[which] = mygrid->getPlaneByName(name)) == NULL) {
	  if ((planes[which] = mygrid->addNewPlane(name,
	       units, 0)) == NULL) {
	    fprintf(stderr,"Scan_channel_selector::Add_channel(): Can't add %s plane\n",name);
	    numchannels--;
	    return -1;
	  }
	}

	// see if we can lookup the image corresponding to this plane. If
 	// not, we add it to the image list
        if ( (d_dataset->dataImages->getImageByName(name)) == NULL) {
	  nmb_ImageGrid *new_image = new nmb_ImageGrid(planes[which]);
	  d_dataset->dataImages->addImage((nmb_Image *)new_image);
	}

	// Add this to the list of available data sets for mapping
	// (The add will fail if this entry is already there.  We
	// ignore this.)
	namelist->addEntry(name);

	return 0;
}

//XXX Should remove this code and only have Handle_report.

int     Scan_channel_selector::Handle_old_report(
		int x, int y,
		long sec, long usec,
		float value, float std_dev)
{
	// Update the height and standard deviation planes.
	// This assumes that they are the first two, which was the case
	// in the old stream files which won't set this up
	if (numchannels>0) {
		planes[0]->setValue(x, y,
			channels[0].offset + channels[0].scale*value);
		planes[0]->setTime(x, y, sec, usec);
	}
	if (numchannels>1) {
		planes[1]->setValue(x, y, 
			channels[1].offset + channels[1].scale*std_dev);
		planes[1]->setTime(x, y, sec, usec);
	}

	// Tell the display to update the subgrid.
	d_dataset->range_of_change.AddPoint(x, y);

	return 0;
}

int     Scan_channel_selector::Handle_report( int x, int y,
		long sec, long usec, float *values, int numvalues)
{
	int	i;
	//float offset, scale;
	//char name[64], units[64];

	if (numvalues != numchannels) {
	  fprintf(stderr,
		  "Scan_channel_selector::Handle_report(): wrong # channels\n");
	  fprintf(stderr,"    (got %d, expected %d)\n",numvalues,numchannels);
	  return -1;
	}

	// Update each of the planes with the values.
	for (i = 0; i < numchannels; i++) {
		planes[i]->setValue(x, y,
			channels[i].offset + channels[i].scale*values[i]);
		planes[i]->setTime(x, y, sec, usec);
	}

	// DEBUG
// 	if (numchannels >=2) {
// 	   if (x == 0) {
// 	      fprintf(stderr, "%d\t%f\t%f Raw data values\n", y, values[0], values[1]);
// 	   }
// 	}

	// Tell the display to update the subgrid.
	d_dataset->range_of_change.AddPoint(x, y);

	return 0;
}

Point_channel_selector::Point_channel_selector
             (Point_results * point,
              nmb_ListOfStrings * namelist, nmb_Dataset * dataset,
              Point_list * pointList) :
    Channel_selector (namelist, dataset),
    myresult (point),
    d_pointList (pointList)
{
	change_from_tcl = 1;		// Report to the microscope what we want
	numchannels = 0;	// No channels mapped yet
	myresult = point;	// Where to store values

	// Get a new checklist.
	checklist = new Tclvar_checklist_with_entry(".data_sets.touch");
	if (checklist == NULL) {
	    fprintf(stderr,"Point_channel_selector(): Can't make checklist\n");
	    return;
	}

	// Put the known types into the checklist.
   // Turned more of these "on" -CCWeigle 8/6/99
	checklist->Add_checkbox_entry("Topography",1, 90);
	checklist->Add_checkbox_entry("Internal Sensor",1, 10);
	checklist->Add_checkbox_entry("Z Modulation",0, 10);
	checklist->Add_checkbox_entry("Lateral Force",1, 10);
	checklist->Add_checkbox_entry("IN 1",0, 10);
	checklist->Add_checkbox_entry("IN 2",0, 10);
	checklist->Add_checkbox_entry("FastTrack",0, 10);
	checklist->Add_checkbox_entry("Z Piezo",1, 10);

	// Set up a callback from the checklist
	checklist->addCallback(tcl_update_callback,this);

	// Set up for old default, which was Topography and standard deviation
	if ((d_dataset->inputGrid->readMode() == READ_DEVICE) ||
            (d_dataset->inputGrid->readMode() == READ_STREAM)) {
            if (Add_channel("Topography","nm",0,1, 90) ) {
		fprintf(stderr,"Point_channel_selector(): Can't get height\n");
	  }
	}
}

Point_channel_selector::~Point_channel_selector (void) {

}


#ifndef USE_VRPN_MICROSCOPE
int	Point_channel_selector::Update_microscope (Microscope * microscope)
#else
int     Point_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
#endif
{
	// If we have changed since the last time we did this, send a
	// message to the microscope requesting a new set of data
	// channels.
    if (change_from_tcl) {
	change_from_tcl = 0;
	if (change_from_microscope) {
	    change_from_microscope = 0;
	    //fprintf(stderr, "Update_microscope Point: ignoring update req.\n");
	    return 0;
	} else {
	  if (microscope->GetNewPointDatasets(checklist) == -1)
		return -1;
	}
    }
    return 0;
}

int	Point_channel_selector::Add_channel(char *name, char *units,
		float offset,float scale, int num_samples)
{
	int	which;

	// Call the base class method to set up the generic channel
	// information.
	// The base class method returns the index of the new channel
	// or -1.  Derived class method returns 0 on success, -1 on failure.
	which = Channel_selector::Add_channel(name, units, offset, scale, num_samples);
	if (which == -1) {
	    fprintf(stderr,"Point_channel_selector::Add_channel(): Failed\n");
	    return -1;
	}

	// See if we can lookup the value for this checkbox.  If not, make one
	if ( (values[which] = myresult->getValueByName(name)) == NULL) {
	  if ((values[which] = myresult->addNewValue(name, units)) == NULL) {
	    fprintf(stderr,"Point_channel_selector::Add_channel(): Can't add %s value\n",name);
	    numchannels--;
	    return -1;
	  }
	}

	// Add this to the list of available data sets for mapping
	// (The add will fail if this entry is already there.  We
	// ignore this.)
	namelist->addEntry(name);

	return 0;
}

int     Point_channel_selector::Handle_old_report( float x, float y,
		long sec, long usec, float value, float std_dev)
{
	// Set location and time in the point results class
	if (myresult) {
		myresult->setPosition(x, y);
		myresult->setTime(sec, usec);
	}

	// Update the height and standard deviation planes.
	// This assumes that they are the first two, which was the case
	// in the old stream files which won't set this up
	if (numchannels>0) {
		values[0]->setValue(
			channels[0].offset + channels[0].scale*value);
	}
	if (numchannels>1) {
		values[1]->setValue( 
			channels[1].offset + channels[1].scale*std_dev);
	}

	return 0;
}

int     Point_channel_selector::Handle_report( float x, float y,
		long sec, long usec, float *vals, int numvalues,
                vrpn_bool accumulatePoints)
{
	int	i;

	if (numvalues != numchannels) {
	    fprintf(stderr,
	     "Point_channel_selector::Handle_report(): wrong # channels\n");
	    fprintf(stderr,"    (got %d, expected %d)\n",numvalues,numchannels);
	    return -1;
	}

	// Set location and time in the point results class
	if (myresult) {
		myresult->setPosition(x, y);
		myresult->setTime(sec, usec);
	}

	// Update each of the values with the new values.
	for (i = 0; i < numchannels; i++) {
		values[i]->setValue(
			channels[i].offset + channels[i].scale*vals[i]);
	}

        // Are we stashing a copy of this data in a point list?
        if (accumulatePoints) {
          d_pointList->addEntry(*myresult);
        }

	return 0;
}

ForceCurve_channel_selector::ForceCurve_channel_selector
    (Point_results *point, nmb_ListOfStrings * namelist,
     nmb_Dataset * dataset) :
	Channel_selector (namelist, dataset)
{
    change_from_tcl = 1;		// Report to the microscope what we want
    numchannels = 0;	// No channels mapped yet
    myresult = point;	// Where to store values

    // Get a new checklist.
    checklist = new Tclvar_checklist_with_entry(".data_sets.forcecurve");
    if (checklist == NULL) {
	fprintf(stderr,"ForceCurve_channel_selector(): Can't make checklist\n");
	return;
    }

    // Put the known types into the checklist.
    // spmlab allows two out of three including the following and
    // the two auxiliary channels, but selection of other than the default
    // hasn't been implemented yet
    checklist->Add_checkbox("Internal Sensor",1);
    if (Add_channel("Internal Sensor", "nA", 0.0, 1.0))
	fprintf(stderr,"ForceCurve_channel_selector(): Can't get h&d\n");
}

ForceCurve_channel_selector::~ForceCurve_channel_selector (void) {

}

#ifndef USE_VRPN_MICROSCOPE
int	ForceCurve_channel_selector::Update_microscope (Microscope * microscope)
#else
int     ForceCurve_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
#endif
{
	// If we have changed since the last time we did this, send a
        // message to the microscope requesting a new set of data
        // channels.
    if (change_from_tcl) {
	change_from_tcl = 0;
	if (change_from_microscope) {
	    change_from_microscope = 0;
	    //fprintf(stderr, "Update_microscope FC: ignoring update req.\n");
	    return 0;
	} else {
	  if (microscope->GetNewPointDatasets(checklist) == -1)
		return -1;
	}
    }
 
    return 0;
}

int	ForceCurve_channel_selector::Add_channel(char *name, char *units,
		float offset,float scale)
{
	int     which;

        // Call the base class method to set up the generic channel
        // information.
        // The base class method returns the index of the new channel
        // or -1.  Derived class method returns 0 on success, -1 on failure.
        which = Channel_selector::Add_channel(name, units, offset, scale);
        if (which == -1) {
            fprintf(stderr,"FC_channel_selector::Add_channel(): Failed\n");
            return -1;
        }

        // See if we can lookup the value for this checkbox.  If not, make one
        if ( (values[which] = myresult->getValueByName(name)) == NULL) {
          if ((values[which] = myresult->addNewValue(name, units)) == NULL) {
            fprintf(stderr,"FC_channel_selector::Add_channel():"
			" Can't add %s value\n",name);
            numchannels--;
            return -1;
          }
        }

        // Add this to the list of available data sets for mapping
        // (The add will fail if this entry is already there.  We
        // ignore this.)
        namelist->addEntry(name);

        return 0;
}

int     ForceCurve_channel_selector::Handle_report( float x, float y, float z,
                long sec, long usec, float *vals, int numvalues)
{
        int     i;

        if (numvalues != numchannels) {
            fprintf(stderr,
             "FC_channel_selector::Handle_report(): wrong # channels\n");
            fprintf(stderr,"    (got %d, expected %d)\n",numvalues,numchannels);
            return -1;
        }

        // Set location and time in the point results class
        if (myresult) {
                myresult->setPosition(x, y, z);
                myresult->setTime(sec, usec);
        }

        // Update each of the values with the new values.
        for (i = 0; i < numchannels; i++) {
                values[i]->setValue(
                        channels[i].offset + channels[i].scale*vals[i]);
        }

        return 0;
}

