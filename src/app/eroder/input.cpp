/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */

/* Input to the simulator */

#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut_UNC.h>
#include "Vec3d.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "input.h"
#include "main.h"


/* First the tip */
/* Here are our AFM tips */
// third arg is the default
// all units in nm
SphereTip sp(5.0);//initial radius value is 5.0 for sp and ics
InvConeSphereTip ics(5.0,1000.,DEG_TO_RAD*20.,tesselation);
Tip tip(&sp,&ics,tesselation,INV_CONE_SPHERE_TIP);
//Tip tip(sp,ics,tesselation,SPHERE_TIP);

