#include "URHeightField.h"
#include <GL/glut_UNC.h>
#include <v.h>

URHeightField::URHeightField():URender()
{
  obj_type = URHEIGHTFIELD;
  d_displayListID = 0;
  visible = false;
  d_textureEnabled = false;
  int i;
  // initialize transformation to the identity
  for (i = 0; i < 16; i++) {
	  d_worldFromObject[i] = 0;
  }
  d_worldFromObject[0] = 1;
  d_worldFromObject[5] = 1;
  d_worldFromObject[10] = 1;
  d_worldFromObject[15] = 1;
}

//virtual 
URHeightField::~URHeightField()
{
  if (d_displayListID != 0) {
    glDeleteLists(d_displayListID, 1);
	d_displayListID = 0;
  }
}

//virtual 
int URHeightField::Render(void *userdata)
{
	if (d_displayListID != 0 && visible) {
		glPushAttrib(GL_TRANSFORM_BIT);
		if (d_textureEnabled && texture) {
			texture->enable(textureTransform, 
				d_worldFromObject, textureInWorldCoordinates);
		}

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMultMatrixd(d_worldFromObject);
		
		glCallList(d_displayListID);
		glPopMatrix();
		glPopAttrib();
		if (d_textureEnabled && texture) {
			texture->disable();
		}
	}
	if (recursion) {
		return ITER_CONTINUE;
	} else {
		return ITER_STOP;
	}
}

void URHeightField::setSurface(nmb_Image *heightValues, 
      double xmin, double ymin, double xmax, double ymax,
	  int stride)
{
  d_minX = xmin;
  d_minY = ymin;
  d_maxX = xmax;
  d_maxY = ymax;
  buildDisplayList(heightValues, stride);
}

inline void vector_cross_X (double a[3], double c[3]) {
    c[0] = 0.0f;
    c[1] = a[2];
    c[2] = -a[1];
}

inline void vector_cross_Y (double a[3], double c[3]) {
    c[0] = -a[2];
    c[1] = 0.0f;
    c[2] = a[0];
}

inline void vector_cross_NX (double a[3], double c[3]) {
    c[0] = 0.0f;
    c[1] = -a[2];
    c[2] = a[1];
}

inline void vector_cross_NY (double a[3], double c[3]) {
    c[0] = a[2];
    c[1] = 0.0f;
    c[2] = -a[0];
}


inline  void    vector_add(double a[3], double b[3], double c[3])
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

inline  void    vector_normalize(double a[3])
{
    double mag;

    mag = sqrt(a[0] * a[0] + a[1]*a[1] + a[2]*a[2]);

    if (mag == 0.0) {
        fprintf(stderr,"vector_normalize:  vector has zero magnitude\n");
        a[0] = a[1] = a[2] = 1.0/sqrt(3.0);
    } else {
        mag = 1.0/mag;
        a[0] *= mag;
        a[1] *= mag;
        a[2] *= mag;
    }
}

// static
int URHeightField::computeNormal(nmb_Image *heightValues, int x, int y,
     double normal[3])
{
  double diff_vec[3];
  double local_norm[3];

  if (x < 0 || x > heightValues->width()-1 ||
      y < 0 || y > heightValues->height()-1) {
    return -1;
  }

  double dx = heightValues->widthWorld()/(double)(heightValues->width()-1);
  double dy = heightValues->heightWorld()/(double)(heightValues->height()-1);
  double dz = 1.0;
  normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;

  if (x+1 < heightValues->width()) {
        diff_vec[0] = dx;
        diff_vec[1] = 0;
        diff_vec[2] = (float) (dz * (heightValues->getValue(x+1,y) -
            heightValues->getValue(x, y)));
        vector_cross_Y(diff_vec, local_norm);
        vector_add(local_norm,normal, normal);
  }
  if (y+1 < heightValues->height()) {
        diff_vec[0] = 0;
        diff_vec[1] = dy;
        diff_vec[2] = (float) (dz * (heightValues->getValue(x,y+1) -
            heightValues->getValue(x,y)));
        vector_cross_NX(diff_vec, local_norm);
        vector_add(local_norm,normal, normal);
  }
  if (x-1 >= 0) {
        diff_vec[0] = -dx;
        diff_vec[1] = 0;
        diff_vec[2] = (float) (dz * (heightValues->getValue(x-1,y) -
            heightValues->getValue(x, y)));
        vector_cross_NY(diff_vec, local_norm);
        vector_add(local_norm,normal, normal);
  }
  if (y-1 >= 0) {
        diff_vec[0] = 0;
        diff_vec[1] = -dy;
        diff_vec[2] = (float) (dz * (heightValues->getValue(x,y-1) -
            heightValues->getValue(x, y)));
        vector_cross_X(diff_vec, local_norm);
        vector_add(local_norm,normal, normal);
  }
  vector_normalize(normal);
  return 0;
}

