#ifdef	_WIN32
#include <io.h>
#endif
#include <fcntl.h> // for open()
#include <malloc.h> // for calloc() and free()
#include <errno.h> // for perror()
#include <time.h> // for time() and ctime()
#include <math.h> // for exp()
// #include <unistd.h>

#include "BCGrid.h"
#include "BCPlane.h"
#include "Topo.h"
#include "BCDebug.h"

#include "readNanoscopeFile.C" // <------------ reading a .C file!

const double STANDARD_DEVIATIONS = 3.0;
extern TopoFile GTF;

int BCGrid::_read_mode = READ_DEVICE;
int BCGrid::_times_invoked = 0;

/******************************************************************************\
@BCGridFill --> Called by constructurs
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 11-2-95 by Russ Taylor
\******************************************************************************/
void
BCGrid::BCGridFill(short num_x, short num_y, 
		       double min_x, double max_x, 
		       double min_y, double max_y,
		       int read_mode, const char** file_names, int num_files)
{
    BCDebug debug("BCGrid::BCGridFill", GRID_CODE);
    int i;

    _num_x = num_x;
    _num_y = num_y;
    _min_x = min_x;
    _max_x = max_x;
    _min_y = min_y;
    _max_y = max_y;
    _derange_x = 1.0; 
    _derange_y = 1.0; 
    _num_planes = 0;

    _head = NULL;

    _detection_sensitivity = 1.0;
    _attenuation_in_z = 1.0;
    _z_max = -1.0e33;; 
    _input_sensitivity = 1.0;
    _z_sensitivity = 1.0;
    _input_1_max = 1.0;
    _input_2_max = 1.0;

    _modified = 1;
    _read_mode = read_mode;

    switch (_read_mode) 
    {
      case READ_STREAM:
      case READ_DEVICE:
      {	
	  addNewPlane("Topography-Forward", "nm", TIMED);
      }
      break;
      case READ_FILE: 
      {	  FILE *infile;

	 //
	 // Read the first file from the list into a grid structure
	 // if there is at least one file to open
	 //
	 if (num_files > 0) {
	  infile = fopen(file_names[0],"r");
	  if (infile == NULL) 
	  {
	      fprintf(stderr,
		 "Error! BCGrid::BCGrid: Could not open input file %s!\n",
			file_names[0]);
	      return;
	  }
  
	  if (readFile(infile,file_names[0]) == -1) 
	  {
	      fprintf(stderr,
		"Error! BCGrid::BCGrid: Could not read grid from %s!\n",
			file_names[0]);
              return;
	  }
	 } // End of reading the first file

	 //
	 // For each of the later grids, read them one at a time into their
	 // own grid structure, then compare the size of the new grid with the
	 // size of the existing grid.  If they match, copy all of the planes
	 // into the first grid, ensuring that all planes have unique names.
	 //
	 for (i = 1; i < num_files; i++) {
	  BCGrid grid(num_x,num_y, min_x,max_x,
		      min_y, max_y, read_mode, file_names[i]);
	  if (!(grid.empty())) {
// 		if ( (grid._num_x != _num_x) ||
// 		     (grid._num_y != _num_y) ||
// 		     (grid._min_x != _min_x) ||
// 		     (grid._max_x != _max_x) ||
// 		     (grid._min_y != _min_y) ||
// 		     (grid._max_y != _max_y) ) {
	    // Only compare the grid sizes, so that we can load
	    // datasets of different scan areas, and re-align them!
		if ( (grid._num_x != _num_x) ||
		     (grid._num_y != _num_y) ) {
			fprintf(stderr,"Error! BCGrid::BCGrid: Grid size mismatch in file %s, ignoring the file\n",file_names[i]);
		} else {
		  BCPlane *nextplane, *newplane;
		  BCString name;
		  for (nextplane = grid.head();
		       nextplane != NULL;
		       nextplane = nextplane->_next) {
			findUniquePlaneName(nextplane->_dataset,&name);
			newplane = addPlaneCopy(nextplane);
			newplane->rename(name);
		  }
		}
	  }
	 }
      }
      break;
      default:
      {
	  perror("BCGrid::BCGrid: Unknown read mode!");
	  return;
      }
  }

  _next=NULL;

  return;
} // ~BCGridFill;

