#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#ifdef __CYGWIN__
// [juliano 9/19/99]
//   added these decl so can compile BCPlane.C on my PC in cygwin.
#include <sys/types.h>  // for off_t
extern "C" {
off_t lseek( int filedes, off_t offset, int whence );
int read( int fildes, void *buf, size_t nbyte );
int write( int fildes, const void *buf, size_t nbyte );
}
#endif
#else
#include <unistd.h>
#endif
#include "Topo.h"
#include "nmb_Image.h"

/**
* Assumes the .release is filled in.  Looks for #Rn.n# OFFSET
* Fills in .verNumber with n.nn (2.30 = 230, 3.00 = 300, etc)
* OFFSET is always parsed. This is the fixed offset
* provided in this file.  This will be fixed for each release.
* v3.00:        offset = 1024 bytes
* v3.01:        offset = 1536 bytes
* v4.0:         offset = 2112 bytes
* v5.0:         offset = 3144 bytes
*/
int TopoFile::parseReleaseString( const char *release, vrpn_int32 * verNumber, vrpn_int32 *nOffset)
{       
    char    rel1[N_RELEASE+1];
    int    ver = 0, ver1 = 0, offset= 0;

    if (release == NULL) return -1;
    memcpy( rel1, release, N_RELEASE);          // copy of release string
    rel1[N_RELEASE] = '\0';              // Make sure string is terminated. 

    offset    = 0;
    if (sscanf(release, "#%*[Rr]%d.%d# %d", &ver, &ver1, &offset)!= 3) {
	fprintf(stderr, "TopoFile::parseReleaseString: Didn't scan whole release string!\n");
	return -1;
    }
    if( ver1 < 0 || ver1 > 99) {
        // invalid minor version!
	fprintf(stderr, "TopoFile::parseReleaseString: invalid minor version!\n");
	return -1;
}
    ver = ver * 100 + ver1;                     // our version

    if( offset < 0) offset = 0;
    *verNumber = (vrpn_int32)ver;
    *nOffset  = (vrpn_int32)offset;

//      fprintf(stderr, "TopoFile::parseReleaseString:  "
//  	    "Got release %d, offset %d.\n", ver, offset);
    return 0; 
} // end of parseReleaseString

// ParseHeader (int)
// called by readTopoFile (const char *)
// calls parseReleaseString (char *)

int TopoFile::parseHeader(int in_descriptor){
	// text portion of header is 256 bytes long.
	char     s[N_RELEASE + 1];
	vrpn_int32 verNumber, nOffset;

	printf("TopoFile::ParseHeader: Parsing data file\n");
        if(lseek(in_descriptor, 0, SEEK_SET) == -1) {   /* Error */
                fprintf(stderr,"TopoHeader::ParseHeader(): Can't seek to zero");
                return -1;
        }

	/* get release string */
	// N_RELEASE is max size of release string.
	if (read(in_descriptor,s,N_RELEASE*sizeof(char)) != N_RELEASE*sizeof(char)) {
		fprintf(stderr,"ParseHeader: Error during release string read\n");
		return -1;
	}	
	// tells us how big the header is
	if (parseReleaseString(s, &verNumber, &nOffset) < 0) {
	    fprintf(stderr,"TopoHeader::ParseHeader(): Can't get version number");
	    return -1;
    	}

	// allocate a header of the right size.
	header = new char[nOffset];
	if(header==NULL){ fprintf(stderr,"Unable to allocate room for header\n"); exit(-1);}
        iHeaderLength = nOffset;

	// Go back to the beginning of the file.
        if(lseek(in_descriptor, 0, SEEK_SET) == -1) {   /* Error */
                fprintf(stderr,"TopoHeader::ParseHeader(): Can't seek to zero");
                return -1;
        }
	// read the full header. 
	if (read(in_descriptor,header,sizeof(char)*nOffset) != (signed)(nOffset*sizeof(char))) {
		fprintf(stderr,"ParseHeader: Error during read\n");
		return -1;
	}	
	parseHeader(header, nOffset);
	return 1;
}

/** called by ParseHeader(int) and by nmm_MicroscopeRemote when
 getting header over the network.
*/
int TopoFile::parseHeader(const char *buf, vrpn_int32 length){
	const char * temp;
	vrpn_int32 verNumber, nOffset;

	// header only valid if we get to the bottom without an error. 
	this->valid_header=0;

	// Header could either come from a network message or a file.
	if(header != buf){
	    // header from the network (I think)
		if (header != NULL) delete [] header;
		header = new char [length];
		if(header == NULL) {
			fprintf(stderr,"Unable to allocate memory for header "
			       "buffer.  Aborting.\n");
	                return -1;
		}
                iHeaderLength = length;
		memcpy(header, buf, length);
	}

	temp = header;
	// Yes, we are doing this twice if header read from a file,
	// but only once if read from the network.
	if (parseReleaseString(header, &verNumber, &nOffset) < 0) {
	    return -1;
	}
	if (verNumber <= 300){
	    fprintf(stderr,"Topo.C::parseHeader:Version is less than 3.01 -"
		    " Sorry, can't read it.\n");
	    return -1;
	}
	if (length != nOffset) {
	    fprintf(stderr, "Topo.C:: WARNING - actual length of header"
		    " and length stored in header are different!\n"
	     "   Topo file header probably corrupt, continuing anyway.\n");
	}
	temp = header + 256;
	iRelease = verNumber;
	iOffset = nOffset;
	if ( getGridInfoFromHeader(temp) < 0 ) {
	    return -1;
	}
	this->valid_header = 1;
	printGridInfoFromHeader();
	return(0);	
}


#if (defined(linux) || defined(__CYGWIN__) || defined(_WIN32))
#define DONT_SWAP_BYTES
#endif

static vrpn_int32 iSwap (const char * i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_int32	out;
	out = * (vrpn_int32 *) i;
#else
	static vrpn_int32 out;
	char *o;

	o = (char*)(&out);
	*o = *(i+3);
	*(o+1) = *(i+2);
	*(o+2) = *(i+1);
	*(o+3) = *i;
#endif
	return(out);
}
static vrpn_int32 iSwap (vrpn_int32 i) { return (iSwap((const char *) &i)); }

/* Unused
static vrpn_uint32 uiSwap (const char * i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_uint32	out;
	out = * (vrpn_uint32 *) i;
#else
	static vrpn_uint32 out;
	char *o;

	o = (char*)(&out);
	*o = *(i+3);
	*(o+1) = *(i+2);
	*(o+2) = *(i+1);
	*(o+3) = *i;
#endif
	return(out);
}

static vrpn_float64 dSwap(char *i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_float64 out;
	out = * (vrpn_float64 *) i;
#else
	static	vrpn_float64 out;
	char *o;

      	o = (char*)(&out);
	*o = *(i+7);
	*(o+1) = *(i+6);
	*(o+2) = *(i+5);
	*(o+3) = *(i+4);
	*(o+4) = *(i+3);
	*(o+5) = *(i+2);
	*(o+6) = *(i+1);
	*(o+7) = *(i);
#endif
	return(out);
}
*/
static vrpn_float64 dSwap(const char *i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_float64 out;
	out = * (vrpn_float64 *) i;
#else
	static	vrpn_float64 out;
	char *o;

      	o = (char*)(&out);
	*o = *(i+7);
	*(o+1) = *(i+6);
	*(o+2) = *(i+5);
	*(o+3) = *(i+4);
	*(o+4) = *(i+3);
	*(o+5) = *(i+2);
	*(o+6) = *(i+1);
	*(o+7) = *(i);
#endif
	return(out);
}
static vrpn_float64 dSwap(vrpn_float64 i) { 
    return (dSwap((const char *) &i)); 
}

// Topometrix has a #define float double (in topospm.h),
// so everything defined as a float is REALLY a double.
// However, we may have written out files with floats instead
// of doubles in the past, so we need to be flexible.

static vrpn_float32	fSwap (const char * i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_float32	out;
	out = * (vrpn_float32 *) i;
#else
	static	vrpn_float32  out;
	char *o;

      	o = (char*)(&out);
	*o = *(i+3);
	*(o+1) = *(i+2);
	*(o+2) = *(i+1);
	*(o+3) = *i;
#endif
	return(out);
}

static vrpn_float32 fSwap(vrpn_float32 i) {
    return( fSwap((const char *) &i));
}

static short	sSwap (const char* i)
{
#ifdef	DONT_SWAP_BYTES
	short	out;
	out = * (short *) i;
#else
	static	short	out;
	char	*o;
	
	o = (char*)(&out);
	*o = *(i+1);
	*(o+1) = *i;
#endif
	return(out);
}
static vrpn_int16 sSwap(vrpn_int16 i) { 
    return (sSwap((const char *) &i)); 
}

