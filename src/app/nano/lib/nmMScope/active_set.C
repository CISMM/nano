/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	<Tcl_Linkvar.h>
#include	<Tcl_Netvar.h>
#include	"active_set.h"
#include	<BCPlane.h>
#include	<nmb_Globals.h>

#include "nmm_MicroscopeRemote.h"

/** Merely sets a flag, so the next time UpdateMicroscope gets called
 new datasets are requested.
*/
void tcl_update_callback( int /*val*/, void *udata)
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
    namelist (namelist),
    d_dataset (dataset) 
{
    for (int i = 0; i < MAX_CHANNELS; i++ ) {
        active_list[i]=NULL;
        numsamples_list[i]=NULL;
    }
}

// Moved here because virtual functions can't be inlined but
// declaration in the header implies inlining request for
// some compilers - TCH 1 Oct 97

Channel_selector::~Channel_selector (void) {
    for (int i = 0; i < MAX_CHANNELS; i++ ) {
        if (active_list[i]!=NULL) {
            delete active_list[i];
        }
        if ( numsamples_list[i]!=NULL) {
            delete  numsamples_list[i];
        }
    }  
}

int	Channel_selector::Set(const char *channel)
{
    int index;
    // Set the channel in the lists.  
    if ((index = channel_list.getIndex(channel)) == -1) {
        return -1;
    } else if (active_list[index] == NULL){
        return -1;
    }
    *active_list[index] = 1;
    return 0;
}

int	Channel_selector::Unset(const char *channel)
{
    int index;
    // UnSet the channel in the lists.  
    if ((index = channel_list.getIndex(channel)) == -1) {
        return -1;
    } else if (active_list[index] == NULL){
        return -1;
    }
    *active_list[index] = 0;
    return 0;
}

int	Channel_selector::Is_set(const char *channel)
{
    int index;
    // Set the channel in the lists.  
    if ((index = channel_list.getIndex(channel)) == -1) {
        return 0;
    } else if (active_list[index] == NULL){
        return 0;
    }
    return (*active_list[index] == 1);
}

// virtual
int Channel_selector::Update_microscope (nmm_Microscope_Remote *) {
  return 0;
}