/******************************************************************************\
@BCGrid --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCGrid::BCGrid(short num_x, short num_y, 
		       double min_x, double max_x, 
		       double min_y, double max_y,
		       int read_mode, const char** file_names, int num_files)
{
	BCGridFill(num_x,num_y, min_x,max_x, min_y,max_y, read_mode,
		file_names,num_files);
}

/******************************************************************************\
@BCGrid --> constructor
--------------------------------------------------------------------------------
   description: Calls the more general read routine after modifying the
		file_name parameter into a list format.  This is done for
		backwards compatibility with code that passed only one file
		name in.
        author: Russ Taylor
 last modified: 11-2-95 by Russ Taylor
\******************************************************************************/
BCGrid::BCGrid(short num_x, short num_y, 
		       double min_x, double max_x, 
		       double min_y, double max_y,
		       int read_mode, const char* file_name)
{    
	const char	*files[1];
	BCDebug debug("BCGrid::BCGrid", GRID_CODE);

	files[0] = file_name;
	BCGridFill(num_x,num_y, min_x,max_x, min_y,max_y, read_mode,
		files, 1);

} // BCGrid;


/******************************************************************************\
@BCGrid --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCGrid::BCGrid(short num_x, short num_y, 
		       double min_x, double max_x, 
		       double min_y, double max_y)
{    
    BCDebug debug("BCGrid::BCGrid", GRID_CODE);

    _num_x = num_x;
    _num_y = num_y;
    _min_x = min_x;
    _max_x = max_x;
    _min_y = min_y;
    _max_y = max_y;
    _derange_x = 1.0; 
    _derange_y = 1.0; 
    _num_planes = 0;

    _head = NULL;

    _next = NULL;
    _detection_sensitivity = 1.0;
    _attenuation_in_z = 1.0;
    _z_max = -1.0e33;; 
    _input_sensitivity = 1.0;
    _z_sensitivity = 1.0;
    _input_1_max = 1.0;
    _input_2_max = 1.0;

    _modified = 1;
}


/******************************************************************************\
@~BCGrid --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/ 
BCGrid::~BCGrid()
{   
    BCPlane* current;
    BCPlane* next;

    current = _head;

    while (current != NULL)
    {
	next = current->_next;

	delete(current);
	
	current = next;
    } 
} // ~BCGrid

/******************************************************************************\
@findUniquePlaneName
--------------------------------------------------------------------------------
   description: Finds a plane name that does not match any existing plane
		name already linked to the Grid.  It does so by appending
		numbers (from 2 on up) to the base_name until it finds a
		name not already in the list.  If the base name is unique
		as it stands, then no number is appended.
        author: Russ Taylor
 last modified: 11-2-95 by Russ Taylor
\******************************************************************************/
void
BCGrid::findUniquePlaneName(BCString base_name, BCString *result_name)
{
	int	next_number_to_try = 2;
	char	appendix[10];
	*result_name = base_name;

	while (getPlaneByName(*result_name) != NULL) {
		sprintf(appendix,"%d",next_number_to_try);
		next_number_to_try++;
		*result_name = base_name;
		*result_name += appendix;
	}

} // ~findUniquePlaneName