static vrpn_uint16   usFix (vrpn_uint16 * s)
{
#ifdef  DONT_SWAP_BYTES
        return * s;
#else
        static  vrpn_uint16   out;
        char    * i, * o;

        i = (char *) (s);
        o = (char *) (&out);
        *o = *(i + 1);
        *(o + 1) = *i;

        return(out);
#endif
}


/* Unused
static vrpn_uint16 usSwap (const char *i)
{
#ifdef	DONT_SWAP_BYTES
	vrpn_uint16	out;
	out = * (vrpn_uint16 *) i;
#else
	static vrpn_uint16 out;
	char   *o;

	o = (char*)(&out);
	*o = *(i+1);
	*(o+1) = *i;
#endif
	return(out);
}
*/

#if 0
// Use these functions to read successive types out
// of a char buffer.
// Problem with Topo file - structure alignment issues
// mean types are not necessarily continuous in memory!
// So, we aren't using these right now.
static int iSwapAdvance (char * & i) {
  int out;
  out = iSwap(i);
  i += sizeof(int);
  return out;
}

static int uiSwapAdvance (char * & i) {
  int out;
  out = uiSwap(i);
  i += sizeof(vrpn_uint32);
  return out;
}

static short sSwapAdvance (char * & i) {
  short out;
  out = sSwap(i);
  i += sizeof(short);
  return out;
}

static vrpn_uint16 usSwapAdvance (char * & i) {
  vrpn_uint16 out;
  out = usSwap(i);
  i += sizeof(vrpn_uint16);
  return out;
}

static double dSwapAdvance (char * & i) {
  double out;
  out = dSwap(i);
  i += sizeof(double);
  return out;
}

static int iSwapAdvance (const char * & i) {
  int out;
  out = iSwap(i);
  i += sizeof(int);
  return out;
}

static int uiSwapAdvance (const char * & i) {
  int out;
  out = uiSwap(i);
  i += sizeof(vrpn_uint32);
  return out;
}

static short sSwapAdvance (const char * & i) {
  short out;
  out = sSwap(i);
  i += sizeof(short);
  return out;
}

static vrpn_uint16 usSwapAdvance (const char * & i) {
  vrpn_uint16 out;
  out = usSwap(i);
  i += sizeof(vrpn_uint16);
  return out;
}

static double dSwapAdvance (const char * & i) {
  double out;
  out = dSwap(i);
  i += sizeof(double);
  return out;
}
#endif  // 0

/** Try to do the minimum of work possible to get the information we
need from a Topometrix file header. We read in the header in a
chunk, then extract only the info we need. In version 5.00, we do
this by casting a structure which matches the structure used by
Topo to write out the file. In version 4.0 and previous, we use the
byte counts to go to the right spot in the header to get the
info. In all cases we need to do a byte swap of the info we
retrieve, because SGI/HP/other UNIX boxes have their byte order
swapped from that of the PC. (*Swap procedures don't do anything on
a PC).
*/
int TopoFile::getGridInfoFromHeader (const char * temp) {
    unsigned int i;

  // How we parse the header depends totally on the version number
  if (iRelease >=500) {
      // For version 5 files, we have the exact structure
      // used to write out the header. We use swap procedures
      // to read from it because SGI/unix and PC byte order are different.
      PTOPODOC doc;
      doc = (PTOPODOC)temp;

      // We have release and offset info already, just check it.
      if (iRelease != iSwap(doc->sInfo.iRelease)) {
	  fprintf(stderr, "Topo.C:: WARNING text header release and "
		  "binary header release not the same, continuing anyway\n");
      }
      if (iOffset != iSwap(doc->sInfo.iOffset)) {
	  fprintf(stderr, "Topo.C:: WARNING text header offset and "
		  "binary header offset not the same, continuing anyway\n");
      }

      for (i = 0; i < sizeof(szRelease); i++)
	  szRelease[i] = doc->sInfo.szRelease[i];
      fprintf(stderr, "TopoFile::getInfo got szRelease \"%s\"\n",
	      szRelease);
      for (i = 0; i < sizeof(szDatetime); i++)
	  szDatetime[i] = doc->sInfo.szDatetime[i];
      //fprintf(stderr, "TopoFile::ParseDocInfo got szDatetime \"%s\"\n",
      //szDatetime);
      for (i = 0; i < sizeof(szDescription); i++)
	  szDescription[i] =  doc->sInfo.szDescription[i];
      //fprintf(stderr, "TopoFile::ParseDocInfo got szDescription \"%s\"\n",
      //sInfo.szDescription);

      iRows = iSwap(doc->sInfo.iRows);
      iCols = iSwap(doc->sInfo.iCols);
      //fprintf(stderr, "iCols is %d\n", sInfo.iCols);
      fXmin = dSwap(doc->sInfo.fXmin);
      fXmax = dSwap(doc->sInfo.fXmax);
      fYmin = dSwap(doc->sInfo.fYmin);
      fYmax = dSwap(doc->sInfo.fYmax);
      fDACtoWorld = dSwap(doc->sInfo.fDACtoWorld);
      fDACtoWorldZero = dSwap(doc->sInfo.fDACtoWorldZero);
      iWorldUnitType = sSwap(doc->sInfo.iWorldUnitType);
      iXYUnitType = sSwap(doc->sInfo.iXYUnitType);

      for (i = 0; i < sizeof(szWorldUnit); i++)
	  szWorldUnit[i] = doc->sInfo.szWorldUnit[i];
      for(i = 0; i < sizeof(szXYUnit); i++)
	  szXYUnit[i] = doc-> sInfo.szXYUnit[i] ;
      
      iLayers = sSwap(doc->sInfo.iLayers);

      //bHasBkStrip = iSwap(doc->sInfo.bHasBkStrip);
  } else if (iRelease >=301) {
      if (iRelease <400) {
	  fprintf(stderr, "Topo.C:: WARNING - parsing a Topo file "
		  "header whose version is less than 4.00 just "
		  "like it is a 4.00 file.\n"
		  "  This may cause errors.\n");
      }
      // Get data from the proper place 
      // Stolen from topofile.c::convertWin31ToNT
      const char * w;
      // Use a -1 offset so the numbers we add to w match the 
      // byte count from the document.h header in version
      // 4.0 of Topo code (in CVS repository as revision 1.4)
    w = temp -1;
      // We have release and offset info already, just check it.
      if (iRelease != iSwap(w+1)) {
	  fprintf(stderr, "Topo.C:: WARNING text header release and "
		  "binary header release not the same, continuing anyway\n");
      }
      if (iOffset != iSwap(w+5)) {
	  fprintf(stderr, "Topo.C:: WARNING text header offset and "
		  "binary header offset not the same, continuing anyway\n");
      }

      for (i = 0; i < sizeof(szRelease); i++)
	  szRelease[i] = *(const char *)(w + 9 + i);
      fprintf(stderr, "TopoFile::getInfo got szRelease \"%s\"\n",
	      szRelease);
      for (i = 0; i < sizeof(szDatetime); i++)
	  szDatetime[i] = *(const char *)(w + 25 + i);
      //fprintf(stderr, "TopoFile::ParseDocInfo got szDatetime \"%s\"\n",
      //szDatetime);
      for (i = 0; i < sizeof(szDescription); i++)
	  szDescription[i] = *(const char *)(w + 45 + i);
      //fprintf(stderr, "TopoFile::ParseDocInfo got szDescription \"%s\"\n",
      //sInfo.szDescription);

      iRows = iSwap(w+151);
      iCols = iSwap(w+155);
      //fprintf(stderr, "iCols is %d\n", sInfo.iCols);
      // Floats instead of doubles in version 4.0
      fXmin = fSwap(w+163);
      fXmax = fSwap(w+ 167);
      fYmin = fSwap(w+171);
      fYmax = fSwap(w+175);
      fDACtoWorld = fSwap(w+179);
      fDACtoWorldZero = fSwap(w+183);
      iWorldUnitType = sSwap(w+191);
      iXYUnitType = sSwap(w+193);

      for (i = 0; i < sizeof(szWorldUnit); i++)
	  szWorldUnit[i] = *(const char *)(w + 195 + i);
      for(i = 0; i < sizeof(szXYUnit); i++)
	  szXYUnit[i] = *(const char *)(w + 205 + i);
      
      iLayers = sSwap(w + 225);

      // BOOL is 2 bytes in 4.0
      //bHasBkStrip = sSwap(w+229);
  } else {
      fprintf(stderr, "Topo.C::getInfo: Version is less than 3.01 -"
	      " Sorry, can't read it.\n");
      return -1;
  }
  return 0;

}

