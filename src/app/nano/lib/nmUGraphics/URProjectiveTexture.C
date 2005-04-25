#include "URProjectiveTexture.h"
#include "GL/glu.h"
#include "error_display.h"

URProjectiveTexture::URProjectiveTexture():
  d_textureObjectCreated(false),
  d_imageChangedSinceTextureInstalled(false),
  d_enabled(false),
  d_doingFastUpdates(false),
  d_textureID(0),
  d_greyscaleImage(NULL),
  d_colorImage(NULL),
  d_textureMatrixNumX(0),
  d_textureMatrixNumY(0),
  d_textureBlendFunction(GL_DECAL),
  d_opacity(1.0),
  d_wrapMode(GL_REPEAT),
  d_colormap_data_min(0),
  d_colormap_data_max(1),
  d_colormap_color_min(0),
  d_colormap_color_max(1),
  d_colormap(NULL),
  d_update_colormap(false),
  d_update_opacity(false),
  d_greyscaleImageTooBig(false),
  d_colorImageTooBig(false)
{

}

void URProjectiveTexture::setWrapMode(GLuint wrapMode)
{
	d_wrapMode = wrapMode;
	return;
}

int URProjectiveTexture::doFastUpdates(bool enable)
{
	d_doingFastUpdates = enable;
	return 0;
}

int URProjectiveTexture::setOpacity(double opacity)
{
	// clamp to [0..1]
	if (opacity > 1) {
		d_opacity = 1.0;
	} else if (opacity < 0) {
		d_opacity = 0.0;
	} else {
		d_opacity = opacity;
	}
	d_update_opacity = true;
	return 0;
}

int URProjectiveTexture::setImage(nmb_Image *image)
{
	d_colorImage = NULL;
    if (d_greyscaleImage) {
        if (d_greyscaleImage->width() != image->width() || 
            d_greyscaleImage->height() != image->height() ||
            d_greyscaleImage != image) {
            d_imageChangedSinceTextureInstalled = true;
        }
    }
    else {
        d_imageChangedSinceTextureInstalled = true;
    }
    d_greyscaleImage = image;

	return 0;
}

int URProjectiveTexture::setImage(PPM *image)
{
	d_colorImage = image;
	d_greyscaleImage = NULL;
	d_imageChangedSinceTextureInstalled = true;
	return 0;
}

int URProjectiveTexture::setTextureBlendFunction(GLuint blendFunc)
{
  if (blendFunc == GL_BLEND || blendFunc == GL_MODULATE ||
	  blendFunc == GL_DECAL) {
	d_textureBlendFunction = blendFunc;
  } else {
	fprintf(stderr, "setTextureBlendFunction: invalid value\n");
	return -1;
  }
  return 0;
}

int URProjectiveTexture::installTexture(int width, int height, void *data,
					GLuint internalFormat, GLuint dataFormat, GLuint dataType)
{
	
	if (!d_textureObjectCreated) {
		glGenTextures(1, &d_textureID);
		d_textureObjectCreated = true;
	}

	glBindTexture(GL_TEXTURE_2D, d_textureID);
	d_textureMatrixNumX = width;
	d_textureMatrixNumY = height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	float tex_color[4] = {1.0, 1.0, 1.0, 1.0};
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tex_color);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, d_wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, d_wrapMode);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	int errval;
    while ((errval = glGetError()) != GL_NO_ERROR) {
      fprintf(stderr, " Error before making texture: %s.\n", gluErrorString(errval));
    }
	if (gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, width,
                       height, dataFormat, dataType, data)!=0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 width, height, 0, dataFormat, dataType, data);
	}
    while ((errval = glGetError()) != GL_NO_ERROR) {
      fprintf(stderr, " Error making texture: %s.\n", gluErrorString(errval));
    }
	return 0;
}

