#ifndef NMTC_TIPCONVOLUTION_H
#define NMTC_TIPCONVOLUTION_H
#include "nmg_Graphics.h"
#include "nmb_Image.h"
#include "nmb_String.h"
#include "nmr_Registration_Proxy.h"
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"

class nmtc_TipConvolution {
 public:
  nmtc_TipConvolution(nmg_Graphics *g, nmb_ImageList *im);
  ~nmtc_TipConvolution();
  static void handle_resultImageName_change(const char *name, void *ud);
  static void handle_convolutionImageData_change(const char *name, void *ud);
  static void handle_convolutionTipName_change(const char *name, void *ud); 

  void CreateConvolutionImage(const char *imageName);
  

 protected:

  Tclvar_string d_convolutionImageData;
  Tclvar_string d_convolutionTipName;
  // Tclvar_string d_convolutionEnabled;
  Tclvar_string d_resultImageName;

  nmg_Graphics *d_graphicsDisplay;
  nmb_ImageList *d_imageList;

};
#endif








