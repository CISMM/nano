#include "nmg_StateGuardian.h"

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::Constructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_StateGuardian::
nmg_StateGuardian()
{
	//Set all the internal variables to GL's initial state
	d_lightModelAmbient[0] = 0.2f; d_lightModelAmbient[1] = 0.2f;
	d_lightModelAmbient[2] = 0.2f; d_lightModelAmbient[3] = 1;

	d_1dTextureEnabled = VRPN_FALSE;
        d_2dTextureEnabled = VRPN_FALSE;
#ifdef  sgi
        d_3dTextureEnabled = VRPN_FALSE;
#endif

	d_blendingEnabled = VRPN_FALSE;
	d_colorMaterialEnabled = VRPN_FALSE;
	blending_source = GL_ZERO;
	blending_destination = GL_ONE;
	d_materialColorFace = GL_FRONT_AND_BACK;
	d_materialColorMode = GL_AMBIENT_AND_DIFFUSE;
	d_materialFace = GL_FRONT;

	d_materialAmbient[0] = 0.2f; d_materialAmbient[1] = 0.2f;
	d_materialAmbient[2] = 0.2f; d_materialAmbient[3] = 1;

	d_materialDiffuse[0] = 0.8f; d_materialDiffuse[1] = 0.8f;
	d_materialDiffuse[2] = 0.8f; d_materialDiffuse[3] = 1;

	d_materialSpecular[0] = 0; d_materialSpecular[1] = 0;
	d_materialSpecular[2] = 0; d_materialSpecular[3] = 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::Destructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_StateGuardian::
~nmg_StateGuardian()
{
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setLightModelAmbient
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setLightModelAmbient(GLfloat ambient[4])
{
	if (d_lightModelAmbient[0] != ambient[0] || d_lightModelAmbient[1] != ambient[1] ||
		d_lightModelAmbient[2] != ambient[2] || d_lightModelAmbient[3] != ambient[3]) {
		
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		d_lightModelAmbient[0] = ambient[0]; d_lightModelAmbient[1] = ambient[1];
		d_lightModelAmbient[2] = ambient[2]; d_lightModelAmbient[3] = ambient[3];
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setBlendingEnabled
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setBlendingEnabled(vrpn_bool enabled)
{
	if (d_blendingEnabled != enabled) {
		d_blendingEnabled = enabled;
		if (d_blendingEnabled) {
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setBlendingFunc
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setBlendingFunc(GLenum source, GLenum destination)
{
	if (blending_source != source || blending_destination != destination) {
		blending_source = source;
		blending_destination = destination;
		glBlendFunc(source, destination);
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setColorMaterialEnabled
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setColorMaterialEnabled(vrpn_bool enabled)
{
	if (d_colorMaterialEnabled != enabled) {
		d_colorMaterialEnabled = enabled;
		if (d_colorMaterialEnabled) {
			glEnable(GL_COLOR_MATERIAL);
		}
		else {
			glDisable(GL_COLOR_MATERIAL);
		}
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setColorMaterial
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setColorMaterial(GLenum face, GLenum color_mode)
{
	if (d_materialColorFace != face || d_materialColorMode != color_mode) {
		d_materialColorFace = face;
		d_materialColorMode = color_mode;
		glColorMaterial(face, color_mode);
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setMaterial
//      Access: Public
// Description: This isn't right, but I don't have a lot of time...
//				The reason it is wrong, is that if you change
//				the back face, then front face, then repeat
//				the process with the same values, this will 
//				keep re-issuing.  Really need seperate variables
//				for front and back face.  But should be okay
//				with what we currently do.
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setMaterial(GLenum face, GLenum color_mode, GLfloat color[4])
{
	bool face_equal = d_materialFace != face;
	bool ambient_equal = d_materialAmbient[0] != color[0] ||
						 d_materialAmbient[1] != color[1] || 
						 d_materialAmbient[2] != color[2] ||
						 d_materialAmbient[3] != color[3];
	bool diffuse_equal = d_materialDiffuse[0] != color[0] ||
						 d_materialDiffuse[1] != color[1] || 
						 d_materialDiffuse[2] != color[2] ||
						 d_materialDiffuse[3] != color[3];
	bool specular_equal = d_materialSpecular[0] != color[0] ||
						 d_materialSpecular[1] != color[1] || 
						 d_materialSpecular[2] != color[2] ||
						 d_materialSpecular[3] != color[3];
	if (color_mode == GL_AMBIENT_AND_DIFFUSE) {
		if (!face_equal || !ambient_equal || !diffuse_equal) {
			d_materialFace = face;
			d_materialAmbient[0] = color[0];
			d_materialAmbient[1] = color[1];
			d_materialAmbient[2] = color[2];
			d_materialAmbient[3] = color[3];

			d_materialDiffuse[0] = color[0];
			d_materialDiffuse[1] = color[1];
			d_materialDiffuse[2] = color[2];
			d_materialDiffuse[3] = color[3];

			glMaterialfv(face, color_mode, color);

		}		
		return;
	}

	if (color_mode == GL_AMBIENT) {
		if (!face_equal || !ambient_equal) {
			d_materialFace = face;
			d_materialAmbient[0] = color[0];
			d_materialAmbient[1] = color[1];
			d_materialAmbient[2] = color[2];
			d_materialAmbient[3] = color[3];

			glMaterialfv(face, color_mode, color);
		}		
		return;
	}

	if (color_mode == GL_DIFFUSE) {
		if (!face_equal || !diffuse_equal) {
			d_materialFace = face;
			d_materialDiffuse[0] = color[0];
			d_materialDiffuse[1] = color[1];
			d_materialDiffuse[2] = color[2];
			d_materialDiffuse[3] = color[3];

			glMaterialfv(face, color_mode, color);
		}		
		return;
	}

	if (color_mode == GL_SPECULAR) {
		if (!face_equal || !specular_equal) {
			d_materialFace = face;
			d_materialSpecular[0] = color[0];
			d_materialSpecular[1] = color[1];
			d_materialSpecular[2] = color[2];
			d_materialSpecular[3] = color[3];

			glMaterialfv(face, color_mode, color);
		}		
		return;
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::setMaterialShininess
//      Access: Public
// Description: See above
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
setMaterialShininess(GLenum face, GLfloat shininess)
{
	if (d_materialFace != face || d_shininess != shininess) {
		d_materialFace = face;
		d_shininess = shininess;
		glMaterialf(face, GL_SHININESS, shininess);
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::set1DTextureEnabled
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
set1DTextureEnabled(vrpn_bool enabled)
{
	if (d_1dTextureEnabled != enabled) {
		d_1dTextureEnabled = enabled;
		if (d_1dTextureEnabled) {
			glEnable(GL_TEXTURE_1D);
		}
		else {
			glDisable(GL_TEXTURE_1D);
		}
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::set1DTextureEnabled
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
set2DTextureEnabled(vrpn_bool enabled)
{
	if (d_2dTextureEnabled != enabled) {
		d_2dTextureEnabled = enabled;
		if (d_2dTextureEnabled) {
			glEnable(GL_TEXTURE_2D);
		}
		else {
			glDisable(GL_TEXTURE_2D);
		}
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_StateGuardian::set1DTextureEnabled
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_StateGuardian::
set3DTextureEnabled(vrpn_bool enabled)
{
#ifdef	sgi
	if (d_3dTextureEnabled != enabled) {
		d_3dTextureEnabled = enabled;
		if (d_3dTextureEnabled) {
			glEnable(GL_TEXTURE_3D_EXT);
		}
		else {
			glDisable(GL_TEXTURE_3D_EXT);
		}
	}
#endif
}
