
#ifdef sgi
#include <unistd.h>  // for gethostname
#endif
#ifdef _WIN32
#include <winsock2.h>  // for gethostname
#endif

#include "nmb_MorphologyPlane.h"
#include "BCPlane.h"
#include "nmb_Dataset.h"

#include <GL/gl.h> 
#include <GL/glut_unc.h>

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif

// static 
const int nmb_MorphologyPlane::
DILATE_TYPE = 0;

// static
const int nmb_MorphologyPlane::
ERODE_TYPE = 1;

// static
const int nmb_MorphologyPlane::
OPEN_TYPE = 2;

// static
const int nmb_MorphologyPlane::
CLOSE_TYPE = 3;

nmb_MorphologyPlane::
nmb_MorphologyPlane( const char* inputPlaneName,
         const char* tipImageName,
         int morphologySelect,
		 const char* outputPlaneName,
		 nmb_Dataset* dataset 
		 // Dataset to which this plane will be added.
		 )
  throw( nmb_CalculatedPlaneCreationException )
  : nmb_CalculatedPlane( outputPlaneName, dataset )
{
  // Are all the pointer arguments valid?
  if( inputPlaneName == NULL || tipImageName == NULL
      || dataset == NULL || dataset->inputGrid == NULL )
    {
      char s[] = "Internal error in nmb_MorphologyPlane(...):  invalid argument.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  // Is the a destination plane name the same as the source plane name?
  if( strcmp( outputPlaneName, inputPlaneName ) == 0
      || strcmp( outputPlaneName, tipImageName ) == 0) 
    {
      char s[] = "Cannot create morphology plane.  "
	"Morphology plane cannot have the same name as the original plane or tip image.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  this->morphologySelect = morphologySelect;

  // try to get the requested source planes...
  this->sourcePlane = dataset->inputGrid->getPlaneByName(inputPlaneName);
  this->tipImage = dataset->dataImages->getImageByName(tipImageName);

  if( this->sourcePlane == NULL || this->tipImage == NULL)
    {
      char s[] = "Cannot create morphology plane.  "
	"Could not get input plane or image:  ";
      char msg[1024];
      if (sourcePlane == NULL)
	sprintf( msg, "%s%s.", s, inputPlaneName );
      else if (tipImage == NULL)
    sprintf( msg, "%s%s.", s, tipImageName );
      else // how did this happen?
    sprintf( msg, "%s.", s );
      throw nmb_CalculatedPlaneCreationException( msg );
    }

  // Ensure that the units match on the input planes.
  // Actually, only warn if they are not the same
  if ( sourcePlane->units()->compare(*tipImage->unitsValue()) != 0) 
    {
      fprintf(stderr, "nmb_MorphologyPlane(...): Unit mismatch\n");
      fprintf(stderr, "    (%s vs. %s)\n",
	      sourcePlane->units()->c_str(),
	      tipImage->unitsValue()->c_str());
    }
  
  //////////////
  // create output plane 

    int x, y;
    double z;

    
    // create a display list for the tip

    // get the valid data region
    short tip_xmin = 0, tip_ymin = 0, tip_xmax = 0, tip_ymax = 0;
    tipImage->validDataRange( &tip_xmin, &tip_xmax, &tip_ymin, &tip_ymax );

    short tip_xnum = tip_xmax - tip_xmin + 1;
    short tip_ynum = tip_ymax - tip_ymin + 1;

    GLuint dilateTip = glGenLists(1);

    glNewList(dilateTip, GL_COMPILE);
    glPointSize(1);
    glBegin(GL_POINTS);

    for (x = tip_xmin; x < tip_xnum; x++) {
        for (y = tip_ymin; y < tip_ynum; y++) {
            z = tipImage->getValue(x, y) - tipImage->maxValue();

            glVertex3f(x - tip_xnum / 2, y - tip_ynum / 2, z);
        }
    }

    glEnd();
    glEndList();

    GLuint erodeTip = glGenLists(1);

    glNewList(erodeTip, GL_COMPILE);
    glPointSize(1);
    glBegin(GL_POINTS);
    for (x = tip_xmin; x < tip_xnum; x++) {
        for (y = tip_ymin; y < tip_ynum; y++) {
            z = tipImage->getValue(x, y) - tipImage->maxValue();

            glVertex3f(x - tip_xnum / 2, y - tip_ynum / 2, -z);
        }
    }
    glEnd();
    glEndList();


    // Use the units for the first plane.  
    // It is assumed that the second will have been scaled appropriately.
    char newunits[1000];
    sprintf(newunits, "%s", sourcePlane->units()->c_str());
    createCalculatedPlane( newunits, sourcePlane, dataset );

    // get the valid data region
    short xmin = 0, ymin = 0, xmax = 0, ymax = 0;
    sourcePlane->findValidDataRange( &xmin, &xmax, &ymin, &ymax );

    short xnum = xmax - xmin + 1;
    short ynum = ymax - ymin + 1;

    double zmin = sourcePlane->minValue();
    double zmax = sourcePlane->maxValue();

    // set up OpenGL
    short border = 1;       // handle edge effects

    short offset_x = 500;       
    short offset_y = 500;
    // XXX
    // This moves the viewport we are using closer to the center of the screen, 
    // since we need this portion of the window to be on top for the rendering
    // to work correctly.  A more elegant solution would be to render to an offscreen buffer...
    glViewport(offset_x - border, offset_y - border, xnum + 2*border, ynum + 2*border);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-border, xnum - 1 + border, -border, ynum - 1 + border, -zmin, -zmax);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);

    int inner_radius = 2;
    int outer_radius = 5;

    // allocate buffer for read back
    GLfloat* depthBuffer = new GLfloat[xnum * ynum];

    switch(morphologySelect) {
        case DILATE_TYPE:
            glClearDepth(0.0);
            glDepthFunc(GL_GREATER);

            glClear(GL_DEPTH_BUFFER_BIT);

            // do dilation in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = sourcePlane->value(x, y);

                    if (z > zmin) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(dilateTip);
                        glPopMatrix();
                    }
                }
            }
            break;
        case ERODE_TYPE:
            glClearDepth(1.0);
            glDepthFunc(GL_LESS);

            glClear(GL_DEPTH_BUFFER_BIT);

            // do erosion in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = sourcePlane->value(x, y);

                    if (z < zmax) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(erodeTip);
                        glPopMatrix();
                    }
                }
            }
            break;
        case OPEN_TYPE:
            // erode, then dilate
            glClearDepth(1.0);
            glDepthFunc(GL_LESS);

            glClear(GL_DEPTH_BUFFER_BIT);

            // do erosion in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = sourcePlane->value(x, y);

                    if (z < zmax) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(erodeTip);
                        glPopMatrix();
                    }
                }
            }

            // read back z buffer
            glReadPixels(offset_x, offset_y, xnum, ynum, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

            glDepthFunc(GL_GREATER);

            // do dilation in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = zmin + depthBuffer[y * ynum + x] * (zmax - zmin);

                    if (z > zmin) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(dilateTip);
                        glPopMatrix();
                    }
                }
            }
            break;
        case CLOSE_TYPE:
            // dilate, then erode
            glClearDepth(0.0);
            glDepthFunc(GL_GREATER);

            glClear(GL_DEPTH_BUFFER_BIT);

            // do dilation in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = sourcePlane->value(x, y);

                    if (z > zmin) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(dilateTip);
                        glPopMatrix();
                    }
                }
            }

            // read back z buffer
            glReadPixels(offset_x, offset_y, xnum, ynum, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

            glDepthFunc(GL_LESS);

            // do erosion in zbuffer
            for (x = 0; x < xnum; x++) {
                for (y = 0; y < ynum; y++) {
                    z = zmin + depthBuffer[y * ynum + x] * (zmax - zmin);

                    if (z < zmax) {
                        glPushMatrix();
                        glTranslatef(x, y, z);
                        glCallList(erodeTip);
                        glPopMatrix();
                    }
                }
            }
            break;
        default:
            fprintf(stderr, "Unrecognized mathematical morphology type\n");
            break;
    }

    // return OpenGL to normalcy
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glClearDepth(1.0);
    glDepthFunc(GL_LESS);

    // get rid of display lists
    glDeleteLists(dilateTip, 1);
    glDeleteLists(erodeTip, 1);

    // read back z buffer
    glReadPixels(offset_x, offset_y, xnum, ynum, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

    // fill in new plane
    for (x = 0; x < xnum; x++) {
        for (y = 0; y < ynum; y++) {
            calculatedPlane->setValue(x + xmin, y + ymin, zmin + depthBuffer[y * ynum + x] * (zmax - zmin));
        }
    }

    delete [] depthBuffer;

    // register ourselves to receive plane updates
    sourcePlane->add_callback( sourcePlaneChangeCallback, this );

} // end nmb_MorphologyPlane( ... )


