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
    
    // set tube yaw angle (in-plane rotation angle)
    //glRotatef(d_yaw, 0.0, 0.0, 1.0 ); 
    
    // set roll angle around tube axis
    //glRotatef(d_pitch,  0.0, 1.0, 0.0 );
    
    // set roll angle around tube axis
    //glRotatef(d_roll,  1.0, 0.0, 0.0 );
    
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
    double D1;
    float pitch, yaw, roll;
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
        D1 = sqrt(x*x + z*z);
        pitch = (float)asin(z / D1);


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

