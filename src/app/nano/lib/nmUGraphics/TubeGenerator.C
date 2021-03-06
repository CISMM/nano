/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "TubeGenerator.h"

TubeGenerator::TubeGenerator()
    :   d_tesselation(40), d_stride(10)
{
}

void TubeGenerator::SetPoints(q_vec_type *points, int num)
{
    if (d_tube_points) {
        delete [] d_tube_points;
    }

    d_num_points = num;
    d_tube_points = new q_vec_type[num];
    for(int i = 0; i < num; i++) {
        d_tube_points[i][0] = points[i][0];
        d_tube_points[i][1] = points[i][1];
        d_tube_points[i][2] = points[i][2];
    }
}


void TubeGenerator::SetTesselation(int tess)
{
    d_tesselation = tess;
}

void TubeGenerator::SetStride(int stride)
{
    d_stride = stride;
}

void TubeGenerator::drawSphere( double diameter)
{
    GLUquadricObj* qobj;
    
    qobj = gluNewQuadric();
    gluQuadricDrawStyle( qobj, GLU_FILL);
    gluQuadricNormals( qobj, GLU_FLAT );
    
    gluSphere( qobj, diameter/2.0f, d_tesselation, d_tesselation);
}   

void TubeGenerator::drawCylinder( double diameter, double height)
{
    GLUquadricObj* qobj;
    
    qobj = gluNewQuadric();
    gluQuadricDrawStyle( qobj, GLU_FILL);
    gluQuadricNormals( qobj, GLU_FLAT );
    
    gluCylinder( qobj, diameter/2.0f, diameter/2.0f, height, d_tesselation,
        d_tesselation);
}

void TubeGenerator::drawTube(q_vec_type center, double diameter, double length)
{
    glPushMatrix();
    glTranslatef((float)center[0], (float)center[0], (float)center[0]);
    
    // draw cylinder with its axis parallel to X-axis
    glPushMatrix();
    /* we now have to align the cylinder with the Z-axis
     * if the tube did not have any translation and rotation
     * we want, the tube to be along the X-axis. GL on the other
     * hand draws a cylinder along the Z-axis by default. And so
     * we need to do the following rotate 
     */
    glRotatef(90, 0.0, 1.0, 0.0 );
    /* The position of the tube is given by its centre on the
     * axis. We want to draw the cylinder starting from its base
     * Get to its base
     */
    glTranslatef( 0.0, 0.0, - (float)length/2.0f );  
    drawCylinder( diameter, length );      // tube axis starts parallel to Z
    glTranslatef( - (float)length/2.0f, 0., 0 );  
    glPopMatrix();
    
    
    // draw spherical endcap on tube
    glPushMatrix();
    glTranslatef( (float)length/2.0f, 0., 0. );
    drawSphere(diameter); 
    glPopMatrix();
    
    // draw spherical endcap on tube's other end
    glPushMatrix();
    glTranslatef( - (float)length/2.0f, 0., 0. );
    drawSphere(diameter); 
    glPopMatrix();
    
    glPopMatrix();
}

int TubeGenerator::Load(URender *, GLuint *&Dlist_array)
{
    q_vec_type center;
    double x,y,z, length;
    int num_objects = (d_num_points / d_stride) + (d_num_points % d_stride != 0);
    Dlist_array = new GLuint[num_objects];
    int base = glGenLists(num_objects);
    if (base == 0) {
        fprintf(stderr, "Unable to generate display lists in TubeGenerator\n");
    }
    int count = 0;
    for(int i = 0; i < d_num_points; i+=d_stride) {        
        center[0] = 0.5 * (d_tube_points[i][0] + d_tube_points[i+d_stride][0]);
        center[1] = 0.5 * (d_tube_points[i][1] + d_tube_points[i+d_stride][1]);
        center[2] = 0.5 * (d_tube_points[i][2] + d_tube_points[i+d_stride][2]);
        x = d_tube_points[i+d_stride][0] - d_tube_points[i][0];
        y = d_tube_points[i+d_stride][1] - d_tube_points[i][1];
        z = d_tube_points[i+d_stride][2] - d_tube_points[i][2];
        length = sqrt(x*x + y*y + z*z);

        Dlist_array[count++] = base;
        glNewList(base++, GL_COMPILE);
        drawTube(center, d_overall_diameter, length);
        glEndList();
    }

    return num_objects;
}

int TubeGenerator::ReLoad(URender *node, GLuint *&Dlist_array)
{
    return Load(node, Dlist_array);
}

