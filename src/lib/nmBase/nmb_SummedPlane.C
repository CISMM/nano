
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
{
  // Are all the pointer arguments valid?
  if( inputPlaneName1 == NULL || inputPlaneName2 == NULL
      || outputPlaneName == NULL
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
  if ( strcmp(sourcePlane1->units()->Characters(),
	      sourcePlane2->units()->Characters()) != 0) 
    {
      fprintf(stderr, "nmb_SummedPlane(...): Unit mismatch\n");
      fprintf(stderr, "    (%s vs. %s)\n",
	      sourcePlane1->units()->Characters(),
	      sourcePlane2->units()->Characters());
    }
  
  // Add the host name to the plane name so we can distinguish
  // where the plane came from
  char newOutputPlaneName[256];
#if 1
  char hostname[256];
  gethostname( hostname, 256 );
  sprintf( newOutputPlaneName, "%s from %s", outputPlaneName, hostname );
#else
  // XXX 3rdTech only - no weird plane names.
  sprintf(newOutputPlaneName, "%s", outputPlaneName);
#endif
  
  // create output plane 
  this->summedPlane = createSummedPlane( dataset, newOutputPlaneName );

  // add ourselves to the dataset
  dataset->addNewCalculatedPlane( this );
  
  // register ourselves to receive plane updates
  sourcePlane1->add_callback( sourcePlaneChangeCallback, this );
  sourcePlane2->add_callback( sourcePlaneChangeCallback, this );

  // let interested parties know a new plane has been created.
  nmb_CalculatedPlane::addNewCalculatedPlane( this );

} // end nmb_SummedPlane( ... )



BCPlane* nmb_SummedPlane::
createSummedPlane( nmb_Dataset* dataset,
		   const char* outputPlaneName )
  throw( nmb_CalculatedPlaneCreationException )
{
  // Precondition:  there does not exist a plane of name 
  //  outputPlaneName in dataset.  Also, sourcePlane1 and 2
  //  are set to valid planes.
  BCPlane* outputPlane = dataset->inputGrid->getPlaneByName( outputPlaneName );

  if( outputPlane != NULL )
    {
      // a plane already exists by this name, and we disallow that.
      char s[] = "Cannot create summed plane.  "
	"A plane already exists of the name:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, outputPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // plane of name "outputPlaneName" does not exist already.
  // Use the units for the first plane.  
  // It is assumed that the second will have been scaled appropriately.
  char newunits[1000];
  sprintf(newunits, "%s_flat", sourcePlane1->units()->Characters());
  outputPlane 
    = dataset->inputGrid->addNewPlane(outputPlaneName, newunits, NOT_TIMED);
  if( outputPlane == NULL ) 
    {
      char s[] = "Could not create summed plane.  Can't make plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, outputPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }
  
  TopoFile tf;
  nmb_Image *im = dataset->dataImages->getImageByPlane( sourcePlane1 );
  nmb_Image *output_im = new nmb_ImageGrid( outputPlane );
  if( im != NULL ) 
    {
      im->getTopoFileInfo(tf);
      output_im->setTopoFileInfo(tf);
    } 
  else 
    {
      fprintf(stderr, "nmb_SummedPlane: Warning, input image not in list\n");
    }
  dataset->dataImages->addImage(output_im);
  
  // fill in the new plane.
  for(int x = 0; x <= dataset->inputGrid->numX() - 1; x++) 
    {
      for( int y = 0; y <= dataset->inputGrid->numY() - 1; y++) 
	{
	  outputPlane->setValue( x, y,
				 (float) ( sourcePlane1->value(x, y) 
					   + scale 
					   * sourcePlane2->value(x, y) ) );
	}
    }
  return outputPlane;
} // end nmb_SummedPlane::createSummedPlane( ... )



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
  this->summedPlane->setValue( x, y, 
			       (float) ( sourcePlane1->value(x, y) 
					 + scale
					 * sourcePlane2->value(x, y) ) );
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
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane1->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane2->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) summedPlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane1->name()->Characters(),
	       sourcePlane1->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane2->name()->Characters(),
	       sourcePlane2->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, summedPlane->name()->Characters(),
	       summedPlane->name()->Length() );
  
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

  nmb_SummedPlane* newSummedPlane = new nmb_SummedPlane;
  newSummedPlane->scale = scale;
  
  // Is the destination plane name the same as the source plane name?
  if( strcmp( outputPlaneName, sourcePlaneName2 ) == 0 
      || strcmp( outputPlaneName, sourcePlaneName2 ) == 0  ) 
    {
      char s[] = "Cannot create summed plane.  "
	"Plane cannot sum from itself.";
      delete sourcePlaneName1;
      delete sourcePlaneName2;
      delete outputPlaneName;
      throw nmb_CalculatedPlaneCreationException( s );
    }
  
  // try to get the requested source planes...
  newSummedPlane->sourcePlane1 
    = dataset->inputGrid->getPlaneByName(sourcePlaneName1);
  newSummedPlane->sourcePlane2 
    = dataset->inputGrid->getPlaneByName(sourcePlaneName2);
  if( newSummedPlane->sourcePlane1 == NULL 
      || newSummedPlane->sourcePlane2 == NULL )
    {
      char s[] = "Cannot create flattened plane from remote. " \
	" Could not get input plane:  ";
      char msg[1024];
      if( newSummedPlane->sourcePlane1 == NULL )
	sprintf( msg, "%s%s.", s, sourcePlaneName1 );
      else if( newSummedPlane->sourcePlane2 == NULL )
	sprintf( msg, "%s%s.", s, sourcePlaneName2 );
      else // how did this happen?
	sprintf( msg, "%s.", s );
      delete sourcePlaneName1;
      delete sourcePlaneName2;
      delete outputPlaneName;
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // create output plane
  newSummedPlane->summedPlane 
    = newSummedPlane->createSummedPlane( dataset, outputPlaneName );

  // add new flattened plane to the dataset
  dataset->addNewCalculatedPlane( newSummedPlane );
  
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