nmb_MorphologyPlane::
~nmb_MorphologyPlane( )
{
  // unregister ourselves to receive plane updates
  if( sourcePlane != NULL )
    sourcePlane->remove_callback( sourcePlaneChangeCallback, this );
} // end ~nmb_MorphologyPlane


bool nmb_MorphologyPlane::
dependsOnPlane( const BCPlane* const plane )
{
  if( plane == NULL ) return false;
  if( plane == sourcePlane /* pointer comparison */ )
    return true;
  else
    return false;
}


bool nmb_MorphologyPlane::
dependsOnPlane( const char* planeName )
{
  if( planeName == NULL ) return false;
  if( this->sourcePlane->name()->compare(planeName) == 0 )
    return true;
  else
    return false;
}


/* static */
void nmb_MorphologyPlane::
sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
			   void* userdata )
{
  nmb_MorphologyPlane* us = (nmb_MorphologyPlane*) userdata;
  if( plane != us->sourcePlane ) 
    {
      cerr << "Internal Error:  nmb_MorphologyPlane::sourcePlaneChangeCallback "
	   << "called with inconsistent nmb_MorphologyPlane and source plane." 
	   << endl;
      return;
    }
  us->_handleSourcePlaneChange( x, y );
} // end sourcePlaneChangeCallback