int	Channel_selector::Clear_channels(void)
{
	int	i;

	// Mark this a being a change from the microscope.
	change_from_microscope = 1;

	// Clear all of the channels that are currently getting data
	numchannels = 0;

	// Clear all of the checkboxes
  	for (i = 0; i < channel_list.numEntries(); i++) {
            if (active_list[i] != NULL) {
                *active_list[i] = 0;
            }
  	}
	return 0;
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
        // NOTE: This probably never happens - we have a complete list
        // of all channels that come from the ThermoMicroscopes SPMs. 
        int index;
        if ((index = channel_list.getIndex(name)) == -1) {
            channel_list.addEntry(name);
            if ((index = channel_list.getIndex(name)) == -1) {
                fprintf(stderr,"Channel_selector::Add_channel(): Can't add name\n");
                return -1;
            }
            // The derived classes had better set these, so they aren't NULL.
            //active_list[index] = new Tclvar_int("dataset(stuff)",1);
            //numsamples_list[index] = new Tclvar_int("dataset(stuff_samples)",10);
  	} 
        if (active_list[index] == NULL){
            // Derived class should take care of this
            //fprintf(stderr,"Channel_selector::Add_channel(): no active tclvar to set\n");
        } else {
            *active_list[index] = 1;
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
        // NOTE: This probably never happens - we have a complete list
        // of all channels that come from the ThermoMicroscopes SPMs. 
        int index;
        if ((index = channel_list.getIndex(name)) == -1) {
            channel_list.addEntry(name);
            if ((index = channel_list.getIndex(name)) == -1) {
                fprintf(stderr,"Channel_selector::Add_channel(): Can't add name\n");
                return -1;
            }
            // The derived classes had better set these, so they aren't NULL.
            //active_list[index] = new Tclvar_int("dataset(stuff)",1);
            //numsamples_list[index] = new Tclvar_int("dataset(stuff_samples)",10);
  	} 
        if (active_list[index] == NULL){
            // Derived class should take care of this
            //fprintf(stderr,"Channel_selector::Add_channel(): no active tclvar to set\n");
        } else {
            *active_list[index] = 1;
        }
        if (numsamples_list[index] == NULL){
            // Derived class should take care of this
            //fprintf(stderr,"Channel_selector::Add_channel(): no numsamples tclvar to set\n");
        } else {
            *numsamples_list[index] = num_samples;
        }

	// Fill in the rest and increment the count
	strncpy(channels[numchannels].units,units,
		sizeof(channels[numchannels].units));
	channels[numchannels].offset = offset;
	channels[numchannels].scale = scale;
	numchannels++;

	return numchannels-1;		// Tell which channel
}

Scan_channel_selector::Scan_channel_selector(BCGrid *grid_to_track,
    nmb_ListOfStrings * namelist, nmb_Dataset * dataset) :
		Channel_selector (namelist, dataset)
{
	change_from_tcl = 0;		
	change_from_microscope = 1;   // Get current datasets from MScope

	numchannels = 0;	// No channels mapped yet
	mygrid = grid_to_track;

 	// Put the known types into the list, but don't require planes for
 	// them.
        // The names and the tcl variable names are "magic" - i.e. they must
        // correspond exactly to the widgets we create for them. 
        // As of 6/00, those are created from a list in setup_menu.tcl
  	channel_list.addEntry("Topography-Forward");
        active_list[0] = new Tclvar_int("data_sets(scan0)",1,tcl_update_callback,this);
  	channel_list.addEntry("Topography-Reverse");
        active_list[1] = new Tclvar_int("data_sets(scan1)",0,tcl_update_callback,this);
  	channel_list.addEntry("Internal Sensor-Forward");
        active_list[2] = new Tclvar_int("data_sets(scan2)",0,tcl_update_callback,this);
  	channel_list.addEntry("Internal Sensor-Reverse");
        active_list[3] = new Tclvar_int("data_sets(scan3)",0,tcl_update_callback,this);
  	channel_list.addEntry("Z Modulation-Forward");
        active_list[4] = new Tclvar_int("data_sets(scan4)",0,tcl_update_callback,this);
  	channel_list.addEntry("Z Modulation-Reverse");
        active_list[5] = new Tclvar_int("data_sets(scan5)",0,tcl_update_callback,this);
  	channel_list.addEntry("Lateral Force-Forward");
        active_list[6] = new Tclvar_int("data_sets(scan6)",0,tcl_update_callback,this);
  	channel_list.addEntry("Lateral Force-Reverse");
        active_list[7] = new Tclvar_int("data_sets(scan7)",0,tcl_update_callback,this);
  	channel_list.addEntry("IN 1-Forward");
        active_list[8] = new Tclvar_int("data_sets(scan8)",0,tcl_update_callback,this);
  	channel_list.addEntry("IN 1-Reverse");
        active_list[9] = new Tclvar_int("data_sets(scan9)",0,tcl_update_callback,this);
  	channel_list.addEntry("IN 2-Forward");
        active_list[10] = new Tclvar_int("data_sets(scan10)",0,tcl_update_callback,this);
  	channel_list.addEntry("IN 2-Reverse");
        active_list[11] = new Tclvar_int("data_sets(scan11)",0,tcl_update_callback,this);
  	channel_list.addEntry("Phase-Forward");
        active_list[12] = new Tclvar_int("data_sets(scan12)",0,tcl_update_callback,this);
  	channel_list.addEntry("Phase-Reverse");
        active_list[13] = new Tclvar_int("data_sets(scan13)",0,tcl_update_callback,this);
  	channel_list.addEntry("FastTrack-Forward");
        active_list[14] = new Tclvar_int("data_sets(scan14)",0,tcl_update_callback,this);
  	channel_list.addEntry("FastTrack-Reverse");
        active_list[15] = new Tclvar_int("data_sets(scan15)",0,tcl_update_callback,this);
  	channel_list.addEntry("Z Piezo-Forward");
        active_list[16] = new Tclvar_int("data_sets(scan16)",0,tcl_update_callback,this);
  	channel_list.addEntry("Z Piezo-Reverse");
        active_list[17] = new Tclvar_int("data_sets(scan17)",0,tcl_update_callback,this);

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

int     Scan_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
{
	// If we have changed since the last time we did this, send a
	// message to the microscope requesting a new set of data
	// channels.
    if (change_from_tcl) {
	change_from_tcl = 0;
  	if (change_from_microscope) {
  	    change_from_microscope = 0;
  	    fprintf(stderr, "Update_microscope Scan: ignoring update req.\n");
  	    return 0;
  	} else {
	  if (microscope->GetNewScanDatasets(&channel_list, active_list) == -1)
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
        // If this was a previously unknown channel, add the active tclvar for it.
        int full_list_index = channel_list.getIndex(name);
        if (active_list[full_list_index] == NULL) {
            char varname[100];
            sprintf(varname, "dataset(scan%d)", full_list_index);
            active_list[full_list_index] = new Tclvar_int(varname,1,tcl_update_callback,this);
        }
        *active_list[full_list_index] = 1;

	// See if we can lookup the plane for this checkbox.  If not, make one
	if ( (planes[which] = mygrid->getPlaneByName(name)) == NULL) {
	  if ((planes[which] = mygrid->addNewPlane(name,
	       units, TIMED)) == NULL) {
	    fprintf(stderr,"Scan_channel_selector::Add_channel(): Can't add %s plane\n",name);
	    numchannels--;
	    return -1;
	  }
	}
        // Save the scale and offset used to acquire the data with the plane.
        // Useful if we save the data to a Topo file
        planes[which]->tm_scale = scale;
        planes[which]->tm_offset = offset;

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
	change_from_tcl = 0;		
	change_from_microscope = 1;   // Get current datasets from MScope
	//change_from_tcl = 1;		// Report to the microscope what we want
	numchannels = 0;	// No channels mapped yet
	myresult = point;	// Where to store values

//  	// Put the known types into the list.
        // The names and the tcl variable names are "magic" - i.e. they must
        // correspond exactly to the widgets we create for them. 
        // As of 6/00, those are created from a list in setup_menu.tcl
 	channel_list.addEntry("Topography");
        active_list[0] = new Tclvar_int("data_sets(touch0)",1,tcl_update_callback,this);
        numsamples_list[0] = new Tclvar_int("data_sets(touch_samples0)",90,tcl_update_callback,this);
 	channel_list.addEntry("Internal Sensor");
        active_list[1] = new Tclvar_int("data_sets(touch1)",1,tcl_update_callback,this);
        numsamples_list[1] = new Tclvar_int("data_sets(touch_samples1)",10,tcl_update_callback,this);
 	channel_list.addEntry("Z Modulation");
        active_list[2] = new Tclvar_int("data_sets(touch2)",0,tcl_update_callback,this);
        numsamples_list[2] = new Tclvar_int("data_sets(touch_samples2)",10,tcl_update_callback,this);
 	channel_list.addEntry("Lateral Force");
        active_list[3] = new Tclvar_int("data_sets(touch3)",1,tcl_update_callback,this);
        numsamples_list[3] = new Tclvar_int("data_sets(touch_samples3)",10,tcl_update_callback,this);
 	channel_list.addEntry("IN 1");
        active_list[4] = new Tclvar_int("data_sets(touch4)",0,tcl_update_callback,this);
        numsamples_list[4] = new Tclvar_int("data_sets(touch_samples4)",10,tcl_update_callback,this);
 	channel_list.addEntry("IN 2");
        active_list[5] = new Tclvar_int("data_sets(touch5)",0,tcl_update_callback,this);
        numsamples_list[5] = new Tclvar_int("data_sets(touch_samples5)",10,tcl_update_callback,this);
 	channel_list.addEntry("FastTrack");
        active_list[6] = new Tclvar_int("data_sets(touch6)",0,tcl_update_callback,this);
        numsamples_list[6] = new Tclvar_int("data_sets(touch_samples6)",10,tcl_update_callback,this);
 	channel_list.addEntry("Z Piezo");
        active_list[7] = new Tclvar_int("data_sets(touch7)",1,tcl_update_callback,this);
        numsamples_list[7] = new Tclvar_int("data_sets(touch_samples7)",10,tcl_update_callback,this);

	// Set up for default, which is Topography 
        // Should be taken care of with first call to Update_Microscope, or message from Thermo...
//  	if ((d_dataset->inputGrid->readMode() == READ_DEVICE) ||
//              (d_dataset->inputGrid->readMode() == READ_STREAM)) {
//              if (Add_channel("Topography","nm",0,1, 90) ) {
//  		fprintf(stderr,"Point_channel_selector(): Can't get height\n");
//  	  }
//  	}
}

Point_channel_selector::~Point_channel_selector (void) {

}


int     Point_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
{
	// If we have changed since the last time we did this, send a
	// message to the microscope requesting a new set of data
	// channels.
    if (change_from_tcl) {
	change_from_tcl = 0;
  	if (change_from_microscope) {
  	    change_from_microscope = 0;
  	    fprintf(stderr, "Update_microscope Point: ignoring update req.\n");
  	    return 0;
  	} else {
  	  if (microscope->GetNewPointDatasets(&channel_list, active_list, numsamples_list) == -1)
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
        // If this was a previously unknown channel, add the active tclvar for it.
        int full_list_index = channel_list.getIndex(name);
        char varname[100];
        if (active_list[full_list_index] == NULL) {
            sprintf(varname, "dataset(touch%d)", full_list_index);
            active_list[full_list_index] = new Tclvar_int(varname,1,tcl_update_callback,this);
        }
        *active_list[full_list_index] = 1;
        if (numsamples_list[full_list_index] == NULL) {
            sprintf(varname, "dataset(touch_samples%d)", full_list_index);
            numsamples_list[full_list_index] = new Tclvar_int(varname,10,tcl_update_callback,this);
        }
        *numsamples_list[full_list_index] = num_samples;

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
        if (accumulatePoints && (d_pointList != NULL)) {
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

    // Put the known types into the checklist.
    // spmlab allows two out of three including the following and
    // the two auxiliary channels, but selection of other than the default
    // hasn't been implemented yet
    channel_list.addEntry("Internal Sensor");
    active_list[0] = new Tclvar_int("data_sets(forcecurve0)",1,tcl_update_callback,this);
    // numsamples isn't really used by the force curve (maybe), but it's required because
    // forcecurve uses the pointresults data type to query the SPM. 
    numsamples_list[0] = new Tclvar_int("data_sets(forcecurve_samples0)",10,tcl_update_callback,this);
    if (Add_channel("Internal Sensor", "nA", 0.0, 1.0))
	fprintf(stderr,"ForceCurve_channel_selector(): Can't get internal sensor\n");
}

ForceCurve_channel_selector::~ForceCurve_channel_selector (void) {

}

int     ForceCurve_channel_selector::Update_microscope (nmm_Microscope_Remote * microscope)
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
  	  if (microscope->GetNewPointDatasets(&channel_list, active_list, numsamples_list) == -1)
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
        // If this was a previously unknown channel, add the active tclvar for it.
        int full_list_index = channel_list.getIndex(name);
        char varname[100];
        if (active_list[full_list_index] == NULL) {
            sprintf(varname, "dataset(forcecurve%d)", full_list_index);
            active_list[full_list_index] = new Tclvar_int(varname,1,tcl_update_callback,this);
        }
        *active_list[full_list_index] = 1;
        if (numsamples_list[full_list_index] == NULL) {
            sprintf(varname, "dataset(forcecurve_samples%d)", full_list_index);
            numsamples_list[full_list_index] = new Tclvar_int(varname,10,tcl_update_callback,this);
        }
        *numsamples_list[full_list_index] = 10;

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