/******************************************************************************\
@addNewPlane
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane*
BCGrid::addNewPlane(BCString dataset, BCString units, int timed)
{
    BCDebug debug("BCGrid::addNewPlane", GRID_CODE);
    
    if (timed)
    {
	CTimedPlane* plane = new CTimedPlane(dataset, units, _num_x, _num_y);
	addPlane(plane);
	return plane;
    }
    else
    {
	CPlane* plane = new CPlane(dataset, units, _num_x, _num_y);
        addPlane(plane);
	return plane;
    }

} // addNewPlane


/******************************************************************************\
@addPlaneCopy
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane* 
BCGrid::addPlaneCopy(BCPlane* plane)
{
    BCDebug debug("BCGrid::addPlaneCopy", GRID_CODE);
    
    if (plane->_timed)
    {
	CTimedPlane* copy = new CTimedPlane((CTimedPlane*) plane);
        addPlane(copy);
	return copy;
    }
    else
    {
	CPlane* copy = new CPlane((CPlane*) plane);
        addPlane(copy);
	return copy;
    }

} // addPlaneCopy


/******************************************************************************\
@deleteHead
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCGrid::deleteHead()
{
    if (_head == NULL)
	return -1;

    BCPlane* temp = _head->_next;
    delete(_head);
    _head = temp;

    _num_planes--;

    return 0;

} // deleteHead


/******************************************************************************\
@empty
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCGrid::empty()
{
    if (_head == NULL)
	return 1;
    else
	return 0;
} // empty

int BCGrid::empty_list()
{ if ( _next==NULL)
     return 1;
  else
     return 0;
}
/******************************************************************************\
@getPlaneByName
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane* 
BCGrid::getPlaneByName(BCString name)
{
    BCPlane* current = _head;

    while (current != NULL)
    {
	if (current->_dataset == name)
	    return current;
	
	current = current->_next;
    }
    
    return NULL;

} // getPlaneByName



/******************************************************************************\
@decimate
--------------------------------------------------------------------------------
   description:
    usage note: IMPORTANT! This method destroys the list of planes maintained
                by the Grid. Thus any pointers to individual planes in this
		list that the user has obtained by using head() or getPlaneByName
		will no longer be valid!
        author: ?
 last modified: 9-16-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCGrid::decimate(short num_x, short num_y)
{
    BCDebug debug("BCGrid::decimate", GRID_CODE);
    
    double x0 = ((double) _num_x - 1.0) * 0.5 / (double) num_x;
    double y0 = ((double) _num_y - 1.0) * 0.5 / (double) num_y;

    double dx = ((double) _num_x - 1.0) / (double) num_x;
    double dy = ((double) _num_y - 1.0) / (double) num_y;

    short mask_num_x = (int) (STANDARD_DEVIATIONS * dx / 2.0);
    short mask_num_y = (int) (STANDARD_DEVIATIONS * dy / 2.0);
 
    double** mask = makeMask(mask_num_x, mask_num_y);
    
    BCPlane* next = _head;
    BCPlane* current = NULL;
    
    while (next != NULL)
    {
	BCPlane* newPlane;
	
	if (next->_timed)
	    newPlane = new CTimedPlane((CTimedPlane*) next);
	else
	    newPlane = new CPlane((CPlane*) next);

	newPlane->_grid = this;

	double x, y;
        int ox, oy;
        int ix, iy;
	
	// loop over the image, keeping track of the total weight of
        // the mask pixels that have actually fallen on the image
        // (some may lap over the edges) and normalize to conserve
        // energy 
        for (x = x0, ox = 0; ox < num_x; x += dx, ox++)
	{
	    ix = (int) (x + 0.5);
	    
	    for(y = y0, oy = 0; oy < num_y; y += dy, oy++)
            {
		iy = (int) (y + 0.5);
		
		double value = 0.0;
		double weight = 0.0;
		int iix, iiy;
		int mask_x, mask_y;
		
		for (iix = ix - mask_num_x, mask_x = -mask_num_x;
		     mask_x <= mask_num_x;
		     iix++, mask_x++)
		{
		    for (iiy = iy - mask_num_y, mask_y = -mask_num_y;
			 mask_y <= mask_num_y; 
			 iiy++, mask_y++)
		    {
                        if ( ( ((unsigned) iix) < (unsigned) _num_x )
			    &&
			     ( ((unsigned) iiy) < (unsigned)_num_y ) )
			{
			    value += next->_value[iix][iiy] *
				mask[mask_x][mask_y];
			    weight += mask[mask_x][mask_y];
			}
		    }
		}

		newPlane->_value[ox][oy]  = value / weight;
		if (newPlane->_value[ox][oy] > newPlane->_max_value)
		    	newPlane->_max_value = newPlane->_value[ox][oy];
		if (newPlane->_value[ox][oy] < newPlane->_min_value)
		    	newPlane->_min_value = newPlane->_value[ox][oy];
            }
	}

	if (current == NULL)
	    _head = newPlane;
	else
	    current->_next = newPlane;
	newPlane->_next = next->_next;
	current = newPlane;
	delete(next);
	next = current->_next;
    }

    // free the mask
    int mask_x;
    for (mask_x = -mask_num_x; mask_x <= mask_num_x; mask_x++ )
	free(mask[mask_x] - mask_num_x);
    free(mask - mask_num_x);

    // reset various member variables (note that the size of the planes this
    // list maintains has been slightly reduced)
    _max_x -= x0;
    _min_x += x0;
    _max_y -= y0;
    _min_y += y0;
    _num_x = num_x;
    _num_y = num_y;
    _modified = 1;
    
} // decimate


/******************************************************************************\
@writeTextFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with CPlane::writeTextFile) writes
                files readable by BCGrid::readTextFile.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int	
BCGrid::writeTextFile(FILE* file, BCPlane* plane)
{
    // put the header information in the file
	
    if (fprintf(file,"DSAA\n") == EOF) {
	perror("BCGrid::writeTextFile: Could not write first line!");
	return -1;
    }
    if (fprintf(file,"%d %d\n", _num_x, _num_y) == EOF) {
	perror("BCGrid::writeTextFile: Could not write num_x and num_y!");
	return -1;
    }
    if (fprintf(file,"%f %f\n", _min_x, _max_x) == EOF) {
	perror("BCGrid::writeTextFile: Could not write min/max x!");
	return -1;
    }
    if (fprintf(file,"%f %f\n", _min_y, _max_y) == EOF) {
	perror("BCGrid::writeTextFile: Could not write min/max y!");
	return -1;
    }

    return plane->writeTextFile(file);

} // writeTextFile


/******************************************************************************\
@writeBinaryFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::writeBinaryFile) writes
                files readable by BCGrid::readBinaryFile.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int	
BCGrid::writeBinaryFile(FILE* file, BCPlane* plane)
{
    if (fwrite("DSBB", 4, 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write first line!");
	return(-1);
    }
    if (fwrite(&_num_x, sizeof(_num_x), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write num_x!");
	return -1;
    }
    if (fwrite(&_num_y, sizeof(_num_y), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write num_y!");
	return -1;
    }
    if (fwrite(&_min_x, sizeof(_min_x), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write min_x!");
	return -1;
    }
    if (fwrite(&_max_x, sizeof(_max_x), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write max_x!");
	return -1;
    }
    if (fwrite(&_min_y,sizeof(_min_y), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write min_y");
	return -1;
    }
    if (fwrite(&_max_y,sizeof(_max_y), 1, file) != 1) 
    {
	perror("BCGrid::writeBinaryFile: Could not write max_y");
	return -1;
    }

    return plane->writeBinaryFile(file);

} // writeBinaryFile


/******************************************************************************\
@writeUNCAFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::writeUNCAFile) writes
                files readable by BCGrid::readUNCAFile.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int  
BCGrid::writeUNCAFile(FILE* file, BCPlane* plane)
{
    // put the header information in the file 

    if (fprintf(file, "UNCA\n") == EOF) 
    {
	perror("BCGrid::writeUNCAFile: Could not write first line");
	return -1;
    }
    if (fprintf(file, "%d %d\n", _num_x, _num_y) == EOF) 
    {
	perror("BCGrid::writeUNCAFile: Could not write num_x and num_y");
	return -1;
    }
    if (fprintf(file,"%f %f\n",_min_x,_max_x) == EOF) 
    {
	perror("BCGrid::writeUNCAFile: Could not write min/max x");
	return -1;
    }
    if (fprintf(file,"%f %f\n",_min_y,_max_y) == EOF) {
	perror("BCGrid::writeUNCAFile: Could not write min/max y");
	return -1;
    }

    return plane->writeUNCAFile(file);

} // writeUNCAFile


int check_format(char *token)
{ int n;
  int i = 0;
  
  n=strlen(token);

/*Skip all the blanks */

  while(token[i]==' ' || token[i]=='\t')
     i++;

