
#include "nmb_FlattenedPlane.h"
#include "BCPlane.h"
#include "nmb_Dataset.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


nmb_FlattenedPlane::
nmb_FlattenedPlane( const char* inputPlaneName,
		    const char* outputPlaneName,
		    float redX, float greenX, float blueX,
		    float redY, float greenY, float blueY,
		    nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  // Are all the pointer arguments valid?
  if( inputPlaneName == NULL || outputPlaneName == NULL
      || dataset == NULL || dataset->inputGrid == NULL )
    {
      char s[] = "Internal error in nmb_FlattenPlane:  invalid argument.";
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

  // Calculate dx, dy and offset.
  calculateDxDyOffset( dataset, redX, greenX, blueX, redY, greenY, blueY );
    
  // Add the host name to the plane name so we can distinguish
  // where the plane came from
  char newOutputPlaneName[256];
#if 1
  // XXX change this to use gethostname(); remove d_hostname from nmb_Dataset
  if( dataset->getHostname( ) ) {
    sprintf(newOutputPlaneName, "%s from %s", outputPlaneName, 
	    dataset->getHostname( ) );
  } else {
    sprintf(newOutputPlaneName, "%s from noHostname", outputPlaneName);
  }
#else
  // XXX 3rdTech only - no weird plane names.
  sprintf(newOutputPlaneName, "%s", outputPlaneName);
#endif
  
  // create output plane
  this->flatPlane = createFlattenedPlane( dataset, newOutputPlaneName );

  // add ourselves to the dataset
  dataset->addNewCalculatedPlane( this );
  dataset->inputPlaneNames->addEntry( newOutputPlaneName );

  // register ourselves to receive plane updates
  sourcePlane->add_callback( sourcePlaneChangeCallback, this );

  // let interested parties know a new plane has been created.
  nmb_CalculatedPlane::addNewCalculatedPlane( this );

} // end nmb_FlattenedPlane( ... )



void nmb_FlattenedPlane::
calculateDxDyOffset( nmb_Dataset* dataset,
		     float redX, float greenX, float blueX,
		     float redY, float greenY, float blueY )
  throw( nmb_CalculatedPlaneCreationException )
{
  // solve for dx, dy in:
  //  z3 - z1 = dx * ( x3 - x1 ) + dy * ( y3 - y1 )
  //  z2 - z1 = dx * ( x2 - x1 ) + dy * ( y2 - y1 )
  // as so:
  //  dy = (z2 - z1 + (z1 - z3) * k) / (y2 - y1 + (y1 - y3) * k)
  //  dx = (z3 - z1 - dy * (y3 - y1)) / (x3 - x1)
  //  where k = ( x2 - x1 ) / ( x3 - x1 )
  double x1, y1, z1;
  double x2, y2, z2;
  double x3, y3, z3;

  x1 = sourcePlane->xInGrid(redX);    y1 = sourcePlane->yInGrid(redY);
  x2 = sourcePlane->xInGrid(greenX);  y2 = sourcePlane->yInGrid(greenY);
  x3 = sourcePlane->xInGrid(blueX);   y3 = sourcePlane->yInGrid(blueY);
  
  // Are the three points co-linear?
  if( (y2-y1) * (x3-x1) + (y1-y3) * (x2-x1) == 0 ) 
    {
      char s[] = "Could not create flattened plane.  Points are collinear.";
      throw nmb_CalculatedPlaneCreationException( s );
    }
 
  // Are pt1 and pt3 co-linear in x?
  // If so, swap pt 2 and pt 3, or division will go bad.
  if( x3 == x1 ) 
    {
      double temp = x3;  x3 = x2;  x2 = temp;
      temp = y3;  y3 = y2;  y2 = temp;
    }

  // have points been requested that are outside the plane?
  if( sourcePlane->valueAt(&z1, redX, redY) == -1 ||
      sourcePlane->valueAt(&z2, greenX, greenY) == -1 ||
      sourcePlane->valueAt(&z3, blueX, blueY) == -1 )
    {
      char s[] = "Could not create flattened plane.  "
	"Points are out of bounds.";
      throw nmb_CalculatedPlaneCreationException( s );
    }
  
  double k = ( x2 - x1 ) / ( x3 - x1 );
  this->dy = (z2 - z1 + (z1 - z3) * k) / (y2 - y1 + (y1 - y3) * k);
  this->dx = (z3 - z1 - this->dy * (y3 - y1)) / (x3 - x1);
  this->offset = dx * dataset->inputGrid->numX() / 2 
    + dy * dataset->inputGrid->numY() / 2;

} // end nmb_FlattenedPlane::calculateDxDyOffset( ... )



BCPlane* nmb_FlattenedPlane::
createFlattenedPlane( nmb_Dataset* dataset,
		      const char* outputPlaneName )
    throw( nmb_CalculatedPlaneCreationException )
{
  // Precondition:  there does not exist a plane of name 
  //  outputPlaneName in dataset
  BCPlane* outputPlane = dataset->inputGrid->getPlaneByName( outputPlaneName );

  if( outputPlane != NULL )
    {
      // a plane already exists by this name, and we disallow that.
      char s[] = "Cannot create flattened plane.  A plane already exists of the name:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, outputPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // plane of name "outputPlaneName" does not exist already
  char newunits[1000];
  sprintf(newunits, "%s_flat", sourcePlane->units()->Characters());

  outputPlane 
    = dataset->inputGrid->addNewPlane(outputPlaneName, newunits, NOT_TIMED);
  if( outputPlane == NULL ) 
    {
      char s[] = "Could not create flattened plane.  Can't make plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, outputPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }
  
  TopoFile tf;
  nmb_Image *im = dataset->dataImages->getImageByPlane( sourcePlane );
  nmb_Image *output_im = new nmb_ImageGrid( outputPlane );
  if( im != NULL ) 
    {
      im->getTopoFileInfo(tf);
      output_im->setTopoFileInfo(tf);
    } 
  else 
    {
      fprintf(stderr, "nmb_FlattenedPlane: Warning, "
	      "input image not in list\n");
    }
  dataset->dataImages->addImage(output_im);
  
  // fill in the new plane.
  for(int x = 0; x <= dataset->inputGrid->numX() - 1; x++) 
    {
      for( int y = 0; y <= dataset->inputGrid->numY() - 1; y++) 
	{
	  outputPlane->setValue(x, y,
				(float) ( sourcePlane->value(x, y) 
					  + offset - dx * x - dy * y ) );
	}
    }

  return outputPlane;
} // end nmb_FlattenedPlane::createFlattenedPlane( ... )




/* static */
void nmb_FlattenedPlane::
sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
			   void* userdata )
{
  nmb_FlattenedPlane* us = (nmb_FlattenedPlane*) userdata;
  if( plane != us->sourcePlane ) 
    {
      cerr << "Internal Error:  nmb_FlattenedPlane::sourcePlaneChangeCallback "
	   << "called with inconsistent nmb_FlattenedPlane and source plane." 
	   << endl;
      return;
    }
  us->_handleSourcePlaneChange( x, y );
} // end sourcePlaneChangeCallback



void nmb_FlattenedPlane::
_handleSourcePlaneChange( int x, int y )
{
  this->flatPlane->setValue(x, y, 
			    (float) (sourcePlane->value(x, y) +
				     this->offset - this->dx * x 
				     - this->dy * y ) );
} // end _handleSourcePlaneChange



void nmb_FlattenedPlane::
sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
		     vrpn_int32 synchCalculatedPlaneMessageType ) const
{
  char planemsg [1024];
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;

  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) FLATTENED_PLANE_TYPE );
  vrpn_buffer( &bufptr, &msglen, (vrpn_float64) dx );
  vrpn_buffer( &bufptr, &msglen, (vrpn_float64) dy );
  vrpn_buffer( &bufptr, &msglen, (vrpn_float64) offset );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) flatPlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, flatPlane->name()->Characters(),
	      flatPlane->name()->Length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane->name()->Characters(),
	      sourcePlane->name()->Length() );
  
  timeval now;
  gettimeofday(&now, NULL);
  conn->pack_message(1024 - msglen, now, synchCalculatedPlaneMessageType, 
		     senderID, planemsg, vrpn_CONNECTION_RELIABLE);

} // end sendCalculatedPlane( ... )



