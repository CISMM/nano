#include <stdlib.h>
#include <string.h>  // for strlen(), strcpy()
#include <ctype.h>

#include <tcl.h>
#include <tk.h>
#include <Tcl_Interpreter.h>

int showgraph (float [], int, int, float, float, const char *);

/*
 * Command-line options:
 */

int showgraph (float array [], int wi, int hi,
               float xscale, float yscale, float yoffset,
               const char * tcl_script_dir)
{
  char var[20];
  char val[20];
  int i;
  char scriptname[1000];

  Tcl_Interp *interp = Tcl_Interpreter::getInterpreter();
  
  sprintf(val,"%d",wi);
  Tcl_SetVar(interp,"wi",val,0);
  sprintf(val,"%d",hi);
  Tcl_SetVar(interp,"hi",val,0);
  sprintf(val,"%g",xscale);
  Tcl_SetVar(interp,"xscale",val,0);
  sprintf(val,"%g",yscale);
  Tcl_SetVar(interp,"yscale",val,0);
  sprintf(val,"%g",yoffset);
  Tcl_SetVar(interp,"yoffset",val,0);
  for(i=0;i<wi;i++)
    {
      sprintf(var,"points(%d)",i);
      sprintf(val,"%g",array[i]);
      Tcl_SetVar(interp,var,val,0);
    }

  sprintf(scriptname,"%s%s",tcl_script_dir,"cross_section.tcl"); 
  if(Tcl_EvalFile(interp,scriptname)!=TCL_OK)
    {
      printf("Can not draw the graph due to %s\n",interp->result);
      //Tcl_Eval(interp,"destroy .");
    }

  return(0);
}

int Tcl_AppInit(Tcl_Interp *)
{
  
  return(0);
}