/*Check if the length are contained with all the digits */

  while(isdigit(token[i])!=0 && i<n)
     i++;

  while(token[i]==' ' || token[i]=='\t')
     i++;
 
  if(i!=n)
    return -1;
  
  return 0;

}


int BCGrid::readSPIPFile(FILE* file, const char *name)
{
  // float current,bias,scanspeed;
  //int intelmode=0;
  //  char starttime[60];
  // current=bias=scanspeed=0.0;
  char *token;
  char buffer[80];
  double  max_value = 1; 
  
  rewind(file);  //we read four byte for magic number
  
  fgets(buffer, 70, file);  // first line specifies fileformat 
 
  while( fgets(buffer, 70,file)!= NULL) {
     if ('%' != *buffer) {   // it is comment, ignore this line
        token = strtok(buffer,"=");
        /*if(token == NULL) {
	     fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
	     return -1;
	} */
        if(token == NULL ) 
           break;

	if(strcmp(token,"xpixels ")== 0) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else  _num_x= atoi(token);
	}
        else if(strcmp(token,"ypixels ")==0 ) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else _num_y= atoi(token);
	}	   
	else if( strcmp(token,"xoffset ") == 0) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else _min_x = atof(token);
	} 
	else if (strcmp(token,"yoffset ") == 0) {
	  token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else _min_y= atof(token);
	} 
        else if( strcmp(token,"xlength ") == 0) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else _max_x = atof(token);
	} 
	else if( strcmp(token,"ylength ") == 0) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else _max_y = atof(token);
	}
	else if( strcmp(token,"bit2nm ") == 0) {
	   token = strtok(NULL,"");
           if( !check_format(token)) {
	        fprintf(stderr,"ReadSPIPFile:Unknown file format\n");
		return -1;
	   }
	   else max_value = atof(token) * 32767.0;
        }
    }
  }

    _max_x= _max_x + _min_x;
    _max_y= _max_y + _min_y;

    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    return (plane->readSPIPFile(file,max_value));

}