void nmb_MorphologyPlane::
_handleSourcePlaneChange( int x, int y )
{

    // XXX

/*
  calculatedPlane->setValue( x, y, 
                             (float) ( sourcePlane1->value(x, y) 
                                       + scale * sourcePlane2->value(x, y) ) );
*/
} // end _handleSourcePlaneChange



void nmb_MorphologyPlane::
sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
		     vrpn_int32 synchCalculatedPlaneMessageType ) const
{
  char planemsg [1024];
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;

  vrpn_buffer( &bufptr, &msglen, MORPHOLOGY_PLANE_TYPE );
  vrpn_buffer( &bufptr, &msglen, morphologySelect );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) sourcePlane->name()->length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) tipImage->name()->length() );
  vrpn_buffer( &bufptr, &msglen, (vrpn_int32) calculatedPlane->name()->length() );
  vrpn_buffer( &bufptr, &msglen, sourcePlane->name()->c_str(),
	       sourcePlane->name()->length() );
  vrpn_buffer( &bufptr, &msglen, tipImage->name()->c_str(),
	       tipImage->name()->length() );
  vrpn_buffer( &bufptr, &msglen, calculatedPlane->name()->c_str(),
	       calculatedPlane->name()->length() );
  
  timeval now;
  gettimeofday(&now, NULL);
  conn->pack_message(1024 - msglen, now, synchCalculatedPlaneMessageType, 
		     senderID, planemsg, vrpn_CONNECTION_RELIABLE);
} // end sendCalculatedPlane( ... )



/* static */
nmb_CalculatedPlane* nmb_MorphologyPlane::
_handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  const char * bufptr = p.buffer;
  vrpn_int32 planeType;

  vrpn_unbuffer( &bufptr, &planeType );
  if( planeType != nmb_CalculatedPlane::MORPHOLOGY_PLANE_TYPE )
    {
      char s[] = "Could not create morphology plane from remote source.  "
	"Wrong type.";
      throw nmb_CalculatedPlaneCreationException( s );
    }

  vrpn_int32 morphologySelect;
  vrpn_int32 sourcePlaneNameLen;
  vrpn_int32 tipImageNameLen;
  vrpn_int32 outputPlaneNameLen;

  vrpn_unbuffer( &bufptr, &morphologySelect );
  vrpn_unbuffer( &bufptr, &sourcePlaneNameLen );
  vrpn_unbuffer( &bufptr, &tipImageNameLen );
  vrpn_unbuffer( &bufptr, &outputPlaneNameLen );

  char* sourcePlaneName = new char[ sourcePlaneNameLen + 1 ];
  char* tipImageName = new char[ tipImageNameLen + 1];
  char* outputPlaneName = new char[ outputPlaneNameLen + 1 ];
  vrpn_unbuffer( &bufptr, sourcePlaneName, sourcePlaneNameLen );
  vrpn_unbuffer( &bufptr, tipImageName, tipImageNameLen );
  vrpn_unbuffer( &bufptr, outputPlaneName, outputPlaneNameLen );
  sourcePlaneName[sourcePlaneNameLen] = '\0';
  tipImageName[tipImageNameLen] = '\0';
  outputPlaneName[outputPlaneNameLen] = '\0';

  // test for idempotency
  nmb_MorphologyPlane* samePlane 
    = dynamic_cast<nmb_MorphologyPlane*>( nmb_CalculatedPlane::getCalculatedPlane( outputPlaneName ) );
  // samePlane will be NULL EITHER if there is currently no plane of the given name,
  // OR if the run-time identified type of the plane with the given name is not 
  // nmb_MorphologyPlane
  if( samePlane != NULL )
  { // see if we got a message to recreate the same plane
    if( morphologySelect == samePlane->morphologySelect 
        && samePlane->sourcePlane->name()->compare(sourcePlaneName) == 0 
        && samePlane->tipImage->name()->compare(tipImageName) == 0)
    {
      // the requested plane is exactly the same as one that already exists,
      // so don't change anything
      delete outputPlaneName;
      delete tipImageName;
      delete sourcePlaneName;
      return samePlane;
    }
  }


  nmb_MorphologyPlane* newMorphologyPlane 
    = new nmb_MorphologyPlane( sourcePlaneName, tipImageName, morphologySelect, 
                               outputPlaneName, dataset );

  // register new morphology plane to receive plane updates
  newMorphologyPlane->sourcePlane->add_callback( sourcePlaneChangeCallback, 
					     newMorphologyPlane );

  delete sourcePlaneName;
  delete tipImageName;
  delete outputPlaneName;
  return newMorphologyPlane;
} // _handle_PlaneSynch( ... )

#ifdef _WIN32
#pragma warning( pop )
#endif

