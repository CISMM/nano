#include "nmtc_TipConvolution.h"
#include "nmr_Util.h"
#include <microscape.h> // for disableOtherTextures

nmtc_TipConvolution::nmtc_TipConvolution(nmg_Graphics *g, nmb_ImageList *im):

  d_convolutionImageData("data_comes_from", "none"),
  d_convolutionTipName("tip_comes_from", "none"),
  d_resultImageName("result_image_name" ,"none"),
  d_graphicsDisplay(g),
  d_imageList(im)

{

  d_resultImageName.addCallback
    (handle_resultImageName_change, (void *)this);
  d_convolutionImageData.addCallback
    (handle_convolutionImageData_change, (void *)this);
  d_convolutionTipName.addCallback
    (handle_convolutionTipName_change, (void *)this);
}

nmtc_TipConvolution::~nmtc_TipConvolution()
{
}

void nmtc_TipConvolution::handle_convolutionImageData_change(const char *name, void *ud)
{
  nmtc_TipConvolution *me = (nmtc_TipConvolution *)ud;
  nmb_Image *im = me->d_imageList->getImageByName(name);
  if(!im) 
    {
      fprintf(stderr, "image not found: %s\n", name);
      return;
    }
}

void nmtc_TipConvolution::handle_convolutionTipName_change(const char *name, void *ud)
{
  nmtc_TipConvolution *me = (nmtc_TipConvolution *)ud;
  nmb_Image *im = me->d_imageList->getImageByName(name);
  if(!im)
    {
      fprintf(stderr, "image not found: %s\n", name);
      return;
    }
}

void nmtc_TipConvolution::handle_resultImageName_change(const char *name, void *ud)
{
  nmtc_TipConvolution *me = (nmtc_TipConvolution *)ud;
  me->CreateConvolutionImage(name);
}


void nmtc_TipConvolution::CreateConvolutionImage(const char *imageName)
{   
   nmb_Image *Tip = d_imageList->getImageByName
              (d_convolutionTipName.string());
   nmb_Image *Surface = d_imageList->getImageByName
              (d_convolutionImageData.string());
   nmb_Image *Boundary;
   long i,j,k,l;
   int initx, inity;
   int tipcentx, tipcenty;  
   long distance,x,y;
   long min, m, n;
   imageName = imageName; // making the compiler happy

   // To make sure we have all the information we need and to see if
   // the user has given a name to to the output image
  
   if((!Surface) ||(!Tip) || (strlen(d_resultImageName.string())) == 0) {
     return;
   }
   
   //Creating a new image called Boundary
   Boundary = new nmb_ImageGrid(
		(const char *)(d_resultImageName.string()),
                (const char *)(Surface->unitsValue()),
                Surface->width(), Surface->height());
 

  //Initializing Boundary image ti infinity
   for (initx = 0; initx < (Surface->width()); initx++){
     for (inity = 0; inity < (Surface->height()); inity++){
       Boundary->setValue(initx, inity, 2147000000);
     }
   }

 
  
   //Finding the center of the tip
   if(Tip->width()% 2 == 0) {  
     tipcentx = (Tip->width())/2 -1;
     tipcenty = (Tip->height())/2-1;
   } else
     {
       tipcentx = (Tip->width())/2;
       tipcenty = (Tip->height())/2;
     }
   
   //Loop over all the points in the Surface image
   for (i=0; i < (Surface->width());i++){
     for (j=0; j < (Surface->height());j++){
    
     
       // Find the amount by which we have to move the center of the tip
       // up or down to line up so that when you subtract the current Tip
       // part from it, it comes back to the surface.  This is basically
       // shifting the Tip up so that its zero value lines up with the
       // current surface location.
       
       distance = (long)(Surface->getValue(i,j) + Tip->getValue(tipcentx,tipcenty));
       
       //Looping over all the points in the tip at each Surface point.  
     
       for(k=0; k < Tip->width(); k++){
	 for(l=0; l < Tip->height(); l++){
	   
	   // Finding the new cordinates on the surface
	   x= i+k-tipcentx;
	   y= j+l-tipcenty; 
	   
	   // If the Tip point is out of the range of the surface, 
	   // then throw it out 
	   // Figure out the height that this point on the tip will carve into
	   // the surface when the center of the tip is at the height specified
	   // by distance.
	   // Make it so that the Tip is inverted: higher values point down
	   // and lower values are above (the highest point on the tip is the
	   // one that carves out the most).  The center of the Tip should end
	   // up carving to the same height as the Surface was.
	   
	   
	   if( (x >= 0 && x < Surface->width()) && 
	       (y >=0 && y<Surface->height())) {          
	     // Beacause we are using simulated tips created in Photoshop to 
	     // test this program, the tip image was using the plane around the 
	     // actual tip to do most of the carving.  Therefore, to eliminate 
	     // that effect, if the value of tip is 0, do not carve.  
	     // This is will allow the carving to be done actully 
	     // by the tip in the plane.
	     if (Tip->getValue(k,l) != 0){
	       double carved_z = double(distance - (long)Tip->getValue(k,l));
           
	       // If the location we are at has been carved lower than the
	       // carving distance already, then we leave it alone.  Otherwise,
	       // carve down there.
	       if (carved_z < (Boundary->getValue(x,y)))
		 Boundary->setValue(x,y, carved_z);
             }
	   }
         } 
       }            
     }
   
   }
   min = 0;
   //   average = 0;
   for (m = 0; m < Surface->width(); m++){
     for(n=0; n < Surface->height(); n++){
       min = min + abs(Surface->getValue(m,n) + Boundary->getValue(m,n))/2;
     }
   }
   //   average = min / Surface->width();
   
   nmb_ImageBounds Surface_bounds;
   Surface->getBounds(Surface_bounds);
   Boundary->setBounds(Surface_bounds);
   TopoFile tf;
   Surface->getTopoFileInfo(tf);
   Boundary->setTopoFileInfo(tf);
   
   d_imageList->addImage(Boundary);
 
} 