int BCGrid::writeSPIPFile(FILE* file, BCPlane* plane)
{
  float current,bias,scanspeed;
  int intelmode=0;
  char starttime[]="10 18 96 16:23:22:22";
  current=bias=scanspeed=0.0;

  if(fprintf(file,"fileformat = bcrstm\n")==EOF) {
    perror("BCGrid::writeSPIPFile: Could not write first line\n");
    return -1;
  }
  if(fprintf(file,"xpixels = %d\nypixels = %d\nxlength = %f\nylength = %f\n",
            _num_x,_num_y, _max_x-_min_x, _max_y-_min_y )==EOF)  {
    perror("BCGrid::writeSPIPFile: Could not write xpixels,ypixels\n");
    return -1;
  }
  if(fprintf(file,"current = %f\nbias = %f\n",current,bias)==EOF) {
    perror("BCGrid::writeSPIPFile: Could not write current and bias\n");
    return -1;
  }
  if(fprintf(file,"starttime = %s\nscanspeed = %f\n",starttime,scanspeed)==EOF) {
    perror("BCGrid::writeSPIPFile: Could not write start time\n");
    return -1;
  }	 
  if(fprintf(file,"intelmode = %d\n",intelmode)==EOF){ 
    perror("BCGrid::writeSPIPFile: Could not write intelmode\n");
    return -1;
  }
  if(fprintf(file,"bit2nm = %f\n",
             (plane->maxValue()-plane->minValue())/32767)==EOF) {
    perror("BCGrid::writeSPIPFile: Could not write bit2nm\n");
    return -1;
  }
  if(fprintf(file,"xoffset = %f\nyoffset = %f\n", _min_x,_min_y)==EOF) {
    perror("BCGrid::writeSPIPFile: Could not write bit2nm\n");
    return -1;
  }
    return plane->writeSPIPFile(file);
}
/******************************************************************************\
@writePPMFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCGrid::writePPMFile(FILE* file, BCPlane* plane)
{
    BCDebug debug("BCGrid::writePPMFile", GRID_CODE);

    int file_descriptor;
    if ( (file_descriptor = fileno(file)) == -1) {
      perror("BCGrid::writePPMFile: Could not get descriptor!");
      return -1;
    }
    char header[127];
    time_t t = time(NULL);

    sprintf(header, "P6\n#nM Scale: %g %g %g Create %s%d %d\n255\n",
	    _max_x - _min_x,
	    _max_y - _min_y,
	    plane->maxValue() - plane->minValue(),
	    ctime(&t), _num_y, _num_x);

    if (write(file_descriptor, header, strlen(header)) == -1)
    {
	perror("BCGrid::writePPMFile: Could not write header!");
    	return -1;
    }

    return plane->writePPMFile(file_descriptor);
    
} // writePPMFile


/******************************************************************************\
@writeRawVolFile
--------------------------------------------------------------------------------
   description: Write a raw volume file from the planes in the grid
                Useful for input to the VolVis program from SUNY Stony Brook
        author: Aron Helser
 last modified: Aug 12 1998 Aron Helser
\******************************************************************************/
int 
BCGrid::writeRawVolFile(const char* file_name)
{
    BCDebug debug("BCGrid::writeRawVolFile", GRID_CODE);

    char buffer[127];

    strcpy(buffer, file_name);

    int  file_descriptor = open(file_name, O_WRONLY|O_CREAT|O_EXCL, 0444);
  
    while (file_descriptor < 0 ) // could not open file called file_name - try to open something else
    {
	static int tries = 0;
	perror("BCGrid::writeRawVolFile: Could not open file!");
	fprintf(stderr, "file: %s\n", buffer);
      
	sprintf(buffer, "tmp_%03d.raw", tries++);

	file_descriptor = open(buffer, O_WRONLY|O_CREAT|O_EXCL, 0444);   
    } 
  
    printf("Writing to %s, please wait...\n", buffer);

    // Write data to file. Scale the values from 0 to 255. Write them 
    // in order from the grid. 
    
    int i;
    BCPlane *head = this->head();
    int num_planes = this->numPlanes();
    double min_min, max_max;

    min_min = head->minValue();
    max_max = head ->maxValue();
    head = head->next();
    for ( i = 0; (i < num_planes) && head; i++ ) {
      min_min = min(min_min, head->minValue());
      max_max = max(max_max, head ->maxValue());
      
      head = head->next();
    }


    head = this->head();
    for ( i = 0; (i < num_planes) && head; i++ ) {
      // Write out values scaled to 0 to 255 in order.
      if( head->writeRawByteFile(file_descriptor, min_min, max_max) != 0) return (-1);
      head = head->next();
    }

    close(file_descriptor);
    return 0;
    

    
} // writeRawVolFile