int TopoFile::printGridInfoFromHeader()
{
     if(!valid_header){
	fprintf(stderr, "No valid header read, no info printed\n"); 
	return -1;
    }
    printf("Binary Release: %ld, Header Offset: %ld\n",
	   (long)iRelease, (long)iOffset);
    printf("Text release string: %s\n",szRelease);
    printf("Date: %s\nDescription: %s\n",szDatetime, szDescription);
    
    printf("Grid Size <X,Y>: <%ld,%ld>\n",(long)iRows, (long)iCols);
    printf("Scan DataX Min/Max: <%f,%f>\n",fXmin,fXmax);
    printf("Scan DataY Min/Max: <%f,%f>\n",fYmin,fYmax);
    
    printf("DAC to World: %f\nDAC to World Zero: %f\n", 
	   fDACtoWorld, fDACtoWorldZero);
    printf("World Units: %s (%ld)\nXY Units: %s (%ld)\n",
	   szWorldUnit, (long)iWorldUnitType, 
	   szXYUnit, (long)iXYUnitType);
	
//      printf("Layers: %d\nbkStrip Data: %s\n", iLayers,
//  	   (bHasBkStrip ? "TRUE":"FALSE"));

    return 0;
}

/**
gridToTopoData must be called before this function so member
variables are set correctly!
Put info which may have changed into the Topo file header before we
write it out. To find the right spot to put the info, we do two
things. In version 5.00, we do this by casting a structure which
matches the structure used by Topo to write out the file. In
version 4.0 and previous, we use the byte counts to go to the right
spot in the header to write the info. In all cases we need to do a
byte swap of the info we write, because SGI/HP/other UNIX boxes
have their byte order swapped from that of the PC. (*Swap
procedures don't do anything on a PC).
*/
int TopoFile::putGridInfoIntoHeader (char * temp) {
    //    unsigned int i;

    // Some code to modify the text header.
  // This code is inspired by topofile.c:iWriteTopoBuffer
  // and modified to use sprintf instead of pointers.
//      sprintf(sInfo.szRelease, "%s %d", GetReleaseString(), 
//          sInfo.iOffset);
  
//      sprintf(header, "%s\n%s\n%s\n", 
//  	    sInfo.szRelease,
//  	    sInfo.szDatetime,
//  	    sInfo.szDescription);

      // Skip over the text header!
    temp += 256;
  // How we parse the header depends totally on the version number
  if (iRelease >=500) {
      // For version 5 files, we have the exact structure
      // used to write out the header. We use swap procedures
      // to write to it because SGI/unix and PC byte order are different.
      PTOPODOC doc;
      doc = (PTOPODOC)temp;

      // Release and offset info already correct. 

      // XXX Might want to change datetime to be current time.
//        for (i = 0; i < sizeof(szDatetime); i++)
//  	  doc->sInfo.szDatetime[i] = szDatetime[i];
      // XXX No mechanism provided to change description.
//        for (i = 0; i < sizeof(szDescription); i++)
//  	  doc->sInfo.szDescription[i] =  szDescription[i];

      doc->sInfo.iRows = iSwap(iRows);
      doc->sInfo.iCols = iSwap(iCols);
      doc->sInfo.fXmin = dSwap(fXmin);
      doc->sInfo.fXmax = dSwap(fXmax);
      doc->sInfo.fYmin = dSwap(fYmin);
      doc->sInfo.fYmax = dSwap(fYmax);
      // These data fields are never changed by anyone, so 
      // no need to change them in the header. 
//        doc->sInfo.fDACtoWorld = dSwap(fDACtoWorld);
//        doc->sInfo.fDACtoWorldZero = dSwap(fDACtoWorldZero);
//        doc->sInfo.iWorldUnitType = sSwap(iWorldUnitType);
//        doc->sInfo.iXYUnitType = sSwap(iXYUnitType);

//        for (i = 0; i < sizeof(szWorldUnit); i++)
//  	  doc->sInfo.szWorldUnit[i] = szWorldUnit[i];
//        for(i = 0; i < sizeof(szXYUnit); i++)
//  	  doc->sInfo.szXYUnit[i] = szXYUnit[i];
      
      doc->sInfo.iLayers = sSwap(iLayers);

      //doc->sInfo.bHasBkStrip = iSwap(bHasBkStrip);

      // The RECT structure tells Topo where the valid data is. 
      // We want it to be the whole NON-ZERO image
      // because zero values in the grid are data which hasn't been
      // sent from the microscope. 
      // WARNING Topo 5.0 seems to expect the Y values to be inverted
      doc->sInfo.rRoi.top = iSwap((vrpn_int32)(iCols -1 - rRoi.top));
      doc->sInfo.rRoi.left = iSwap((vrpn_int32)rRoi.left);
      doc->sInfo.rRoi.bottom = iSwap((vrpn_int32)(iCols -1 -rRoi.bottom));
      doc->sInfo.rRoi.right = iSwap((vrpn_int32)rRoi.right);

  } else if (iRelease >=301) {
      if (iRelease <400) {
	  fprintf(stderr, "Topo.C:: WARNING - parsing a Topo file "
		  "header whose version is less than 4.00 just "
		  "like it is a 4.00 file.\n"
		  "  This may cause errors.\n");
      }
      // Put data in the proper place 
      // Stolen from topofile.c::convertWin31ToNT
      char * w;
      // Use a -1 offset so the numbers we add to w match the 
      // byte count from the document.h header in version
      // 4.0 of Topo code (in CVS repository as revision 1.4)
    w = temp -1;
      // Release and offset info already correct. 

      // XXX Might want to change datetime to be current time.
//        for (i = 0; i < sizeof(szDatetime); i++)
//  	  szDatetime[i] = *(const char *)(w + 25 + i);
      // XXX No mechanism provided to change description.
//        for (i = 0; i < sizeof(szDescription); i++)
//  	  szDescription[i] = *(const char *)(w + 45 + i);

    vrpn_int32 tempi;
    vrpn_float32 tempf;
    vrpn_int16 temps;
    
    tempi = iSwap(iRows);
    memcpy(w+151, &tempi, sizeof(tempi));
    tempi = iSwap(iCols);
    memcpy(w+155, &tempi, sizeof(tempi));

      // Floats instead of doubles in version 4.0
    tempf = fSwap((vrpn_float32)fXmin);
    memcpy(w+163, &tempf, sizeof(tempf));
    tempf = fSwap((vrpn_float32)fXmax);
    memcpy(w+167, &tempf, sizeof(tempf));
    tempf = fSwap((vrpn_float32)fYmin);
    memcpy(w+171, &tempf, sizeof(tempf));
    tempf = fSwap((vrpn_float32)fYmax);
    memcpy(w+175, &tempf, sizeof(tempf));
    // These don't ever change, don't write out.
//        fDACtoWorld = fSwap(w+179);
//        fDACtoWorldZero = fSwap(w+183);
//        iWorldUnitType = sSwap(w+191);
//        iXYUnitType = sSwap(w+193);

//  	  szWorldUnit[i] = *(const char *)(w + 195 + i);
//  	  szXYUnit[i] = *(const char *)(w + 205 + i);
      
    // This is 2 bytes in ver 4.0
    temps = sSwap((vrpn_int16)iLayers);
    memcpy(w+225, &temps, sizeof(temps));

      // BOOL is 2 bytes in 4.0
      //bHasBkStrip = sSwap(w+229);

    
      // The RECT structure tells Topo where the valid data is. 
      // We want it to be the whole NON-ZERO image
      // because zero values in the grid are data which hasn't been
      // sent from the microscope. 
    // RECT elements are 2 bytes in version 4.0
    temps = sSwap((vrpn_int16)rRoi.left);
    memcpy(w+339, &temps, sizeof(temps));
    temps = sSwap((vrpn_int16)(iCols -1 - rRoi.top));
    memcpy(w+345, &temps, sizeof(temps));
    temps = sSwap((vrpn_int16)(iCols -1 - rRoi.bottom));
    memcpy(w+341, &temps, sizeof(temps));
    temps = sSwap((vrpn_int16)rRoi.right);
    memcpy(w+343, &temps, sizeof(temps));

  } else {
      fprintf(stderr, "Topo.C::getInfo: Version is less than 3.01 -"
	      " Sorry, can't read it.\n");
      return -1;
  }
  return 0;

}

int TopoFile::printData(){
	int i,j;
	int offset;
	if(!valid_grid) return -1;
	for(i=0; i < iRows; i++){
		offset=i*iCols;
		for(j=0; j < iCols; j++){
			printf("%u,",(vrpn_uint16)griddata[offset+j]);
		}
		printf("\n");
	}
	return 0;
}


