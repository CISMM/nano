#ifndef NMG_STATE_GUARDIAN
#define NMG_STATE_GUARDIAN

#ifdef _WIN32
#include        <windows.h>  // This must be included before <GL/gl.h>
#endif
#include	<GL/gl.h>

#include <vrpn_Types.h>

class nmg_StateGuardian
{
public:
	nmg_StateGuardian();
	~nmg_StateGuardian();

	void setLightModelAmbient(GLfloat ambient[4]);
	void setBlendingEnabled(vrpn_bool enabled);
	void setBlendingFunc(GLenum source, GLenum destination);
	void setColorMaterialEnabled(vrpn_bool enabled);
	void setColorMaterial(GLenum face, GLenum color_mode);
	void setMaterial(GLenum face, GLenum color_mode, GLfloat color[4]);
	void setMaterialShininess(GLenum face, GLfloat shininess);

	void set1DTextureEnabled(vrpn_bool enabled);
	void set2DTextureEnabled(vrpn_bool enabled);
	void set3DTextureEnabled(vrpn_bool enabled);

private:
	GLfloat d_lightModelAmbient[4];
	vrpn_bool d_blendingEnabled;
	vrpn_bool d_colorMaterialEnabled;
	GLenum blending_source, blending_destination;
	GLenum d_materialColorFace, d_materialColorMode;
	GLenum d_materialFace;
	GLfloat d_materialAmbient[4];
	GLfloat d_materialDiffuse[4];
	GLfloat d_materialSpecular[4];
	GLfloat d_shininess;

	vrpn_bool d_1dTextureEnabled;
	vrpn_bool d_2dTextureEnabled;
#ifdef	sgi
	vrpn_bool d_3dTextureEnabled;
#endif
};

#endif
