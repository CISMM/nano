
#ifdef sgi
#include <unistd.h>  // for gethostname
#endif
#ifdef _WIN32
#include <winsock2.h>  // for gethostname
#endif

#include "nmb_LBLFlattenedPlane.h"


#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif



nmb_LBLFlattenedPlane::
nmb_LBLFlattenedPlane( const char* inputPlaneName,
		       const char* outputPlaneName,
		       nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
  : sourcePlane ( NULL ),
    firstLineAvg ( 0 ),
    nmb_CalculatedPlane( outputPlaneName, dataset )
{
  // Are all the pointer arguments valid?
  if( inputPlaneName == NULL || dataset == NULL 
	   || dataset->inputGrid == NULL )
    {
      char s[] = "Internal error in nmb_LBLFlattenPlane:  invalid argument.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  // Is the destination plane name the same as the source plane name?
  if( strcmp( outputPlaneName, inputPlaneName ) == 0 ) 
    {
      char s[] = "Cannot create flattened plane.  "
	"Plane cannot flatten from itself.";
      throw nmb_CalculatedPlaneCreationException( s );
    }
  
  // try to get the requested source plane...
  this->sourcePlane = dataset->inputGrid->getPlaneByName(inputPlaneName);
  if( this->sourcePlane == NULL )
    {
      char s[] = "Cannot create flattened plane.  "
	"Could not get input plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, inputPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }
  
  // create the lbl-flattened plane
  char newunits[1000];
  sprintf(newunits, "%s_lbl_flat", sourcePlane->units()->Characters());

  createCalculatedPlane( newunits, sourcePlane, dataset );
  fillLBLFlattenedPlane( dataset );

  // register ourselves to receive plane updates
  sourcePlane->add_callback( sourcePlaneChangeCallback, this );

} // end nmb_LBLFlattenedPlane( ... )


void nmb_LBLFlattenedPlane::
fillLBLFlattenedPlane( nmb_Dataset* dataset )
{ 
  //Fill the output plane with the line-by-line flattened plane
  int x = 0, y = 0;
  float avgVal = 0, diff = 0;

  //First, find the average height value of the first 2 scan lines and copy
  //the first scan line (unchanged) to the output plane
  for( x = 0; x <= dataset->inputGrid->numX() - 1; x++ ) 
    {
      calculatedPlane->setValue(x, 0, sourcePlane->value(x, 0));
      this->firstLineAvg += sourcePlane->value(x, 0);
      avgVal += sourcePlane->value(x, 1);
    }
  
  this->firstLineAvg /= (float) dataset->inputGrid->numX();
  avgVal /= (float) dataset->inputGrid->numX();
  diff = this->firstLineAvg - avgVal;

  //Compute average height value of current line, and flatten the previous
  //scan line before writing it (prev. line) to the output plane
  for( y = 1; y <= dataset->inputGrid->numY() - 2; y++ ) 
    {
      avgVal = 0;
      for (x = 0; x <= dataset->inputGrid->numX() - 1; x++) 
	{
	  calculatedPlane->setValue( x, y, 
				sourcePlane->value(x, y - 1) + diff ); 
	  avgVal += sourcePlane->value( x, y + 1 );
	}
      avgVal = avgVal / (float) dataset->inputGrid->numX();
      diff = this->firstLineAvg - avgVal;
  }

  //flatten and output the last scan line
  for( x = 0; x <= dataset->inputGrid->numX() - 1; x++ ) 
    {
      float newVal = sourcePlane->value( x, dataset->inputGrid->numY() - 1) 
	+ diff;
      calculatedPlane->setValue( x, dataset->inputGrid->numY() - 1,
			     newVal );
    }

  calculatedPlane->setMinAttainableValue( sourcePlane->minAttainableValue() );
  calculatedPlane->setMaxAttainableValue( sourcePlane->maxAttainableValue() );

} // end nmb_LBLFlattenedPlane::createLBLFlattenedPlane( ... )



/* static */
void nmb_LBLFlattenedPlane::
sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
			   void* userdata )
{
  nmb_LBLFlattenedPlane * us = (nmb_LBLFlattenedPlane *) userdata;
  if( plane != us->sourcePlane ) 
    {
      cerr << "Internal Error:  nmb_LBLFlattenedPlane::"
	   << "sourcePlaneChangeCallback called with inconsistent "
	   << "nmb_LBLFlattenedPlane and source plane." 
	   << endl;
      return;
    }
  us->_handleSourcePlaneChange( x, y );
  
} // end sourcePlaneChangeCallback( ... )



void nmb_LBLFlattenedPlane::
_handleSourcePlaneChange( int x, int y )
{
  float avgVal = 0;
  float diff = 0;
  int   i;

  // only recalculated at the end of a line.
  if( x != sourcePlane->numX() - 1 ) return;

  for( i = 0; i < sourcePlane->numX(); i++ ) {
      avgVal += sourcePlane->value(i, y);
  }
  avgVal /= sourcePlane->numX();

  // update the current line
  diff = firstLineAvg - avgVal; 
  for( i = 0; i < sourcePlane->numX(); i++ ) {
      calculatedPlane->setValue( i, y, sourcePlane->value( i, y ) + diff );
  }
} // end _handleSourcePlaneChange( ... )



void nmb_LBLFlattenedPlane::
sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
		     vrpn_int32 synchCalculatedPlaneMessageType ) const
{
  char planemsg [1024];
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;

  vrpn_buffer( &bufptr, &msglen, LBL_FLATTENED_PLANE_TYPE );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) calculatedPlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane->name()->Characters(),
	      sourcePlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, calculatedPlane->name()->Characters(),
	      calculatedPlane->name()->Length() );
  
  timeval now;
  gettimeofday(&now, NULL);
  conn->pack_message(1024 - msglen, now, synchCalculatedPlaneMessageType, 
		     senderID, planemsg, vrpn_CONNECTION_RELIABLE);
} // end sendCalculatedPlane( ... )



/* static */
nmb_CalculatedPlane* nmb_LBLFlattenedPlane::
_handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  const char * bufptr = p.buffer;
  vrpn_int32 planeType;

  vrpn_unbuffer( &bufptr, &planeType );
  if( planeType != nmb_CalculatedPlane::LBL_FLATTENED_PLANE_TYPE )
    {
      char s[] = "Could not create flattened plane from remote source.  "
	"Wrong type.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  vrpn_int32 outputPlaneNameLen;
  vrpn_int32 sourcePlaneNameLen;

  vrpn_unbuffer( &bufptr, &sourcePlaneNameLen );
  vrpn_unbuffer( &bufptr, &outputPlaneNameLen );

  char* sourcePlaneName = new char[ sourcePlaneNameLen + 1 ];
  char* outputPlaneName = new char[ outputPlaneNameLen + 1 ];
  vrpn_unbuffer( &bufptr, sourcePlaneName, sourcePlaneNameLen );
  vrpn_unbuffer( &bufptr, outputPlaneName, outputPlaneNameLen );
  sourcePlaneName[sourcePlaneNameLen] = '\0';
  outputPlaneName[outputPlaneNameLen] = '\0';

  nmb_LBLFlattenedPlane* newFlatPlane 
    = new nmb_LBLFlattenedPlane( sourcePlaneName, outputPlaneName,
                                 dataset );
  
  // register new flattened plane to receive plane updates
  newFlatPlane->sourcePlane->add_callback( sourcePlaneChangeCallback, 
					   newFlatPlane );

  delete outputPlaneName;
  delete sourcePlaneName;
  return newFlatPlane;
} // _handle_PlaneSynch( ... )



#ifdef _WIN32
#pragma warning( pop )
#endif