int TopoFile::printDocInfo(){

     if(!valid_header){
	fprintf(stderr, "Not a valid header, no info printed\n"); return -1;
    }

    int i;
    int effectiveNMaxGraphs = 16;
    PTOPODOC doc;
    doc = (PTOPODOC)(header + 256);
    PDOCUMENTINFO sInfo = &(doc->sInfo);

	printf("Release: %ld\n\t%s\nHeader Offset: %ld\n",(long)sInfo->iRelease, sInfo->szRelease,(long)sInfo->iOffset);
	printf("Date: %s\nDescription: %s\n",sInfo->szDatetime, sInfo->szDescription);

	for(i=0;i<effectiveNMaxGraphs;i++) {
	    if ((sInfo->fPosX[i]!=0.0) && (sInfo->fPosY[i]!=0.0)) {
		printf("Non-zero Curve(%d) <X,Y>: <%f,%f>\n",i,sInfo->fPosX[i], sInfo->fPosY[i]);
	    }
	}
	printf("Grid Size <X,Y>: <%ld,%ld>\n",(long)sInfo->iRows, (long)sInfo->iCols);
	printf("Data Min/Max: <%d,%d>\n",sInfo->iDACmin,sInfo->iDACmax);
	printf("Scan DataX Min/Max: <%f,%f>\n",sInfo->fXmin,sInfo->fXmax);
	printf("Scan DataY Min/Max: <%f,%f>\n",sInfo->fYmin,sInfo->fYmax);

	printf("DAC to World: %f\nDAC to World Zero: %f\nDAC to color: %d\nDAC to color Zero: %d\n", sInfo->fDACtoWorld, sInfo->fDACtoWorldZero, sInfo->iDACtoColor, sInfo->iDACtoColorZero);
	printf("World Units: %s (%d)\nXY Units: %s (%d)\nRate unit: %s\n",sInfo->szWorldUnit, sInfo->iWorldUnitType, sInfo->szXYUnit, sInfo->iXYUnitType, sInfo->szRateUnit);
	
	printf("Layers: %d\nEChem Data: %s\nbkStrip Data: %s\n", sInfo->iLayers,(sInfo->bHasEchem ?"TRUE":"FALSE" ), (sInfo->bHasBkStrip ? "TRUE":"FALSE"));
	for(i=0; i < effectiveNMaxGraphs; i++){ 
	    if (sInfo->iPts[i]!=0) {
		printf("points in Curve(%d): %d\n",i,sInfo->iPts[i]);
	    }
	}

	printf("Physical units in X: %s\n",sInfo->szXUnit);
	return 1;
}

int TopoFile::printScanParams(){
	if(!valid_header) return-1;
    PTOPODOC doc;
    doc = (PTOPODOC)(header + 256);
    PSCANPARAMS sScanParam = &(doc->sScanParam);

	printf("Raw Data Source (type): %d\nScan directions: %d\n",sScanParam->iDataType,sScanParam->iDataDir);
	printf("Scan datamode: %d\nZ DAC max: %f\n",sScanParam->iDataMode, sScanParam->fScanZmax);
	printf("Z DAC min: %f\nX piezo resol.: %f\n",sScanParam->fScanZmin, sScanParam->fScanXmax);
	printf("Y piezo resol.: %f\nSTM tip voltage (-10,10)volts: %f\n",sScanParam->fScanYmax, sScanParam->fI);

	printf("Scanned Image range: %f\nScan rate (unit/sec): %f\n",sScanParam->fRange,sScanParam->fRate);
	printf("ADC gain flag: %d\nPID Pro: %f\n",sScanParam->iGain,sScanParam->fPro);
	printf("PID Integ: %f\nPID Der: %f\n",sScanParam->fInteg,sScanParam->fDer);
	printf("Z piezo gain flag: %d\nXY scan rotation (radians): %f\n",sScanParam->iGainZ,sScanParam->fRotation);

	printf("Modulation mode (angstroms): %f\n# points to average/img point: %f\n",sScanParam->fModLevel,sScanParam->fAveraging);
	printf("SP Calibration Factor: %f\nXY Calibration Type: %d\n",sScanParam->fSpCalFactor,sScanParam->iCalibType);
	printf("LaserIntensity: %d\nZ scale factor: %d\n",sScanParam->iLaserIntensity,sScanParam->iScaleFactorZ);

	printf("piezo scan location min x: %d\npiezo scan max x: %d\n",sScanParam->iDACminX,sScanParam->iDACmaxX);
	printf("piezo scan location min y: %d\npiezo scan max y: %d\n",sScanParam->iDACminY,sScanParam->iDACmaxY);
	printf("scantype %s\nProbe type: %d\n",sScanParam->cScanType,sScanParam->iProbeType);
	printf("StageType: %d\nCalibration file type: %d\n",sScanParam->iStageType,sScanParam->iCalFileSource);
	printf("Overscan range in X (%%): %f\nOverscan range in Y (%%): %f\n",sScanParam->fOverscanX,sScanParam->fOverscanY);
	return 1;

}

int TopoFile::printScanLayers(){
	if(!valid_header) return -1;

    PTOPODOC doc;
    doc = (PTOPODOC)(header + 256);
    PSCANPARAMSLAYER p;
    p=&(doc->sScanParam.scan3d);	

	printf("Voltage dist start: %f\nVoltate dist stop: %f\nForce limit: %f\n",p->fVzStart,p->fVzStop,p->fVzLimit);

	printf("speed volt/dist ramp 1 (V/sec or micr/sec): %f\n",p->fVzSpeed1);
	printf("speed volt/dist ramp 2 (V/sec or micr/sec): %f\n",p->fVzSpeed2);
	printf("speed volt/dist ramp 3 (V/sec or micr/sec): %f\n",p->fVzSpeed3);
	printf("speed volt/dist ramp 4 (V/sec or micr/sec): %f\n",p->fVzSpeed4);
	printf("pullback distance: %f\n",p->fVzPullback);

	printf("# Layers: %d\n# of halfcycles (for=1, for+back=2): %d\n",p->iLayers,p->iHalfCycles);
	printf("Averagine num/layer point: %d\nDelay time before 1st sample (micr-sec): %f\n",p->iAvgPoint,p->fDelayStart);
	printf("Delay time before sample (micr-sec): %f\nDelay time before pullback (micr-sec): %f\n",p->fDelaySample,p->fDelayPullback);

	printf("Delay time before feeback established (micr-sec): %f\nFeedBack enabled: %ld\n",p->fDelayEstFeedbk,(long)p->bFeedbkPoints);
	printf("FeedBack between curves enabled: %ld\nVolt/dist values relative: %ld\n",(long)p->bFeedbkCurves,(long)p->bVzRelative);
	printf("Modulation frequency: %f\n",p->fVzMod);
	printf("ExtLayer: %d\n",p->iExtLayer);
	printf("bSpecialNCScan: %ld\n",(long)p->bSpecialNCScan);
	return 1;
}

int TopoFile::parseData(int handle){
        int     i,size;
	
	if(!valid_header) {
          fprintf(stderr, "TopoFile::ParseData:  "
                          "Header wasn't valid, so skipping data.\n");
          return -1;
        }

	/* Seeking to where the data should start - this was not done in the
         * code from Topometrix. */
        printf("TopoFile::ParseData: Reading data starting at byte %ld\n",(long)iOffset);
	if( lseek( handle, iOffset, SEEK_SET) == -1) { /* Error */
                return -1;
        }


        size = iCols * iRows * iLayers;
        griddata = new vrpn_uint16[size];
	if(griddata == NULL) {
                fprintf(stderr,"TopoFile::ParseData(): Out of memory\n");
                fprintf(stderr,"  (Can't allocate %ldx%ld grid with %ld layers)\n",
                        (long)iCols, (long)iRows, (long)iLayers);
		return -1;
        }
        iGridDataLength = size;

        /* Seeking to where the data should start - this was not done in the
         * code from Topometrix. */
        if( lseek( handle, iOffset, SEEK_SET) == -1) { /* Error */
		printf("TopoFile::ParseData File seek error\n");
		delete [] griddata;
		return -1;
		
        }

        if ( read(handle, (char*)griddata, size*sizeof(vrpn_uint16)) 
	     != (signed)(size*sizeof(vrpn_uint16))){
		printf("TopoFile::ParseData didn't read all of the data -- error\n");
		delete [] griddata;
		return -1;
        }

        for (i = 0; i < size; i++) griddata[i] = usFix( &griddata[i] );
	valid_grid=1;

        return(0);
}


int TopoFile::readTopoFile(const char* filename){
	FILE * handle;
	int in_descriptor;
	handle = fopen(filename,"rb");
        if (handle == NULL){
        	printf("TopoFile:: readTopoFile Error in opening file %s\n",filename);
		exit(-1);
        }
        if ( (in_descriptor = fileno(handle)) == -1) {
                fprintf(stderr,"TopoHeader::readTopoFile(): Can't get descriptor");
                return(-1);
        }
    // XXX DEBUG find out how big structures are.
//  printf("**DOC Info:\n" );
//  printf("  sizeof TOPODOC: %d\n",  sizeof(TOPODOC));
//  printf("  sizeof SCANPARAMSLAYER: %d\n",  sizeof(SCANPARAMSLAYER));
//  printf("  sizeof SCANPARAMS: %d\n",  sizeof(SCANPARAMS));
//  printf("  sizeof DOCUMENTINFO: %d\n",  sizeof(DOCUMENTINFO));
//printf("  sizeof FILEHEADER: %d\n",  sizeof(FILEHEADER));

        parseHeader(in_descriptor);
//      printDocInfo();
//  	printScanParams();
//  	printScanLayers();
	parseData(in_descriptor);
	fclose(handle);
	return 1;
}