int URProjectiveTexture::createTexture(bool doFastUpdates)
{
	glPushAttrib(GL_TEXTURE_BIT);
	if (d_greyscaleImage) {
		if (!d_textureObjectCreated){
			glGenTextures(1, &d_textureID);
			d_textureObjectCreated = true;
		}
		glBindTexture(GL_TEXTURE_2D, d_textureID);
		d_textureMatrixNumX = d_greyscaleImage->width() + 
			d_greyscaleImage->borderXMin() + d_greyscaleImage->borderXMax();
		d_textureMatrixNumY = d_greyscaleImage->height() + 
			d_greyscaleImage->borderYMin() + d_greyscaleImage->borderYMax();

		double log_base2 = log((double)d_textureMatrixNumX)/log(2.0);
		d_textureMatrixNumX = (int)pow(2.0, ceil(log_base2));
		log_base2 = log((double)d_textureMatrixNumY)/log(2.0);
		d_textureMatrixNumY = (int)pow(2.0, ceil(log_base2));

		
		if (doFastUpdates) {
			initTextureMatrixNoMipmap();
			updateTextureNoMipmap();
		} else {
			updateTextureMipmap();
		}
		d_doingFastUpdates = doFastUpdates;
		d_imageChangedSinceTextureInstalled = false;
	} else if (d_colorImage) {
		if (!d_textureObjectCreated){
			glGenTextures(1, &d_textureID);
			d_textureObjectCreated = true;
		}
		d_textureMatrixNumX = d_colorImage->nx;
		d_textureMatrixNumY = d_colorImage->ny;
		double log_base2 = log((double)d_textureMatrixNumX)/log(2.0);
		d_textureMatrixNumX = (int)pow(2.0, ceil(log_base2));
		log_base2 = log((double)d_textureMatrixNumY)/log(2.0);
		d_textureMatrixNumY = (int)pow(2.0, ceil(log_base2));
		loadColorImageMipmap();
	}

	glPopAttrib();
	return 0;
}

int URProjectiveTexture::enable(double *textureTransform,
								double *objectToWorldTransform,
								bool textureInWorldCoordinates)
{
	if (d_enabled) {
		fprintf(stderr, "URProjectiveTexture::enable: Error, already enabled\n");
		return -1;
	}
	if (!d_textureObjectCreated && !d_greyscaleImage && !d_colorImage) return 0;

	double imageToTexture[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

	if (d_greyscaleImage || d_colorImage) {
		if (d_imageChangedSinceTextureInstalled) {
			createTexture(d_doingFastUpdates);
			d_imageChangedSinceTextureInstalled = false;
		}
        else if (d_doingFastUpdates) {
            updateTextureNoMipmap();
        } else if (d_update_colormap || d_update_opacity) {
			createTexture(d_doingFastUpdates);
            d_update_opacity = false;
			d_update_colormap = false;
		}
		if (d_greyscaleImage) {
			d_greyscaleImage->getImageToTextureTransform(imageToTexture,
				d_textureMatrixNumX, d_textureMatrixNumY);
		} else {
			double scaleFactorX = (double)(d_colorImage->nx)/(double)d_textureMatrixNumX;
			double scaleFactorY = (double)(d_colorImage->ny)/(double)d_textureMatrixNumY;
			imageToTexture[0] = scaleFactorX;
			imageToTexture[5] = scaleFactorY;
		}
	}

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPushAttrib(GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
	if (d_textureObjectCreated) {
		glBindTexture(GL_TEXTURE_2D, d_textureID);
	}
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, d_textureBlendFunction);
	GLfloat textureEnvColor[4] = {1.0, 1.0, 1.0, 1.0};
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,textureEnvColor);

    GLfloat eyePlaneS[] = {1.0, 0.0, 0.0, 0.0};
    GLfloat eyePlaneT[] = {0.0, 1.0, 0.0, 0.0};
    GLfloat eyePlaneR[] = {0.0, 0.0, 1.0, 0.0};
    GLfloat eyePlaneQ[] = {0.0, 0.0, 0.0, 1.0};
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);
    
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);
    
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_R, GL_OBJECT_PLANE, eyePlaneR);
    
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_Q, GL_OBJECT_PLANE, eyePlaneQ);

	switch(d_textureBlendFunction) {
	case GL_BLEND:
	  {
	  float bord_col[4] = {0.0, 0.0, 0.0, 1.0};
	  glTexParameterfv(GL_TEXTURE_2D, 
		  GL_TEXTURE_BORDER_COLOR, bord_col);
	  }
	  break;
	case GL_MODULATE:
	  {
	  float bord_col[4] = {1.0, 1.0, 1.0, 1.0};
	  glTexParameterfv(GL_TEXTURE_2D, 
		  GL_TEXTURE_BORDER_COLOR, bord_col);
	  }
	  break;
	default:
	  {
	  float bord_col[4] = {0.0, 0.0, 0.0, 0.0};
	  glTexParameterfv(GL_TEXTURE_2D, 
		  GL_TEXTURE_BORDER_COLOR, bord_col);
	  }
	  break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, d_wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, d_wrapMode);

	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(imageToTexture);
	if (textureTransform) {
		glMultMatrixd(textureTransform);
	}
	if (textureInWorldCoordinates && objectToWorldTransform) {
		glMultMatrixd(objectToWorldTransform);
	}
	glPopAttrib();

	if (glGetError() != GL_NO_ERROR) {
		fprintf(stderr, 
			"URProjectiveTexture::enable: GL error occurred\n");
	}

	d_enabled = true;
	return 0;
}