void URHeightField::renderWithoutDisplayList(nmb_Image *heightValues, 
	double xmin, double ymin, double xmax, double ymax,
	int stride)
{
	int strideX = stride;
	int strideY = stride;

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);

	// scale normal vectors to unit length after transformation
	glEnable(GL_NORMALIZE);	

	// setup material parameters
	/*
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_diffuse[] = {0.4, 0.4, 0.4, 1.0 };
	GLfloat mat_ambient[] = {0.1, 0.1, 0.1, 1.0 };
	GLfloat mat_emission[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat mat_shininess[] = { 100.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	*/
	glColor4f(1.0, 0.7, 1.0, 1.0);//0.7);
	glEnable(GL_COLOR_MATERIAL);

	// setup lighting
	GLfloat light_position[] = { 0.0, 1.0, 1.0, 0.0 };
	GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat light_diffuse[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat light_specular[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat global_ambient[] = {0.0, 0.0, 0.0, 1.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// enable alpha blending so surface can be transparent
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// For each vertex we only specify the position and normal so
	// we need to disable the other vertex array specifications that
	// are enabled by the other graphics code
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	glDisableClientState(GL_EDGE_FLAG_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	int numX = heightValues->width();
	int numY = heightValues->height();
	double lengthXinWorld = d_maxX - d_minX;
	double lengthYinWorld = d_maxY - d_minY;
	int numStrips;// = numY-1;

	numStrips = (int)ceil((double)(numY-1)/(double)strideY);
	int halfStripLength = ((int)ceil((double)(numX-1)/(double)strideX) + 1);
	int stripLength = 2*halfStripLength;

	GLfloat *normalArray = new GLfloat[stripLength*3];
	GLfloat *vertexArray = new GLfloat[stripLength*3];
	double normal[3] = {0,0,1};
	double y_incr = lengthYinWorld/(double)(numY-1);
	double x_incr = lengthXinWorld/(double)(numX-1);

	int arrayIndex;
	int stripIndex;

	double strip_y_min = d_minY, strip_y_max = d_minY;
	int j_min = 0, j_max = 0;
	for (stripIndex = 0; stripIndex < numStrips; stripIndex++) {
		j_min = j_max;
		j_max += strideY;
		strip_y_min = strip_y_max;
		strip_y_max += y_incr*strideY;
		if (j_max > numY-1) {
			j_max = numY-1;
			strip_y_max = lengthYinWorld;
		}
		double x_value = d_minX;
		int i = 0;
		for (arrayIndex = 0; arrayIndex < halfStripLength; arrayIndex++) {
			
			vertexArray[6*arrayIndex] = x_value;
			vertexArray[6*arrayIndex+1] = strip_y_min;
			vertexArray[6*arrayIndex+2] = heightValues->getValue(i, j_min);

			computeNormal(heightValues, i, j_min, normal);
			normalArray[6*arrayIndex] = normal[0];
			normalArray[6*arrayIndex+1] = normal[1];
			normalArray[6*arrayIndex+2] = normal[2];

			vertexArray[6*arrayIndex+3] = x_value;
			vertexArray[6*arrayIndex+4] = strip_y_max;
			vertexArray[6*arrayIndex+5] = heightValues->getValue(i, j_max);

			computeNormal(heightValues, i, j_max, normal);
			normalArray[6*arrayIndex+3] = normal[0];
			normalArray[6*arrayIndex+4] = normal[1];
			normalArray[6*arrayIndex+5] = normal[2];

			x_value += strideX*x_incr;
			i += strideX;
			if (i > numX-1) {
				i = numX-1;
				x_value = lengthXinWorld;
			}
		}

		glNormalPointer(GL_FLOAT, 0, normalArray);
		glVertexPointer(3, GL_FLOAT, 0, vertexArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, stripLength);
	}


	delete [] vertexArray;
	delete [] normalArray;
	glPopAttrib();
	glPopClientAttrib();

}

void URHeightField::buildDisplayList(nmb_Image *heightValues, 
		int stride)
{
	v_gl_set_context_to_vlib_window();
	if (d_displayListID != 0) {
		glDeleteLists(d_displayListID, 1);
		d_displayListID = 0;
	}
	if (heightValues == NULL) {
		return;
	}
	d_displayListID = glGenLists(1);
	if (d_displayListID == 0) {
		fprintf(stderr, "nm_TipRenderer::buildDisplayList: glGenLists failed\n");
		return;
	}

	glNewList(d_displayListID, GL_COMPILE);

	renderWithoutDisplayList(heightValues, d_minX, d_minY, 
		d_maxX, d_maxY, stride);

	glEndList();

}

void URHeightField::setWorldFromObjectTransform(
				 double *matrix)
{
	int i;
	for (i = 0; i < 16; i++) {
		d_worldFromObject[i] = matrix[i];
	}
}

void URHeightField::setSurfaceRegion(double minX, double minY, double maxX, double maxY)
{
  // the surface currently goes from (d_minX, d_minY) to (d_maxX, d_maxY)
  // now we need to figure out what scaling and translation to apply to get it to
  // fit (minX, minY) (maxX, maxY)
  // m[0]*d_minX + m[4]*d_minY + m[12] = minX
  // m[1]*d_minX + m[5]*d_minY + m[13] = minY
  // m[0]*d_maxX + m[4]*d_maxY + m[12] = maxX
  // m[1]*d_maxX + m[5]*d_maxY + m[13] = maxY
  // m[0]*d_minX + m[4]*d_maxY + m[12] = minX
  // m[1]*d_minX + m[5]*d_maxY + m[13] = maxY

  double scaleX = (maxX - minX)/(d_maxX - d_minX);	// m[0]
  double scaleY = (maxY - minY)/(d_maxY - d_minY);	// m[5]
  double translateX = minX - scaleX*d_minX; // m[12]
  double translateY = minY - scaleY*d_minY; // m[13]

  double m[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
  m[0] = scaleX;
  m[5] = scaleY;
  m[12] = translateX;
  m[13] = translateY;
  setWorldFromObjectTransform(m);
}

void URHeightField::setTextureEnable(bool enable)
{
	d_textureEnabled = enable;
}

int URHeightField::SetProjTextureAll(void *userdata)
{
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URHeightField::SetTextureTransformAll(void *userdata)
{
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}