// Utility routines for writeDocInfo().
// Return error codes.
// August 99, TCH.

/* Not used, so comment out until it is needed.
static int send (int handle, vrpn_int16 s) {
  vrpn_int16 stemp;
  stemp = sSwap((char *) &(stemp = s));
  return write(handle, (char *) &stemp, sizeof(vrpn_int16));
}

static int send (int handle, vrpn_uint16 us) {
  vrpn_uint16 ustemp;
  ustemp = usSwap((char *) &(ustemp = us));
  return write(handle, (char *) &ustemp, sizeof(vrpn_uint16));
}

static int send (int handle, vrpn_int32 i) {
  vrpn_int32 itemp;
  itemp = iSwap((char *) &(itemp = i));
  return write(handle, (char *) &itemp, sizeof(vrpn_int32));
}

static int send (int handle, vrpn_uint32 ui) {
  vrpn_int32 uitemp;
  uitemp = uiSwap((char *) &(uitemp = ui));
  return write(handle, (char *) &uitemp, sizeof(vrpn_uint32));
}

static int send (int handle, vrpn_float32 f) {
  vrpn_float32 ftemp;
  ftemp = fSwap((char *) &(ftemp = f));
  return write(handle, (char *) &ftemp, sizeof(vrpn_float32));
}

static int send (int handle, vrpn_float64 d) {
  vrpn_float64 dtemp;
  dtemp = dSwap((char *) &(dtemp = d));
  return write(handle, (char *) &dtemp, sizeof(vrpn_float64));
}
*/

/**
Header is written out in a lump binary object by
code in topo/topofile.c.
The format is defined as struct DOCUMENTINFO in topo/document.h,
so this code must parse the exact physical layout of that struct.
August 99, TCH.
*/
int TopoFile::writeHeader(int handle){

  // Write a topo file of the same version as we read in, 
  // by writing out exactly the same header as we read in,
  // tweaked only if we have changed some info.

  if (putGridInfoIntoHeader (header) < 0) {
      fprintf(stderr, "writeHeader: unable to add info to header.\n");
      return -1;
  }

  // header is full header from a Topo file. First, there is a 256
  // byte text header which contains the release, offset, date and
  // time in text form. The rest is binary information.
  if (write(handle,header,iOffset) != iOffset) {
      fprintf(stderr, "writeHeader: unable to write header to disk.\n");
      return -1;
  }
  return 0;

  
}

int TopoFile::writeData(int handle){
        int     i,size;
	

	if(!valid_grid) return -1;
        size = iCols * iRows * iLayers ;

	// Fix the byte ordering of the data, if necessary.
        for (i = 0; i < size; i++) griddata[i] = usFix( &griddata[i] );
 
        /* Seeking to where the data should start - this was not done
         * in the code from Topometrix. */
        if( lseek( handle, iOffset, SEEK_SET) == -1) { /* Error */
		fprintf(stderr, "TopoFile::writeData File seek error\n");
		return -1;
        }

        if ( write(handle, (char*)griddata, size*sizeof(vrpn_uint16)) 
	     != (signed)(size*sizeof(vrpn_uint16))){
		fprintf(stderr, "TopoFile::writeData didn't write all of the data -- error.\n");
		return -1;
        }
        return(0);        
}


int TopoFile::writeTopoFile(const char* filename){
    FILE *handle;
    handle=fopen(filename,"wb");
    if(handle==NULL){
	fprintf(stderr, "Topo::writeTopoFile Error in opening file %s\n"
		" No file written.\n",filename);
	return(-1);
    }
    return (writeTopoFile(handle));
}

int TopoFile::writeTopoFile(FILE* handle){

    int in_descriptor;
    if(handle==NULL){
	printf("TopoFile:: writeTopoFile NULL file pointer."
		" No file written.\n");
	return(-1);
    }
    if ( (in_descriptor = fileno(handle)) == -1) {
	fprintf(stderr,"Topo.C::writeTopometrixFile(): Can't get descriptor-"
		" No file written.\n");
	return(-1);
    }
    if (writeHeader(in_descriptor) < 0) {
	fprintf(stderr,"Topo.C::writeTopometrixFile(): Can't write header-"
		" No file written.\n");
	return -1;
    }
    if (writeData(in_descriptor) < 0){
	fprintf(stderr,"Topo.C::writeTopometrixFile(): Can't write data-"
		" No file written.\n");
	return -1;
    }	
    fclose(handle);
    return 0;
}

void TopoFile::fixUpDocStruct(){
    /*    
    if(verNumber > 300){

	if(verNumber < 305){
	    if(sScanParam.iStageType==12){ //MODEL_TOPOCRON # used for ver < 3.05
		if(sScanParam.iProbeType==PROBE_AFM) sScanParam.iStageType=9; //MODEL_TOPOCRON_AFM
		if(sScanParam.iProbeType==PROBE_STM) sScanParam.iStageType=10; //MODEL_TOPOCRON_STM
	    }
	}
	if(verNumber < 307){
	    switch(sScanParam.iStageType){
	    default:
	    case MODEL_STD:
		strcpy(sScanParam.szStageType,"Discoverer");
		switch(sScanParam.iProbeType){
		default:
		case PROBE_AFM:
		    strcpy(sScanParam.szStageName,"DiscovererAFM");
		    strcpy(sScanParam.szStageText,"Discoverer AFM");
		    break;
		case PROBE_STM:
		    strcpy(sScanParam.szStageName,"DiscovererSTMProbeHolder");
		    strcpy(sScanParam.szStageText,"Discoverer STM Probe Holder");
		}
		break;
	    case MODEL_STM:
		strcpy(sScanParam.szStageType,"Discoverer");
		strcpy(sScanParam.szStageName,"DiscovererSTM");
		strcpy(sScanParam.szStageText,"Discoverer STM");
		break;
	    case MODEL_EXPLORER_AFM:
		strcpy(sScanParam.szStageType,"Explorer");
		strcpy(sScanParam.szStageName,"ExplorerAFM");
		strcpy(sScanParam.szStageText,"Explorer AFM");
		break;
	    case MODEL_EXPLORER_STM:
		strcpy(sScanParam.szStageType,"Explorer");
		strcpy(sScanParam.szStageName,"ExplorerSTM");
		strcpy(sScanParam.szStageText,"Explorer STM");
		break;
	    case MODEL_UNIVERSAL_AFM:
		strcpy(sScanParam.szStageType,"Universal");
		strcpy(sScanParam.szStageName,"UniversalAFM");
		strcpy(sScanParam.szStageText,"Universal AFM");
		break;
	    case MODEL_SNOM:
		strcpy(sScanParam.szStageType,"Aurora");
		strcpy(sScanParam.szStageName,"Aurora");
		strcpy(sScanParam.szStageText,"Aurora");
		break;
	    case MODEL_OBSERVER_AFM:
		strcpy(sScanParam.szStageType,"Observer");
		strcpy(sScanParam.szStageName,"ObserverAFM");
		strcpy(sScanParam.szStageText,"Observer AFM");
		break;
	    case MODEL_OBSERVER_STM:
		strcpy(sScanParam.szStageType,"Observer");
		strcpy(sScanParam.szStageName,"ObserverSTM");
		strcpy(sScanParam.szStageText,"Observer STM");
		break;
	    case MODEL_TOPOCRON_AFM:
		strcpy(sScanParam.szStageType,"Omicron");
		strcpy(sScanParam.szStageName,"OmicronAFM");
		strcpy(sScanParam.szStageText,"Omicron AFM");
		break;
	    case MODEL_TOPOCRON_STM:
		strcpy(sScanParam.szStageType,"Omicron");
		strcpy(sScanParam.szStageName,"OmicronSTM");
		strcpy(sScanParam.szStageText,"Omicron STM");
		break;
	    }
	}	
	if(verNumber < 306){
	    sScanParam.fSensorResponse=1;	//in nA/nm
	    sScanParam.fKc=0.032;		//in N/m
	    sInfo.rRoi.left=0;
	    sInfo.rRoi.top=0;
	    sInfo.rRoi.right=sInfo.iCols-1;
	    sInfo.rRoi.bottom=sInfo.iRows-1;
	}

	if(verNumber < 307){
	    if(sInfo.rRoi.bottom==0 || sInfo.rRoi.right==0){
		sInfo.rRoi.left=0;
		sInfo.rRoi.top=0;
		sInfo.rRoi.right=sInfo.iCols-1;
		sInfo.rRoi.bottom=sInfo.iRows-1;
	    }
	}
    }
    else{
	fprintf(stderr,"Unable to process Topo file earlier than 3.01");
	exit(-1);
    }
    */
    return;
}