int URProjectiveTexture::disable()
{
	if (!d_enabled) {
		return -1;
	}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);

	glPopAttrib();
	glPopClientAttrib();

	d_enabled = false;
	return 0;
}

void URProjectiveTexture::changeDataset(nmb_Dataset *data)
{
	d_greyscaleImage = NULL;
}

int URProjectiveTexture::width() {
    if (d_greyscaleImage) {
        return d_greyscaleImage->width();
    }
    else if (d_colorImage) {
        return d_colorImage->nx;
    }
    return 0;
}

int URProjectiveTexture::height() {
    if (d_greyscaleImage) {
        return d_greyscaleImage->height();
    }
    else if (d_colorImage) {
        return d_colorImage->ny;
    }
    return 0;
}

void URProjectiveTexture::setColorMap(nmb_ColorMap* colormap) {
	if (d_colormap) delete d_colormap;
	if (colormap) {
		d_colormap = new nmb_ColorMap(*colormap);
	} else {
		d_colormap = NULL;
	}
}

void URProjectiveTexture::setColorMapMinMax(float data_min, float data_max, float color_min, float color_max) {
    d_colormap_data_min = data_min;
    d_colormap_data_max = data_max;
    d_colormap_color_min = color_min;
    d_colormap_color_max = color_max;
}

void URProjectiveTexture::setUpdateColorMap(bool value) {
    d_update_colormap = value;
}

int URProjectiveTexture::updateTextureNoMipmap()
{
	if (d_greyscaleImageTooBig) {
		return -1;
	}
	if (!d_greyscaleImage) {
		return -1;
	}

    int XMin = d_greyscaleImage->borderXMin();
    int XMax = d_greyscaleImage->borderXMax();
    int YMin = d_greyscaleImage->borderYMin();
    int YMax = d_greyscaleImage->borderYMax();
    int width = d_greyscaleImage->width();
    int height = d_greyscaleImage->height();
    int imageBufferNumX = width + XMin + XMax;
	int imageBufferNumY = height + YMin + YMax;
	void *imageData = d_greyscaleImage->pixelData();

    int pixelType;
	int pixelSize;
    if (d_greyscaleImage->pixelType() == NMB_UINT8){
        pixelType = GL_UNSIGNED_BYTE;
		pixelSize = sizeof(GLubyte);
    } 
    else if (d_greyscaleImage->pixelType() == NMB_UINT16){
        pixelType = GL_UNSIGNED_SHORT;
		pixelSize = sizeof(GLushort);
    } 
    else if (d_greyscaleImage->pixelType() == NMB_FLOAT32){
        pixelType = GL_FLOAT;
		pixelSize = sizeof(GLfloat);
    } 
    else {
        fprintf(stderr, "URProjectiveTexture::updateTexture:"
             "can't handle pixel type\n"); 
        return -1;      
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);

	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, d_textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, d_wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, d_wrapMode);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    if (d_update_colormap && d_colormap) {
        float dummy;
		for (int i = 0; i < GL_CMAP_SIZE; i++) {
			d_colormap->lookup(i, 0, GL_CMAP_SIZE,
              d_colormap_data_min, d_colormap_data_max, d_colormap_color_min, d_colormap_color_max,
              &d_rmap[i], &d_gmap[i], &d_bmap[i], &dummy);
            d_amap[i] = (i + 1) * (1.0 / GL_CMAP_SIZE);
		}
        d_amap[0] = 0;

        d_update_colormap = false;
    }
    if (d_colormap) {
		// gl first converts the luminance pixels to RGBA, then
		// applies the maps we specify. These are created from our
		// color maps above. We're letting GL do the work for us. 
		glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
		glPixelMapfv(GL_PIXEL_MAP_R_TO_R, GL_CMAP_SIZE, &d_rmap[0]);
		glPixelMapfv(GL_PIXEL_MAP_G_TO_G, GL_CMAP_SIZE, &d_gmap[0]);
		glPixelMapfv(GL_PIXEL_MAP_B_TO_B, GL_CMAP_SIZE, &d_bmap[0]);
        glPixelMapfv(GL_PIXEL_MAP_A_TO_A, GL_CMAP_SIZE, &d_amap[0]);
	}
    else {
        glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    }


    glPixelTransferf(GL_ALPHA_SCALE, d_opacity);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// XXX - for some strange reason, the value given for GL_UNPACK_SKIP_PIXELS 
	// isn't having the desired effect - the whole row of the texture is getting
	// transferred instead of just the part of the row specified
	// XXXX -- it's not the whole row.  comment out the SKIP_ROW and SKIP_PIXELS
	// lines to see.  seems like this is just texel interpolation ouside of the 
	// active image area (since the nmb_Image whence this comes has what can be a
	// very large border around it.
    glPixelStorei(GL_UNPACK_SKIP_ROWS, YMin );
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, XMin );
    glPixelStorei(GL_UNPACK_ROW_LENGTH, imageBufferNumX);
	glTexSubImage2D(GL_TEXTURE_2D, 0, XMin, YMin,
                    width, height,
                    GL_LUMINANCE, pixelType, imageData);

	// A workaround that has the desired result (AAS):
	//  but makes projecting textures of any size slooooooow.... (DTM)
