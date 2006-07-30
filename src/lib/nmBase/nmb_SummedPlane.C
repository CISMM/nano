#include <iostream>

#ifdef sgi
#include <unistd.h>  // for gethostname
#endif
#ifdef _WIN32
#include <winsock2.h>  // for gethostname
#endif

#include "nmb_SummedPlane.h"
#include "BCPlane.h"
#include "nmb_Dataset.h"


#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


nmb_SummedPlane::
nmb_SummedPlane( const char* inputPlaneName1,
		 const char* inputPlaneName2,
		 double scale,
		 const char* outputPlaneName,
		 nmb_Dataset* dataset 
		 // Dataset to which this plane will be added.
		 )
  throw( nmb_CalculatedPlaneCreationException )
  : nmb_CalculatedPlane( outputPlaneName, dataset )
{
  // Are all the pointer arguments valid?
  if( inputPlaneName1 == NULL || inputPlaneName2 == NULL
      || dataset == NULL || dataset->inputGrid == NULL )
    {
      char s[] = "Internal error in nmb_SummedPlane(...):  invalid argument.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  // Is the a destination plane name the same as the source plane name?
  if( strcmp( outputPlaneName, inputPlaneName1 ) == 0 
      || strcmp( outputPlaneName, inputPlaneName2 ) == 0) 
    {
      char s[] = "Cannot create summed plane.  "
	"Plane cannot sum from itself.";
      throw nmb_CalculatedPlaneCreationException( s );
    }
  
  this->scale = scale;

  // try to get the requested source planes...
  this->sourcePlane1 = dataset->inputGrid->getPlaneByName(inputPlaneName1);
  this->sourcePlane2 = dataset->inputGrid->getPlaneByName(inputPlaneName2);
  if( this->sourcePlane1 == NULL || this->sourcePlane2 == NULL )
    {
      char s[] = "Cannot create flattened plane.  "
	"Could not get input plane:  ";
      char msg[1024];
      if( sourcePlane1 == NULL )
	sprintf( msg, "%s%s.", s, inputPlaneName1 );
      else if( sourcePlane2 == NULL )
	sprintf( msg, "%s%s.", s, inputPlaneName2 );
      else // how did this happen?
	sprintf( msg, "%s.", s );
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // Ensure that the units match on the input planes.
  // Actually, only warn if they are not the same
  if ( sourcePlane1->units()->compare(*sourcePlane2->units()) != 0) 
    {
      fprintf(stderr, "nmb_SummedPlane(...): Unit mismatch\n");
      fprintf(stderr, "    (%s vs. %s)\n",
	      sourcePlane1->units()->c_str(),
	      sourcePlane2->units()->c_str());
    }
  
  //////////////
  // create output plane 

  // Use the units for the first plane.  
  // It is assumed that the second will have been scaled appropriately.
  char newunits[1000];
  sprintf(newunits, "%s_flat", sourcePlane1->units()->c_str());
  createCalculatedPlane( newunits, sourcePlane1, dataset );
  
  // get the valid data region
  short xmin1 = 0, ymin1 = 0, xmax1 = 0, ymax1 = 0;
  sourcePlane1->findValidDataRange( &xmin1, &xmax1, &ymin1, &ymax1 );
  short xmin2 = 0, ymin2 = 0, xmax2 = 0, ymax2 = 0;
  sourcePlane2->findValidDataRange( &xmin2, &xmax2, &ymin2, &ymax2 );

  short xmin = MIN( xmin1, xmin2 ), ymin = MIN( ymin1, ymin2 );
  short xmax = MAX( xmax1, xmax2 ), ymax = MAX( ymax1, ymax2 );


  // fill in the new plane.
  for(int x = 0; x <= dataset->inputGrid->numX() - 1; x++) 
  {
    for( int y = 0; y <= dataset->inputGrid->numY() - 1; y++) 
    {
      if( x >= xmin && x <= xmax && y >= ymin && y <= ymax )
        calculatedPlane->setValue( x, y,
                                   (float) ( sourcePlane1->value(x, y) 
					     + scale 
                                             * sourcePlane2->value(x, y) ) );
      else
        calculatedPlane->setValue( x, y, 0 );
    }
  }
  
  // register ourselves to receive plane updates
  sourcePlane1->add_callback( sourcePlaneChangeCallback, this );
  sourcePlane2->add_callback( sourcePlaneChangeCallback, this );

} // end nmb_SummedPlane( ... )


nmb_SummedPlane::
~nmb_SummedPlane( )
{
  // unregister ourselves to receive plane updates
  if( sourcePlane1 != NULL )
    sourcePlane1->remove_callback( sourcePlaneChangeCallback, this );
  if( sourcePlane2 != NULL )
    sourcePlane2->remove_callback( sourcePlaneChangeCallback, this );
} // end ~nmb_FlattenedPlane


bool nmb_SummedPlane::
dependsOnPlane( const BCPlane* const plane )
{
  if( plane == NULL ) return false;
  if( plane == sourcePlane1 /* pointer comparison */ )
    return true;
  else if( plane == sourcePlane2 )
    return true;
  else
    return false;
}


bool nmb_SummedPlane::
dependsOnPlane( const char* planeName )
{
  if( planeName == NULL ) return false;
  if(this->sourcePlane1->name()->compare(planeName) == 0 )
    return true;
  else if( this->sourcePlane2->name()->compare(planeName) == 0 )
    return true;
  else
    return false;
}


/* static */
void nmb_SummedPlane::
sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
			   void* userdata )
{
  nmb_SummedPlane* us = (nmb_SummedPlane*) userdata;
  if( plane != us->sourcePlane1 && plane != us->sourcePlane2 ) 
    {
      cerr << "Internal Error:  nmb_SummedPlane::sourcePlaneChangeCallback "
	   << "called with inconsistent nmb_SummedPlane and source plane." 
	   << endl;
      return;
    }
  us->_handleSourcePlaneChange( x, y );
} // end sourcePlaneChangeCallback



void nmb_SummedPlane::
_handleSourcePlaneChange( int x, int y )
{
  calculatedPlane->setValue( x, y, 
                             (float) ( sourcePlane1->value(x, y) 
                                       + scale * sourcePlane2->value(x, y) ) );
} // end _handleSourcePlaneChange



void nmb_SummedPlane::
sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
		     vrpn_int32 synchCalculatedPlaneMessageType ) const
{
  char planemsg [1024];
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;

  vrpn_buffer( &bufptr, &msglen, SUMMED_PLANE_TYPE );
  vrpn_buffer( &bufptr, &msglen, scale );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane1->name()->length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane2->name()->length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) calculatedPlane->name()->length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane1->name()->c_str(),
	       sourcePlane1->name()->length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane2->name()->c_str(),
	       sourcePlane2->name()->length() );
  vrpn_buffer( &bufptr, &msglen, calculatedPlane->name()->c_str(),
	       calculatedPlane->name()->length() );
  
  timeval now;
  gettimeofday(&now, NULL);
  conn->pack_message(1024 - msglen, now, synchCalculatedPlaneMessageType, 
		     senderID, planemsg, vrpn_CONNECTION_RELIABLE);
} // end sendCalculatedPlane( ... )