int TopoFile::topoDataToGrid(BCGrid* G, const char* filename){
        int     row, col, layer;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        double  min_z, max_z;
        int     data_type;
        BCPlane* plane;         // A new plane of data to be filled
        BCString name;          // Name assigned to the new plane

	if(valid_grid == 0 || valid_header == 0) return -1;

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

        switch (iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %ld (%s)\n",
                        (long)iXYUnitType, szXYUnit);
                return(-1);
        }

        switch (iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = TF_HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = TF_FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %ld (%s)\n",
                        (long)iWorldUnitType, szWorldUnit);
                return(-1);
        }

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        G->_num_x = iCols;
        G->_num_y = iRows;
        G->_min_x = fXmin * XYunits2nm;
        G->_max_x = fXmax * XYunits2nm;
        G->_min_y = fYmin * XYunits2nm;
        G->_max_y = fYmax * XYunits2nm;
        // We know the file size, set our own grid size.
        G->setGridSize(G->_num_x, G->_num_y);

	/* Printf duplicates info printed from read of header. 
        printf("  min_x %f max_x %f\n",G->_min_x,G->_max_x);
        printf("  min_y %f max_y %f\n",G->_min_y,G->_max_y);
        printf("  Units in X/Y: %s (type %d)\n",szXYUnit,
                iXYUnitType);
        printf("  Units in Z: %s (type %d)\n",szWorldUnit,
                iWorldUnitType);
	*/

        /*
         * Make it possible to read multiple planes.  They won't
         * always be height, but I think they will all be the same
         * type.
         */

        for (layer = 0; layer < iLayers; layer++) {

                /*
                 * Allocate a plane to hold the height information
                 */

                switch (data_type) {
                  case TF_HEIGHT:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "nm", NOT_TIMED);
                        break;
                  case TF_AUX:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "V", NOT_TIMED);
                        break;
                  case TF_DEFLECTION:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "nA", NOT_TIMED);
                        break;
                  case TF_FORCE_MODULATION_DATA:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "nA/Angstrom", NOT_TIMED);
                        break;
                  default:
                        fprintf(stderr,"Unknown data type for Topo data (%d)\n",
                                data_type);
                }
                plane->_image_mode = data_type;

                /*
                 * Fill in the data
                 */

                plane->setScale(1.0);
                Zscale = fDACtoWorld;
                Zoffset = fDACtoWorldZero;
	/* Printf duplicates info printed from read of header. 
        	printf("  Zscale=%f Zoffset=%f\n",Zscale,Zoffset);
	*/
                min_z = max_z = Zunits2nm * (griddata[layer*G->_num_x*G->_num_y] *
                                Zscale + Zoffset);      // First element

                // Data is stored in the Topo file in column-major order
                // starting from the lower left corner.
                // the factor layer*_num_x*_num_y gets you to the start
                // of a layer.
                // the factor col*_num_y + row gets you to the right
                // element in that layer.
                // The Y elements are stored backwards with respect to
                // the Plane structure, so Y needs to be inverted.
                for (col = 0; col < G->_num_y; col++) {
                  for (row = 0; row < G->_num_x; row++) {
                        int y = (G->_num_y - 1) - col;
                        plane->setValue(row,y, Zunits2nm * (griddata[(layer*G->_num_x + col)*G->_num_y + row] * Zscale + Zoffset) );
                        min_z = min(min_z, plane->value(row,y));
                        max_z = max(max_z, plane->value(row,y));
                  }
                }
                plane->setMinAttainableValue(min_z);
                plane->setMaxAttainableValue(max_z);
        }
	/* Printf duplicates info printed from read of header. 
        printf("  min_z %f max_z %f\n",min_z,max_z);
	*/
        return(0);
}

int TopoFile::imageToTopoData(nmb_Image *I) {
	int     row, col;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        int     data_type;

        if(valid_header == 0) {
            return -1;
        }
        if(griddata != NULL) {
            delete [] griddata;
        }

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

        switch (iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %ld (%s)\n",
                        (long)iXYUnitType, szXYUnit);
                return(-1);
        }

        switch (iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = TF_HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = TF_FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %ld (%s)\n",
                        (long)iWorldUnitType, szWorldUnit);
                return(-1);
        }
        // Satisfy compiler
        data_type = data_type;
        
        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        iCols = I->width();
        iRows = I->height();
        fXmin = I->boundX(nmb_ImageBounds::MIN_X_MIN_Y)/ XYunits2nm;
        fXmax = I->boundX(nmb_ImageBounds::MAX_X_MAX_Y)/ XYunits2nm;
        fYmin = I->boundY(nmb_ImageBounds::MIN_X_MIN_Y)/ XYunits2nm;
        fYmax = I->boundY(nmb_ImageBounds::MAX_X_MAX_Y)/ XYunits2nm;

        iLayers = 1;

        short top,left, bottom,right;
        if(I->validDataRange(&top,&left,&bottom,&right) < 0) {
            printf("No valid data to export - abort\n");
            return -1;
        } else {
            printf("  Valid data range discovered: t %d, l %d, b %d, r %d\n",
                   top, left, bottom, right);
            rRoi.top = top;
            rRoi.left = left;
            rRoi.bottom = bottom;
            rRoi.right = right;
        }

        Zscale = fDACtoWorld;
        Zoffset = fDACtoWorldZero;


        printf("  Units in X/Y: %s (type %ld)\n",szXYUnit,
                (long)iXYUnitType);
        printf("  Units in Z: %s (type %ld)\n",szWorldUnit,
                (long)iWorldUnitType);

        griddata=new vrpn_uint16[iCols*iRows];
        if(griddata == NULL){
                fprintf(stderr,"Unable to allocate griddata\n"); 
                return -1;
        }
        iGridDataLength = iCols*iRows;

        // Data is stored in the Topo file in column-major order
        // starting from the lower left corner.
        // the factor col*_num_y + row gets you to the right
        // element 
        // The Y elements are stored backwards with respect to
        // the Plane structure, so Y needs to be inverted.

        for (col = 0; col < iRows; col++) {
            for (row = 0; row < iCols; row++) {
                int y = (iRows - 1) - col;
                griddata[(col)*iRows+ row] =
                  (vrpn_uint16)((
                         ((float)I->getValue(row,y)/(float)Zunits2nm)
                                 -(float)Zoffset
                                 )/(float)Zscale
                                );
            }
        }
        valid_grid = 1;
        return(0);
}

int TopoFile::gridToTopoData(BCGrid* G, BCPlane *P){
        int     row, col;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        int     data_type;
        BCPlane* plane;         // A new plane of data to be filled
        BCString name;          // Name assigned to the new plane

	if(valid_header == 0) return -1;
	if(griddata != NULL) delete [] griddata;

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

        switch (iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %ld (%s)\n",
                        (long)iXYUnitType, szXYUnit);
                return(-1);
        }

        switch (iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = TF_HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = TF_FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %ld (%s)\n",
                        (long)iWorldUnitType, szWorldUnit);
                return(-1);
        }
        // Satisfy compiler
        data_type = data_type;

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        iCols = G->_num_x;
        iRows = G->_num_y;
        fXmin = G->_min_x / XYunits2nm;
        fXmax = G->_max_x/ XYunits2nm;
        fYmin = G->_min_y / XYunits2nm;
        fYmax = G->_max_y / XYunits2nm;
       	iLayers = 1;
	plane = P;

	short top,left, bottom,right;
	if(P->findValidDataRange(&top,&left,&bottom,&right) < 0) {
	    printf("No valid data to export - abort\n");
	    return -1;
	} else {
	    printf("  Valid data range discovered: t %d, l %d, b %d, r %d\n",
		   top, left, bottom, right);
	    rRoi.top = top;
	    rRoi.left = left;
	    rRoi.bottom = bottom;
	    rRoi.right = right;
	}

        Zscale = fDACtoWorld;
        Zoffset = fDACtoWorldZero;


        printf("  Units in X/Y: %s (type %ld)\n",szXYUnit,
                (long)iXYUnitType);
        printf("  Units in Z: %s (type %ld)\n",szWorldUnit,
                (long)iWorldUnitType);
	
	griddata=new vrpn_uint16[iCols*iRows];
	if(griddata == NULL){
		fprintf(stderr,"Unable to allocate griddata\n"); 
		return -1;
	}
        iGridDataLength = iCols*iRows;

	// Data is stored in the Topo file in column-major order
        // starting from the lower left corner.
        // the factor col*_num_y + row gets you to the right
        // element
        // The Y elements are stored backwards with respect to
        // the Plane structure, so Y needs to be inverted.
	
        for (col = 0; col < iRows; col++) {
            for (row = 0; row < iCols; row++) {
	        int y = (iRows - 1) - col;
                griddata[(col)*iRows+ row] = 
		  (vrpn_uint16)((
			 ((float)plane->value(row,y)/(float)Zunits2nm)
				 -(float)Zoffset
				 )/(float)Zscale
				);
            }
        }
	valid_grid = 1;
	return(0);
}