/*
	void *offsetImageData = ((GLubyte *)imageData + (XMin + YMin*imageBufferNumX)*pixelSize);
	int i;
	for (i = 0; i < height; i++) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, XMin, YMin+i,
						width, 1,
						GL_LUMINANCE, pixelType, offsetImageData);
		offsetImageData = ((GLubyte *)offsetImageData + imageBufferNumX*pixelSize);
	}
*/

	glPopClientAttrib();
    glPopAttrib();

    return 0;
}

int URProjectiveTexture::initTextureMatrixNoMipmap()
{
  if (d_textureMatrixNumX==0 ||
	  d_textureMatrixNumY==0) {
    return -1;
  }

  int pixelType;
  if (d_greyscaleImage->pixelType() == NMB_UINT8){
      pixelType = GL_UNSIGNED_BYTE;
  } 
  else if (d_greyscaleImage->pixelType() == NMB_UINT16){
      pixelType = GL_UNSIGNED_SHORT;
  } 
  else if (d_greyscaleImage->pixelType() == NMB_FLOAT32){
      pixelType = GL_FLOAT;
  } 
  else {
      fprintf(stderr, "URProjectiveTexture::updateTexture:"
           "can't handle pixel type\n"); 
      return -1;      
  }

  int array_size = 2*d_textureMatrixNumX*d_textureMatrixNumY;
  void* clearData = NULL; 

  // Check to see if there is enough texture memory
  // XXX - The X and Y dimensions are doubled as a hack to get this to warn
  // us before its too late. The problem may be that some other part of the
  // program could be using texture resources and we wouldn't know about it
  // and this way of checking only tells us if we have enough texture memory
  // assuming all texture resources are available to us (see the opengl red book)
  // Checking this is better than doing no check at all but I can't guarantee
  // that this is a failsafe. (AAS)
  glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,
                    d_textureMatrixNumX*2, d_textureMatrixNumY*2, 0, 
                    GL_LUMINANCE_ALPHA, pixelType, NULL);
  GLint installedWidth;
  GLint installedHeight;
  glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
			GL_TEXTURE_WIDTH, &installedWidth);
  glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
			GL_TEXTURE_HEIGHT, &installedHeight);
  bool outOfTextureMemory = (installedWidth == 0 || installedHeight == 0);

  if (outOfTextureMemory) {
    display_warning_dialog("Warning, the image, %s,\nis too big to display as a texture",
		d_greyscaleImage->name()->c_str());
    d_greyscaleImageTooBig = true;
  } else {
    d_greyscaleImageTooBig = false;
  }

  if (d_greyscaleImageTooBig) {
	  return -1;
  }

  if (pixelType == GL_UNSIGNED_BYTE) {
      clearData = new GLubyte [array_size];

      if (!clearData) {
        fprintf(stderr, 
            "URProjectiveTexture::clearTexture: Error, out of memory\n");
        return -1;
      }

      int i , j , k = 0;
      GLubyte value, alpha;
      for (i = 0; i < d_textureMatrixNumX; i++){
        for (j = 0; j < d_textureMatrixNumY; j++){
          switch(d_textureBlendFunction) {
          case GL_BLEND:
             value = 20*((i/30+j/30)%2);
             alpha = 255;
             break;
          case GL_MODULATE:
             value = 200 + 55*((i/30+j/30)%2);
             alpha = 255;//200 + 55*((i/60+j/60)%2);
             break;
          case GL_DECAL:
             value = 0;
             alpha = 0;
		     break;
	  default:
		 break;
          }
          ((GLubyte*)clearData)[2 * k] = value;
          ((GLubyte*)clearData)[2 * k + 1] = alpha;
          k++;
        }
      }
      
      glPushAttrib(GL_TEXTURE_BIT);
	  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
          glBindTexture(GL_TEXTURE_2D, d_textureID);

	  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				d_textureMatrixNumX, d_textureMatrixNumY, 0, 
				GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, clearData);

	  glPopClientAttrib();
      glPopAttrib();

  }
  else if (pixelType == GL_UNSIGNED_SHORT) {
      clearData = new GLushort [array_size];

      if (!clearData) {
        fprintf(stderr, 
            "URProjectiveTexture::clearTexture: Error, out of memory\n");
        return -1;
      }

      int i , j , k = 0;
      GLushort value, alpha;
      for (i = 0; i < d_textureMatrixNumX; i++){
        for (j = 0; j < d_textureMatrixNumY; j++){
          switch(d_textureBlendFunction) {
          case GL_BLEND:
             value = 5000*((i/30+j/30)%2);
             alpha = 65535;
             break;
          case GL_MODULATE:
             value = 55535 + 10000*((i/30+j/30)%2);
             alpha = 65535;
             break;
          case GL_DECAL:
             value = 0;
             alpha = 0;
		     break;
	      default:
		     break;
          }
          ((GLushort*)clearData)[2 * k] = value;
          ((GLushort*)clearData)[2 * k + 1] = alpha;
          k++;
        }
      }

      glPushAttrib(GL_TEXTURE_BIT);
      glBindTexture(GL_TEXTURE_2D, d_textureID);

	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            d_textureMatrixNumX, d_textureMatrixNumY, 0, 
            GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT, clearData);

      glPopAttrib();
  }
  else if (pixelType == GL_FLOAT) {
      clearData = new GLfloat [array_size];

      if (!clearData) {
        fprintf(stderr, 
            "URProjectiveTexture::clearTexture: Error, out of memory\n");
        return -1;
      }

      int i , j , k = 0;
      GLubyte value, alpha;
      for (i = 0; i < d_textureMatrixNumX; i++){
        for (j = 0; j < d_textureMatrixNumY; j++){
          switch(d_textureBlendFunction) {
          case GL_BLEND:
             value = ((i/30+j/30)%2);
             alpha = 1.0;
             break;
          case GL_MODULATE:
             value = 0.8 + 0.2*((i/30+j/30)%2);
             alpha = 1.0;
             break;
          case GL_DECAL:
             value = 0;
             alpha = 0;
		     break;
	      default:
		     break;
          }
          ((GLfloat*)clearData)[2 * k] = value;
          ((GLfloat*)clearData)[2 * k + 1] = alpha;
          k++;
        }
      }

      glPushAttrib(GL_TEXTURE_BIT);
 //     glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, d_textureID);

	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            d_textureMatrixNumX, d_textureMatrixNumY, 0, 
            GL_LUMINANCE_ALPHA, GL_FLOAT, clearData);

      glPopAttrib();
  }

  if (clearData) {
	delete [] clearData;
  }
  return 0;
}