/******************************************************************************\

  The following BCGrid methods below are PRIVATE!
  
\******************************************************************************/

  
/******************************************************************************\
@addPlane
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCGrid::addPlane(BCPlane* plane)
{
    BCString msg = "BCGrid::addPlane(" + plane->_dataset + ")";
    BCDebug debug(msg, GRID_CODE);

    _num_planes++;

    BCPlane* last = NULL;
    BCPlane* current = _head;
    
    while (current!= NULL)
    {
	last = current;
	current = current->_next;
    }
    
    if (last == NULL)
	_head = plane; 
    else 
	last->_next = plane;

    plane->_grid = this;

} // addPlane



/******************************************************************************\
@makeMask
--------------------------------------------------------------------------------
   description: This method makes a Gaussian mask.
         input: num_x and num_y (one-sided extents, not dimensions!)
        author: ?
 last modified: 9-16-95 by Kimberly Passarella Jones
\******************************************************************************/
double**
BCGrid::makeMask(short num_x, short num_y)
{
    BCDebug debug("BCGrid::makeMask", GRID_CODE);

    double  **mask = (double**) calloc(2 * num_x + 1, sizeof(double *)) + num_x;    

    double  x_scale = -(num_x * num_x)/(2 * STANDARD_DEVIATIONS);
    double  y_scale = -(num_y * num_y)/(2 * STANDARD_DEVIATIONS);

    int x, y;
    for (x = -num_x; x <= num_x; x++)
    {
	mask[x] = (double *) calloc(2 * num_y + 1, sizeof(double )) + num_y;
	for (y = -num_y; y <= num_y; y++)
	    mask[x][y] = exp(x * x * x_scale + y * y * y_scale);
    }

    return mask;
    
} // makeMask