int TopoFile::gridToTopoData(BCGrid* G){
        int     row, col, layer;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        int     data_type;
        BCPlane* plane;         // A new plane of data to be filled
        BCString name;          // Name assigned to the new plane

	if(valid_header == 0) return -1;
	if(griddata != NULL) delete [] griddata;

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

        switch (iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %ld (%s)\n",
                        (long)iXYUnitType, szXYUnit);
                return(-1);
        }

        switch (iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = TF_HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = TF_DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = TF_FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %ld (%s)\n",
                        (long)iWorldUnitType, szWorldUnit);
                return(-1);
        }
        // Satisfy compiler
        data_type = data_type;

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        iCols=G->_num_x;
        iRows=G->_num_y;
        fXmin=G->_min_x / XYunits2nm;
        fXmax=G->_max_x/ XYunits2nm;
        fYmin=G->_min_y / XYunits2nm;
        fYmax=G->_max_y / XYunits2nm;
       	iLayers=G->numPlanes();
	plane=G->head();

        printf("  Units in X/Y: %s (type %ld)\n",szXYUnit,
                (long)iXYUnitType);
        printf("  Units in Z: %s (type %ld)\n",szWorldUnit,
                (long)iWorldUnitType);
	
                Zscale = fDACtoWorld;
                Zoffset = fDACtoWorldZero;


		griddata=new vrpn_uint16[iLayers*iCols*iRows];
		if(griddata == NULL){ fprintf(stderr,"Unable to allocate griddata\n"); return -1;}
                iGridDataLength = iLayers*iCols*iRows;

		// Data is stored in the Topo file in column-major order
                // starting from the lower left corner.
                // the factor layer*_num_x*_num_y gets you to the start
                // of a layer.
                // the factor col*_num_y + row gets you to the right
                // element in that layer.
                // The Y elements are stored backwards with respect to
                // the Plane structure, so Y needs to be inverted.
	
		for(layer =0; layer < iLayers; layer++){
       	         for (col = 0; col < iRows; col++) {
                  for (row = 0; row < iCols; row++) {
                        int y = (iRows - 1) - col;
                        griddata[(layer*iCols+ col)*iRows+ row] = \
				(vrpn_uint16)(
					(
						((float)plane->value(row,y)/(float)Zunits2nm)\
						-(float)Zoffset\
					)/(float)Zscale
				);
                  }
                 }
		 plane=plane->next();
		}
	valid_grid = 1;
	return(0);
}

/**
suppose we read in some other file format and we want to output the grid
as a topo file but since we didn't read in a topo file we don't have a
TopoFile initialized correctly to allow us to fill it in with the data
and write it out - this function does the necessary initialization so that
you can load any data loaded into BCGrid into spmlab
*/
int TopoFile::initForConversionToTopo(double z_scale_nm, double z_offset_nm) {
    if (header) {
	fprintf(stderr, "Error: TopoFile::initForConversionToTopo "
		"- cannot overwrite loaded topo file\n");
    }
    printf("TopoFile::initForConversionToTopo scale %f offset %f\n", z_scale_nm, z_offset_nm);
    iRelease = 400;
    iOffset = 2112;
    header = new char[iOffset];
    iHeaderLength = iOffset;
    // Set whole header to be zero.
    memset(header, 0, iOffset*sizeof(char));
    sprintf(header, "#R4.00# 2112\n08/10/98 00:04:34\nconverted file\n");

    PTOPODOC doc;
    doc = (PTOPODOC)(header + 256);
    PDOCUMENTINFO sInfo = &(doc->sInfo);
    
    /* Need this info!
    sInfo->iCols = 300;
    sInfo->iRows = 300;
    */
    sInfo->fDACtoWorld = z_scale_nm;
    sInfo->fDACtoWorldZero = z_offset_nm;

    sprintf(sInfo->szWorldUnit, "nm");
    sprintf(sInfo->szXYUnit, "nm");
    sInfo->iWorldUnitType = 1;	// indicates nm
    sInfo->iXYUnitType = 2;	// indicates nm
    sInfo->iRelease = iRelease;
    sInfo->iOffset = iOffset;
    sprintf(sInfo->szRelease, "#R4.00# 2112");
    sprintf(sInfo->szRateUnit, "\265m/s");
    sInfo->iLayers = 1;
    valid_header = 1;
    return 0;
}


int BCGrid::readTopometrixFile(TopoFile &TGF, const char *filename){

	TGF.readTopoFile(filename);
	TGF.topoDataToGrid(this,filename);	
	return 1;	
	
}

// mostly copied from topofile.c