int URProjectiveTexture::updateTextureMipmap()
{
	if (!d_greyscaleImage) {
		return -1;
	}
    
    int XMin = d_greyscaleImage->borderXMin();
    int XMax = d_greyscaleImage->borderXMax();
    int YMin = d_greyscaleImage->borderYMin();
    int YMax = d_greyscaleImage->borderYMax();
    int width = d_greyscaleImage->width();
    int height = d_greyscaleImage->height();
    int imageBufferNumX = width + XMin + XMax;
	int imageBufferNumY = height + YMin + YMax;
	void *imageData = d_greyscaleImage->pixelData();

    int pixelType;
    if (d_greyscaleImage->pixelType() == NMB_UINT8){
        pixelType = GL_UNSIGNED_BYTE;
    } else if (d_greyscaleImage->pixelType() == NMB_UINT16){
        pixelType = GL_UNSIGNED_SHORT;
    } else if (d_greyscaleImage->pixelType() == NMB_FLOAT32){
        pixelType = GL_FLOAT;
    } else {
        fprintf(stderr, "URProjectiveTexture::updateTexture:"
             "can't handle pixel type\n"); 
        return -1;      
    }

	//glPushAttrib(GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, d_textureID);

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, 
		GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, 
		GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, d_wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, d_wrapMode);

	GLint internalFormat = GL_RGBA;

	if (d_update_colormap && d_colormap) {
        float dummy;
		for (int i = 0; i < GL_CMAP_SIZE; i++) {
			d_colormap->lookup(i, 0, GL_CMAP_SIZE,
              d_colormap_data_min, d_colormap_data_max, d_colormap_color_min, d_colormap_color_max,
              &d_rmap[i], &d_gmap[i], &d_bmap[i], &dummy);
            d_amap[i] = (i + 1) * (1.0 / GL_CMAP_SIZE);
		}
        d_amap[0] = 0;

        d_update_colormap = false;
    }
    if (d_colormap) {
		// gl first converts the luminance pixels to RGBA, then
		// applies the maps we specify. These are created from our
		// color maps above. We're letting GL do the work for us. 
		glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
		glPixelMapfv(GL_PIXEL_MAP_R_TO_R, GL_CMAP_SIZE, &d_rmap[0]);
		glPixelMapfv(GL_PIXEL_MAP_G_TO_G, GL_CMAP_SIZE, &d_gmap[0]);
		glPixelMapfv(GL_PIXEL_MAP_B_TO_B, GL_CMAP_SIZE, &d_bmap[0]);
        glPixelMapfv(GL_PIXEL_MAP_A_TO_A, GL_CMAP_SIZE, &d_amap[0]);
	}

	void* imageDataAlpha = NULL;
    int i, j, k = 0;
    if (pixelType == GL_UNSIGNED_BYTE) {
        imageDataAlpha= new GLubyte[2 * imageBufferNumX * imageBufferNumY];
        for (j = 0; j < imageBufferNumY; j++) {
            for (i = 0; i < imageBufferNumX; i++) {
                ((GLubyte*)imageDataAlpha)[2 * k] = ((GLubyte*)imageData)[k];
                if (j < YMin || j >= height + YMax || i < XMin || i >= width + XMax) {
                    ((GLubyte*)imageDataAlpha)[2 * k + 1] = 0;    // zero alpha for border
                }
                else {
                    ((GLubyte*)imageDataAlpha)[2 * k + 1] = d_opacity*255;
                }
                k++;
            }
        }
	}
    else if (pixelType == GL_UNSIGNED_SHORT) {
        imageDataAlpha= new GLushort[2 * imageBufferNumX * imageBufferNumY];
        for (j = 0; j < imageBufferNumY; j++) {
            for (i = 0; i < imageBufferNumX; i++) {
                ((GLushort*)imageDataAlpha)[2 * k] = ((GLushort*)imageData)[k];
                if (j < YMin || j >= height + YMax || i < XMin || i >= width + XMax) {
                    ((GLushort*)imageDataAlpha)[2 * k + 1] = 0;    // zero alpha for border
                }
                else {
                    ((GLushort*)imageDataAlpha)[2 * k + 1] = d_opacity*65535;
                }
                k++;
            }
        }
    }
    else if (pixelType == GL_FLOAT) {
		nmb_Image *im_copy = new nmb_ImageGrid(d_greyscaleImage);
		im_copy->normalize();
        imageData = im_copy->pixelData();

        imageDataAlpha= new GLfloat[2 * imageBufferNumX * imageBufferNumY];
        for (j = 0; j < imageBufferNumY; j++) {
            for (i = 0; i < imageBufferNumX; i++) {
                ((GLfloat*)imageDataAlpha)[2 * k] = ((GLfloat*)imageData)[k];
                if (j < YMin || j >= height + YMax || i < XMin || i >= width + XMax) {
                    ((GLfloat*)imageDataAlpha)[2 * k + 1] = 0;    // zero alpha for border
                }
                else {
                    ((GLfloat*)imageDataAlpha)[2 * k + 1] = d_opacity;
                }
                k++;
            }
        }
	    nmb_Image::deleteImage(im_copy);
	}
   
    gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat,
	    imageBufferNumX, imageBufferNumY,
	    GL_LUMINANCE_ALPHA, pixelType, imageDataAlpha);

	GLenum err = glGetError();
    if (err!=GL_NO_ERROR) {
      printf(" Error making colormap texture: %s.\n", gluErrorString(err));
	}

	d_textureMatrixNumX = imageBufferNumX;
	d_textureMatrixNumY = imageBufferNumY;

	glPopAttrib();
	glPopClientAttrib();

	if (imageDataAlpha) delete [] imageDataAlpha;
	imageDataAlpha = NULL;
	return 0;
}