/******************************************************************************\
@readFile
--------------------------------------------------------------------------------
   description: This method will read any file type. It first determines what 
                is in the file by looking the first four characters, which
		are kind of a "magic number" for the file.  Then it reads the
 		file contents.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCGrid::readFile(FILE* file, const char *filename)
{
    char magic[5];
    
// parse the header information in the file
    
    if (fread(magic, 4, 1, file) != 1) 
    {
	perror("BCGrid::readFile: Could not get magic number!");
	return -1;
    }

    if (strncmp(magic,"DSBB",4) == 0) 
    {
	return readBinaryFile(file, filename);
    } 
    else if (strncmp(magic,"DSAA",4) == 0) 
    {
	return readTextFile(file, filename);
    } 
    else if (strncmp(magic,"UNCA",4) == 0) 
    {
	return readUNCAFile(file, filename);
    } 
    else if (strncmp(magic,"file",4) == 0) 
    {   
	return readSPIPFile(file, filename);
    } 

    else if (strncmp(magic,"UNCB",4) == 0) 
    {
	return readUNCBFile(file, filename);
    } 
    else if (strncmp(magic,"Data",4) == 0) 
    {
	return readNanoscopeFileWithoutHeader(file, filename);
    } 
    else if (strncmp(magic,"\\*Fi",4) == 0) 
    {
	return readBinaryNanoscopeFile(file, filename); 
    } 
    else if (strncmp(magic,"?*Fi",4) == 0) 
    {
        return readAsciiNanoscopeFile(file, filename);
    } 
    else if (strncmp(magic,"#R3.0",4) == 0) 
    {
        //return readTopometrixFile(file,filename);
        return readTopometrixFile(GTF,filename);
    } 
    else if (strncmp(magic,"#R4.0",4) == 0) 
    {
        //return readTopometrixFile(file, filename);
        return readTopometrixFile(GTF, filename);
    } 
    else if (strncmp(magic,"P6",2) == 0) 
    {
	return readPPMorPGMFile(file, filename);
    } 
    else if (strncmp(magic,"P2",2) == 0) 
    {
	return readPPMorPGMFile(file, filename);
    } 
    else 
    {
	magic[4] = '\0';
	fprintf(stderr,"Error! BCGrid::readFile: Bad magic number (%s)!\n",
		magic);
	return -1;
    }

} // readFile


/******************************************************************************\
@readTextFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::readTextFile) read 
                files with the format given below:

		DSAA 		<-- 4 character ascii
		_num_x _num_y	<-- integers
	        _min_x, _max_x	<-- doubles
		_min_y, _max_y	<-- doubles

                min_value max_value      <-- doubles
                value sec usec           <-- double, long, long

		It also creates a plane (an instance of the BCPlane class) and
		adds it to the lists of planes which begins with _head. 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCGrid::readTextFile(FILE* file, const char *name)
{
    // parse the header information in the file
	
    if (fscanf(file,"%hd %hd", &_num_x, &_num_y) != 2) 
    {
	perror("BCGrid::readTextFile: Could not read num_x and num_y!");
	return -1;
    }
    if (fscanf(file,"%lf %lf", &_min_x, &_max_x) != 2) 
    {
	perror("BCGrid::readTextFile: Could not read min/max x!");
	return -1;
    }
    if (fscanf(file,"%lf %lf", &_min_y, &_max_y) != 2) 
    {
	perror("BCGrid::readTextFile: Could not read min/max y!");
	return -1;
    }

    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    if (plane->readTextFile(file) == -1)
	return -1;

    float scrap;

    if (fscanf(file,"%f",&scrap) == 1) 
	perror("BCGrid::readTextFile: WARNING: Not at file end upon completion!");

    return 0;

} // readTextFile


/******************************************************************************\
@readBinaryFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::readBinaryFile) reads
                files withthe format given below:

		DSBB 		<-- 4 character ascii
		_num_x _num_y	<-- integers
	        _min_x, _max_x	<-- doubles
		_min_y, _max_y	<-- doubles

                min_value max_value      <-- doubles
                value sec usec color     <-- double, long, long, double

		It also creates a plane (an instance of the BCPlane class) and
		adds it to the lists of planes which begins with _head.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int	
BCGrid::readBinaryFile(FILE* file, const char *name)
{
    // parse the header information in the file

    if (fread(&_num_x, sizeof(_num_x), 1, file) != 1) 
    {
	perror("BCGrid::readBinaryFile: Could not read num_x!");
	return -1;
    }
    if (fread(&_num_y, sizeof(_num_y), 1, file) != 1) 
    {
	perror("BCGrid::readBinaryFile: Could not read num_y!");
	return -1;
    }
    if (fread(&_min_x, sizeof(_min_x), 1, file) != 1) 
    {
	perror("BCGrid::readBinaryFile: Could not read min_x!");
	return -1;
    }
    if (fread(&_max_x, sizeof(_max_x), 1, file) != 1) 
    {
	perror("BCGrid::readBinaryFile: Could not read max_x!");
	return -1;
    }
    if (fread(&_min_y, sizeof(_min_y),1,file) != 1)
    {
	perror("BCGrid::readBinaryFile: Could not read min_y!");
	return -1;
    }
    if (fread(&_max_y, sizeof(_max_y),1,file) != 1) 
    {
	perror("BCGrid::readBinaryFile: Could not read max_y!");
	return -1;
    }

    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    if (plane->readBinaryFile(file) == -1)
	return -1;

    float scrap;

    if (fread(&scrap, 1, 1, file) == 1)
	perror("BCGrid::readBinaryFile: WARNING: Not at file end upon completion!");

    return 0;

} // readBinaryFile


/******************************************************************************\
@readUNCAFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::readUNCAFile) read 
                files with the format given below:

		UNCA 		<-- 4 character ascii
		_num_x _num_y	<-- integers
	        _min_x, _max_x	<-- doubles
		_min_y, _max_y	<-- doubles

                min_value max_value      <-- doubles
                value sec usec		 <-- double, long, long

		It also creates a plane (an instance of the BCPlane class) and
		adds it to the lists of planes which begins with _head.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCGrid::readUNCAFile(FILE* file, const char *name)
{
    // parse the header information in the file 

    if (fscanf(file, "%hd %hd", &_num_x, &_num_y) != 2) 
    {
	perror("BCGrid::readUNCAFile: Could not read num_x and num_y");
	return -1;
    }
    if (fscanf(file, "%lf %lf", &_min_x, &_max_x) != 2) 
    {
	perror("BCGrid::readUNCAFile: Could not read min/max x");
	return -1;
    }
    if (fscanf(file, "%lf %lf", &_min_y, &_max_y) != 2) 
    {
	perror("BCGrid::readUNCAFile: Could not read min/max y");
	return -1;
    }
    
    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    if (plane->readUNCAFile(file) == -1)
	return -1;

    float scrap;

    if (fscanf(file,"%f", &scrap) == 1) 
	perror("BCGrid::readUNCAFile: WARNING: Not at file end upon completion!");

    return 0;

} // readUNCAFile


/******************************************************************************\
@readUNCBFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::readUNCBFile) read 
                files with the format given below:

		UNCB		<-- 4 character ascii
		_num_x _num_y	<-- integers
	        _min_x, _max_x	<-- doubles
		_min_y, _max_y	<-- doubles

                min_value max_value      <-- doubles
                value sec usec color     <-- double, long, long, double

		It also creates a plane (an instance of the BCPlane class) and 
		adds it to the lists of planes which begins with _head.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int     
BCGrid::readUNCBFile(FILE* file, const char *name)
{
    // parse the header information in the file

    if (fread(&_num_x,sizeof(_num_x), 1, file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read num_x!");
	return -1;
    }
    if (fread(&_num_y,sizeof(_num_y), 1, file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read num_y!");
	return -1;
    }
    if (fread(&_min_x, sizeof(_min_x), 1, file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read min_x!");
	return -1;
    }
    if (fread(&_max_x, sizeof(_max_x), 1, file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read max_x!");
	return -1;
    }
    if (fread(&_min_y, sizeof(_min_y), 1, file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read min_y!");
	return -1;
    }
    if (fread(&_max_y, sizeof(_max_y),1,file) != 1) 
    {
	perror("BCGrid::readUNCBFile: Could not read max_y");
	return -1;
    }

    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    if (plane->readUNCBFile(file) == -1)
	return -1;

    float scrap;

    if (fread(&scrap, 1, 1, file) == 1)
	perror("BCGrid::readUNCBFile: WARNING: Not at file end upon completion!");

    return 0;

} // readUNCBfile



/******************************************************************************\
@readComment
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int     
BCGrid::readComment(FILE *file, char *buffer, double* max_value) 
{
    double max_x, max_y, max_z;
    int	 values_read;

    BCDebug debug("BCGrid::readPPMComment", GRID_CODE);

    // get the next line of the file 
    fgets(buffer, 70, file);

    debug.watch("buffer", buffer);

    // if it doesn't start with a '#', it's not a comment so return 0
    if ('#' != *buffer) 
	return 0;

    // it is a comment - see if it's our comment
    if ((values_read = sscanf(buffer, "# nM Scale: %lf %lf %lf", &max_x, &max_y, &max_z)) == 3)
    {
	_max_x = max_x;
	_max_y = max_y;
        *max_value = max_z;
    }
    else if (values_read == 1) 
    {
        debug.watch("max z", max_z);
        *max_value = max_z;
    }

    return 1;

} // readComment


/******************************************************************************\
@readPPMorPGMFile
--------------------------------------------------------------------------------
   description: This method (in conjunction with BCPlane::readPPMorPGMFile and
                readComment) reads files with the format given below:

		P2 or P6	<-- 2 character ascii
		_num_x _num_y	<-- integers
                max_color       <-- integer

                value           <-- unsigned char

                (It ignores lines beginning with '#', and only uses the first
                channel (red).)

		It also creates a plane (an instance of the BCPlane class) and adds
		it to the lists of planes which begins with _head.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCGrid::readPPMorPGMFile(FILE *file, const char *name)
{
    BCDebug debug("BCGrid::readPPMFile", GRID_CODE);

    // initialize the scaling parameters.

    _min_x = _min_y = 0.0;
    _max_x = _max_y = 4000.0;

    //  rewind the file pointer (we went too far when getting the
    //  magic number in readFile (4B for a 2B magic #))
    rewind(file);

    debug.warn("file rewound");

    char buffer[80];
    double max_value = 400.0;
    double min_value = 0.0;

    // get magic number
    while (readComment(file, buffer, &max_value));

    // make sure it is "P6"
    char magic[80];
    if (!sscanf(buffer, "%s", magic) || strncmp( magic, "P6", 2 ) ) 
    {
         fprintf(stderr,
		"BCGrid::readPPMorPGMFile: bad magic number(%s)!",buffer);
	 return -1;
    }

    // get grid dimensions (and make certain they are positive)
    while (readComment(file, buffer, &max_value));

    if (!sscanf(buffer, "%hd %hd", &(_num_y), &(_num_x)) || !_num_x || !_num_y)
    {
	fprintf(stderr, "Error! BCGrid::readPPMorPGMFile: bad dimensions %dx%d!\n",
		_num_x, _num_y);
	return -1;
    }

    // get maximum color value.(and make certain it is in the range (0..255] - it
    // will be used to normalize plane values
    while (readComment(file, buffer, &max_value));

    double max_color;
    if (!sscanf(buffer, "%lf", &max_color) || (0.0 >= max_color))
    {
        fprintf(stderr, "Error! BCGrid::readPPMorPGMFile: bad color %lf!\n",
                max_color);
        return -1;
    }

    BCPlane* plane = addNewPlane(name, "nm", TIMED);

    plane->_max_value = max_value;
    plane->_min_value = min_value;

    double scale = max_value/max_color;

    if (plane->readPPMorPGMFile(file, scale) == -1)
	return -1;

    return 0;
    
}  // readPPMorPGMFile


ostream& operator << (ostream& os, BCGrid* grid)
{
    os << "***************************************" << endl;

    os << "Contents of this instance of BCGrid:" << endl << endl;

    BCPlane* current = grid->_head;

    cout << grid->_num_planes << " grids: " << endl;

    while (current != NULL)
    {
        os << current->_dataset << endl;

	current  = current->_next;
    }

    os << "***************************************" << endl;
    
    return os;
}