/* static */
nmb_CalculatedPlane* nmb_SummedPlane::
_handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  const char * bufptr = p.buffer;
  vrpn_int32 planeType;

  vrpn_unbuffer( &bufptr, &planeType );
  if( planeType != nmb_CalculatedPlane::SUMMED_PLANE_TYPE )
    {
      char s[] = "Could not create flattened plane from remote source.  "
	"Wrong type.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  vrpn_float64 scale;
  vrpn_int32 sourcePlaneNameLen1;
  vrpn_int32 sourcePlaneNameLen2;
  vrpn_int32 outputPlaneNameLen;

  vrpn_unbuffer( &bufptr, &scale );
  vrpn_unbuffer( &bufptr, &sourcePlaneNameLen1 );
  vrpn_unbuffer( &bufptr, &sourcePlaneNameLen2 );
  vrpn_unbuffer( &bufptr, &outputPlaneNameLen );

  char* sourcePlaneName1 = new char[ sourcePlaneNameLen1 + 1 ];
  char* sourcePlaneName2 = new char[ sourcePlaneNameLen2 + 1 ];
  char* outputPlaneName = new char[ outputPlaneNameLen + 1 ];
  vrpn_unbuffer( &bufptr, sourcePlaneName1, sourcePlaneNameLen1 );
  vrpn_unbuffer( &bufptr, sourcePlaneName2, sourcePlaneNameLen2 );
  vrpn_unbuffer( &bufptr, outputPlaneName, outputPlaneNameLen );
  sourcePlaneName1[sourcePlaneNameLen1] = '\0';
  sourcePlaneName2[sourcePlaneNameLen2] = '\0';
  outputPlaneName[outputPlaneNameLen] = '\0';

  // test for idempotency
  nmb_SummedPlane* samePlane 
    = dynamic_cast<nmb_SummedPlane*>( nmb_CalculatedPlane::getCalculatedPlane( outputPlaneName ) );
  // samePlane will be NULL EITHER if there is currently no plane of the given name,
  // OR if the run-time identified type of the plane with the given name is not 
  // nmb_SummedPlane
  if( samePlane != NULL )
  { // see if we got a message to recreate the same plane
    if( scale == samePlane->scale
        && samePlane->sourcePlane1->name()->compare(sourcePlaneName1) == 0 
        && samePlane->sourcePlane2->name()->compare(sourcePlaneName2) == 0 )
    {
      // the requested plane is exactly the same as one that already exists,
      // so don't change anything
      delete outputPlaneName;
      delete sourcePlaneName1;
      delete sourcePlaneName2;
      return samePlane;
    }
  }


  nmb_SummedPlane* newSummedPlane 
    = new nmb_SummedPlane( sourcePlaneName1, sourcePlaneName2, scale, 
                           outputPlaneName, dataset );

  // register new flattened plane to receive plane updates
  newSummedPlane->sourcePlane1->add_callback( sourcePlaneChangeCallback, 
					     newSummedPlane );
  newSummedPlane->sourcePlane2->add_callback( sourcePlaneChangeCallback, 
					     newSummedPlane );

  delete sourcePlaneName1;
  delete sourcePlaneName2;
  delete outputPlaneName;
  return newSummedPlane;
} // _handle_PlaneSynch( ... )

#ifdef _WIN32
#pragma warning( pop )
#endif