void URProjectiveTexture::loadColorImageMipmap()
{
	int x,y;
	int r=0,g=0,b=0,a=0;
	int i;

	int numTexels = d_textureMatrixNumX * d_textureMatrixNumY;
	// multiply by 4 so we can store 4 values at each texel
	GLubyte * texture = new GLubyte [4 * numTexels];
  
  // Fill the whole texture with black.  This will make a border around
  // any area not filled by the PPM file.
	switch (d_textureBlendFunction) {
	case GL_MODULATE:
		r = 255;
		g = 255;
		b = 255;
		a = 255;
		break;
	case GL_BLEND:
		r = 0;
		g = 0;
		b = 0;
		a = 255;
		break;
	case GL_DECAL:
		a = 0;
		break;
	default:
		r = 255;
		g = 255;
		b = 255;
		a = 255;
		break;
    }
	for (i = 0; i < numTexels; i++) {
		texture[i*4] = r;
		texture[i*4+1] = g;
		texture[i*4+2] = b;
		texture[i*4+3] = a;
	}

	// Fill in the part of the texture that the PPM file covers
	// Invert Y because the coordinate system in the PPM file starts
	// in the upper left corner, and our coordinate system in the lower
	// left.
	if (d_textureBlendFunction == GL_DECAL) {
		a = d_opacity*255;
	} else {
		a = 255;
	}
	int imageNumX = d_colorImage->ny, imageNumY = d_colorImage->ny;
	int texelIndex;
	for (y = imageNumY-1; y >= 0; y--) {
		texelIndex = y*d_textureMatrixNumX;
		for (x = 0; x < imageNumX; x++) {
			d_colorImage->Tellppm(x,y, &r, &g, &b);
			if (d_textureBlendFunction != GL_DECAL) {
				r = (GLubyte)((float)r*d_opacity);
				g = (GLubyte)((float)g*d_opacity);
				b = (GLubyte)((float)b*d_opacity);
			}
			texture[texelIndex*4] = r;
			texture[texelIndex*4+1] = g;
			texture[texelIndex*4+2] = b;
			texture[texelIndex*4+3] = a;
			texelIndex++;
		}
	}

	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, d_textureID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,d_wrapMode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,d_wrapMode);

	glTexParameteri( GL_TEXTURE_2D, 
		GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, 
		GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	// Build the texture map and set the mode for 2D textures
	if (gluBuild2DMipmaps(GL_TEXTURE_2D,4, d_textureMatrixNumX, 
			d_textureMatrixNumY, GL_RGBA, GL_UNSIGNED_BYTE, texture) != 0) {
		printf("Error making mipmaps, using single-level texture instead.\n");
		glTexImage2D(GL_TEXTURE_2D, 0, 4, d_textureMatrixNumX, 
			d_textureMatrixNumY, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		if (glGetError()!=GL_NO_ERROR) {
			printf(" Error making texture.\n");
		}
	}
	glPopClientAttrib();
	glPopAttrib();

	delete [] texture;
}