void TopoFile::convertWin31ToNT (const char * /*w*/) {

  /*
    int i;
    const char *l;
  int effectiveNMaxGraphs = 8;
    printf("TopoFile::ConvertWin31ToNT executing.\n");

    w -= 1;
    for (i=0;i<N_DATETIME;i++)
        sInfo.szDatetime[i] = *(const char *)(w+25+i);
    for (i=0;i<N_DESCRIPTION;i++)
        sInfo.szDescription[i] = *(const char *)(w+45+i);
    for (i=0;i<effectiveNMaxGraphs;i++)
        sInfo.fPosX[i] = fSwap(w + 85 + i * sizeof(vrpn_float32));
    for (i=0;i<effectiveNMaxGraphs;i++)
        sInfo.fPosY[i] = fSwap(w + 117 + i * sizeof(vrpn_float32));
    sInfo.iCurves = sSwap((w+149));
    sInfo.iRows = iSwap((w+151));
    sInfo.iCols = iSwap((w+155));
    sInfo.iDACmax = usSwap((w+159));
    sInfo.iDACmin = usSwap((w+161));
    sInfo.fXmin = fSwap(w+163);
    sInfo.fXmax = fSwap(w+167);
    sInfo.fYmin = fSwap(w+171);
    sInfo.fYmax = fSwap(w+175);
    sInfo.fDACtoWorld = fSwap(w+179);
    sInfo.fDACtoWorldZero = fSwap(w+183);
    sInfo.iDACtoColor = usSwap(w+187);
    sInfo.iDACtoColorZero = usSwap((w+189));
    sInfo.iWorldUnitType = sSwap((w+191));
    sInfo.iXYUnitType = sSwap((w+193));

    for(i=0;i<10;i++)
        sInfo.szWorldUnit[i] = *(const char *)(w+195+i);
    for(i=0;i<10;i++)
        sInfo.szXYUnit[i] = *(const char *)(w+205+i);
    for(i=0;i<10;i++)
        sInfo.szRateUnit[i] = *(const char *)(w+215+i);

    sInfo.iLayers = sSwap((w+225));
    sInfo.bHasEchem = sSwap((w+227));
    sInfo.bHasBkStrip = sSwap((w+229));
    for(i=0;i<effectiveNMaxGraphs;i++)
        sInfo.iPts[i] = sSwap((w+231) + i * sizeof(short));
    sInfo.iXUnitType = sSwap((w+247));
    for(i=0;i<10;i++)
        sInfo.szXUnit[i] = *(const char *)(w+249+i);
    sInfo.bHasAcqDisplay = (TOPOBOOL)sSwap(w+259);
    sInfo.iTilt = sSwap((w+261));
    sInfo.iScaleZ = sSwap((w+263));
    sInfo.iFilter = sSwap((w+265));
    sInfo.iShading = sSwap((w+267));
    for (i = 0; i < 8; i++) {
        sInfo.dTiltC[i] = fSwap(w + 269 + i * sizeof(vrpn_float32));
        //sInfo.dTiltC[i] = *((double *)(w+269)+i);
    }
    sInfo.iDACDisplayZero = usSwap(w+333);    
    sInfo.iDACDisplayRange = usSwap(w+335);        
    sInfo.rRoi.left = sSwap(w+339);
    sInfo.rRoi.top = sSwap(w+345);
    sInfo.rRoi.right = sSwap(w+343);
    sInfo.rRoi.bottom = sSwap(w+341);    

    w += 768;

    sScanParam.iDataType = sSwap(w+1);
    sScanParam.iDataDir = sSwap(w+3);    
    sScanParam.iDataMode = sSwap(w+5);        
    sScanParam.fScanZmax = fSwap(w+7);
    sScanParam.fScanZmin = fSwap(w+11);    
    sScanParam.fScanXmax = fSwap(w+15);    
    sScanParam.fScanYmax = fSwap(w+19);        

    sScanParam.fVtip = fSwap(w+23);        
    sScanParam.fI = fSwap(w+27);        
    sScanParam.fVz = fSwap(w+31);        
    sScanParam.fRange = fSwap(w+35);        
    sScanParam.fRate = fSwap(w+39);        
    sScanParam.iGain = sSwap(w+43);        
    sScanParam.fPro = fSwap(w+45);        
    sScanParam.fInteg = fSwap(w+49);        
    sScanParam.fDer = fSwap(w+53);            
    sScanParam.iGainZ = sSwap(w+57);        


    sScanParam.fRotation = fSwap(w+59);            
    sScanParam.fModLevel = fSwap(w+63);            
    sScanParam.fAveraging = fSwap(w+67);            
    sScanParam.fSpCalFactor = fSwap(w+71);            
    sScanParam.iCalibType = sSwap(w+75);        
    sScanParam.iLaserIntensity = sSwap(w+77);        
    sScanParam.iScaleFactorZ = usSwap(w+79);        
    sScanParam.iDACminX = usSwap(w+81);            
    sScanParam.iDACmaxX = usSwap(w+83);            
    sScanParam.iDACminY = usSwap(w+85);            
    sScanParam.iDACmaxY = usSwap(w+87);            
    for(i=0;i<6;i++)
        sScanParam.cScanType[i] = *(const char *)(w+89+i);
    sScanParam.iProbeType = sSwap(w+95);        
    sScanParam.iStageType = sSwap(w+97);        
    sScanParam.iCalFileSource = sSwap(w+99);        
    sScanParam.fOverscanX = fSwap(w+101);            
    sScanParam.fOverscanY = fSwap(w+105);           
    sScanParam.iSetpointUnits = sSwap(w+109);            
    sScanParam.fNcRegAmp = fSwap(w+111);
    sScanParam.iGainXY = sSwap(w+115);    
    sScanParam.iOffsetX = usSwap(w+117);    
    sScanParam.iOffsetY = usSwap(w+119);        
    for(i=0;i<4;i++)
        sScanParam.fHysteresisX[i] = fSwap(w + 121 + i * sizeof(vrpn_float32));
    for(i=0;i<4;i++)
        sScanParam.fHysteresisY[i] = fSwap(w + 137 + i * sizeof(vrpn_float32));
    sScanParam.iOffsetZ = usSwap(w+153);    
    for(i=0;i<4;i++)
        sScanParam.fHysteresisZ[i] = fSwap(w + 155 + i * sizeof(vrpn_float32));
    sScanParam.fCrossTalkCoef = fSwap(w+171);
    sScanParam.fSensorResponse = fSwap(w+175);
    sScanParam.fKc = fSwap(w+179);    
    sScanParam.iCantileverType = sSwap(w+183);        
    for(i=0;i<16;i++)
        sScanParam.szScannerSerialNumber[i] = *(const char *)(w+185+i);
    sScanParam.iZlinearizer = sSwap(w+201);            
    sScanParam.iADC = iSwap(w+203);                
    sScanParam.bNonContact = (TOPOBOOL)sSwap(w+207);
    sScanParam.CantileverType = sSwap(w+209);                    
    sScanParam.fDriveAmplitude = fSwap(w+211);                
    
    sScanParam.fDriveFrequency = fSwap(w+215);
    sScanParam.iNonContactMode = sSwap(w+219);
    sScanParam.iNonContactPhase = sSwap(w+221);
    
    l = w+257-1;
    sScanParam.scan3d.fVzStart = fSwap(l+1);
    sScanParam.scan3d.fVzStop = fSwap(l+5);
    sScanParam.scan3d.fVzLimit = fSwap(l+9);
    sScanParam.scan3d.fVzArray = 0.0;  // bogus array
    sScanParam.scan3d.fVzSpeed1 = fSwap(l+17);
    sScanParam.scan3d.fVzSpeed2 = fSwap(l+21);
    sScanParam.scan3d.fVzSpeed3 = fSwap(l+25);
    sScanParam.scan3d.fVzSpeed4 = fSwap(l+29);
    sScanParam.scan3d.fVzPullback = fSwap(l+33);
    sScanParam.scan3d.iLayers = sSwap(l+37);    
    sScanParam.scan3d.iHalfCycles = sSwap(l+39);    
    sScanParam.scan3d.iAvgPoint = sSwap(l+41);        
    sScanParam.scan3d.fDelayStart = fSwap(l+43);
    sScanParam.scan3d.fDelaySample = fSwap(l+47);
    sScanParam.scan3d.fDelayPullback = fSwap(l+51);
    sScanParam.scan3d.fDelayEstFeedbk = fSwap(l+55);
    sScanParam.scan3d.bFeedbkPoints = (TOPOBOOL)sSwap(l+59);    
    sScanParam.scan3d.bFeedbkCurves = (TOPOBOOL)sSwap(l+61);    
    sScanParam.scan3d.bVzRelative = (TOPOBOOL)sSwap(l+63);        
    sScanParam.scan3d.fVzStart = fSwap(l+65);
    sScanParam.scan3d.fVzStart = fSwap(l+69);
    sScanParam.scan3d.iExtLayer = sSwap(l+73);            
    sScanParam.scan3d.bSpecialNCScan = (TOPOBOOL)sSwap(l+75);
    
    for(i=0;i<64;i++)
        sScanParam.szStageType[i] = *(const char *)(w+513+i);

    for(i=0;i<64;i++)
        sScanParam.szStageName[i] = *(const char *)(w+577+i);

    for(i=0;i<64;i++)
        sScanParam.szStageText[i] = *(const char *)(w+641+i);

    sScanParam.iOldOffsetX = usSwap(w+705);            
    sScanParam.iOldOffsetY = usSwap(w+707);            
    sScanParam.iOldDACminX = usSwap(w+709);            
    sScanParam.iOldDACmaxX = usSwap(w+711);            
    sScanParam.iOldDACminY = usSwap(w+713);            
    sScanParam.iOldDACmaxY = usSwap(w+715);                
    sScanParam.iNonContactMode = sSwap(w+717);                        
  */
}

TopoFile::TopoFile()
{
    header=NULL;
    valid_header=0;
    valid_grid=0;
    griddata = NULL;
    iHeaderLength = 0;
    iGridDataLength = 0;
    initForConversionToTopo(1.0, 0.0);
}

TopoFile::TopoFile(const TopoFile &t)
{
    iRelease = t.iRelease;
    iOffset = t.iOffset;
    memcpy(szRelease, t.szRelease, N_RELEASE);
    memcpy(szDatetime, t.szDatetime, N_DATETIME);
    memcpy(szDescription, t.szDescription, N_DESCRIPTION);
    iRows = t.iRows;
    iCols = t.iCols;
    fXmin = t.fXmin;
    fXmax = t.fXmax;
    fYmin = t.fYmin;
    fYmax = t.fYmax;
    fDACtoWorld = t.fDACtoWorld;
    fDACtoWorldZero = t.fDACtoWorldZero;
    iWorldUnitType = t.iWorldUnitType;
    iXYUnitType = t.iXYUnitType;
    memcpy(szWorldUnit, t.szWorldUnit, 10);
    memcpy(szXYUnit, t.szXYUnit, 10);
    iLayers = t.iLayers;
    rRoi = t.rRoi;
    valid_grid = vrpn_FALSE;
    iGridDataLength = 0;
    griddata = NULL;
    header = NULL;
    iHeaderLength = t.iHeaderLength;
    if (iHeaderLength > 0) {
       header = new char[iHeaderLength];
       if (!header) {
          iHeaderLength = 0;
          fprintf(stderr, "TopoFile::TopoFile: Error: out of memory (1)\n");
          return;
       }
       memcpy(header, t.header, iHeaderLength*sizeof(char));
    }
}

TopoFile &TopoFile::operator = (const TopoFile &t)
{
    iRelease = t.iRelease;
    iOffset = t.iOffset;
    memcpy(szRelease, t.szRelease, N_RELEASE);
    memcpy(szDatetime, t.szDatetime, N_DATETIME);
    memcpy(szDescription, t.szDescription, N_DESCRIPTION);
    iRows = t.iRows;
    iCols = t.iCols;
    fXmin = t.fXmin;
    fXmax = t.fXmax;
    fYmin = t.fYmin;
    fYmax = t.fYmax;
    fDACtoWorld = t.fDACtoWorld;
    fDACtoWorldZero = t.fDACtoWorldZero;
    iWorldUnitType = t.iWorldUnitType;
    iXYUnitType = t.iXYUnitType;
    memcpy(szWorldUnit, t.szWorldUnit, 10);
    memcpy(szXYUnit, t.szXYUnit, 10);
    iLayers = t.iLayers;
    rRoi = t.rRoi;
    valid_grid = vrpn_FALSE;
    iGridDataLength = 0;
    griddata = NULL;
    header = NULL;
    iHeaderLength = t.iHeaderLength;
    if (iHeaderLength > 0) {
       header = new char[iHeaderLength];
       if (!header) {
          iHeaderLength = 0;
          fprintf(stderr, "TopoFile::TopoFile: Error: out of memory (1)\n");
          return *this;
       }
       memcpy(header, t.header, iHeaderLength*sizeof(char));
    }
    return *this;
}

TopoFile::~TopoFile()
{
    if(header!=NULL) delete [] header;
    if(griddata!=NULL) delete [] griddata;
}