/* static */
nmb_CalculatedPlane* nmb_FlattenedPlane::
_handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  const char * bufptr = p.buffer;
  vrpn_int32 planeType;

  vrpn_unbuffer( &bufptr, &planeType );
  if( planeType != nmb_CalculatedPlane::FLATTENED_PLANE_TYPE )
    {
      char s[] = "Could not create flattened plane from remote source.  "
	"Wrong type.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  vrpn_int32 outputPlaneNameLen;
  vrpn_int32 sourcePlaneNameLen;
  vrpn_float64 dx;
  vrpn_float64 dy;
  vrpn_float64 offset;

  vrpn_unbuffer( &bufptr, &dx );
  vrpn_unbuffer( &bufptr, &dy );
  vrpn_unbuffer( &bufptr, &offset );
  vrpn_unbuffer( &bufptr, &outputPlaneNameLen );
  vrpn_unbuffer( &bufptr, &sourcePlaneNameLen );

  char* outputPlaneName = new char[ outputPlaneNameLen + 1 ];
  char* sourcePlaneName = new char[ sourcePlaneNameLen + 1 ];
  vrpn_unbuffer( &bufptr, outputPlaneName, outputPlaneNameLen );
  vrpn_unbuffer( &bufptr, sourcePlaneName, sourcePlaneNameLen );
  outputPlaneName[outputPlaneNameLen] = '\0';
  sourcePlaneName[sourcePlaneNameLen] = '\0';

  nmb_FlattenedPlane* newFlatPlane = new nmb_FlattenedPlane;
  newFlatPlane->dx = dx;
  newFlatPlane->dy = dy;
  newFlatPlane->offset = offset;
  
  // Is the destination plane name the same as the source plane name?
  if( strcmp( outputPlaneName, sourcePlaneName ) == 0 ) 
    {
      char s[] = "Cannot create flattened plane.  "
	"Plane cannot flatten from itself.";
      delete outputPlaneName;
      delete sourcePlaneName;
      throw nmb_CalculatedPlaneCreationException( s );
    }
  
  // try to get the requested source plane...
  newFlatPlane->sourcePlane 
    = dataset->inputGrid->getPlaneByName(sourcePlaneName);
  if( newFlatPlane->sourcePlane == NULL )
    {
      char s[] = "Cannot create flattened plane from remote. " \
	" Could not get input plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, sourcePlaneName );
      delete outputPlaneName;
      delete sourcePlaneName;
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // create output plane
  newFlatPlane->flatPlane 
    = newFlatPlane->createFlattenedPlane( dataset, outputPlaneName );

  // add new flattened plane to the dataset
  dataset->addNewCalculatedPlane( newFlatPlane );
  
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

