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

#include "FileGenerator.h"
#include "WaveFrontFileGenerator.h"
#include "MSIFileGenerator.h"
#include "TubeFileGenerator.h"
#include "SpiderGenerator.h"
#include "DsAxisGenerator.h"
#include "ProjTextObjGenerator.h"
#include <stdio.h>
#include <string.h>


FileGenerator::FileGenerator(const char *fname, const char * exten)
 : GeometryGenerator(), extension(exten), filename(NULL)
{
    StoreFilename(fname);
}

void FileGenerator::StoreFilename(const char* fname){
    if (fname == NULL) {
        return;
    }

    int len=strlen(fname);
    if(strncmp(fname+(len-3),extension,3)!=0){
        fprintf(stderr, "FileGenerator of extension %s called on incorrect file type\n", extension);
        filename = NULL;
        return;
    }
    if(filename) delete []filename;
    filename=new char[strlen(fname)+1];
    strcpy(filename,fname);
    return;
}

FileGenerator* FileGenerator::CreateFileGenerator(const char *fname)
{
    int index,i;

    index=strlen(fname);		//save filename

    for(i=index-1; i>=0; i--){
        if(fname[i]=='.') break;
    }
    if(strncmp(fname+i+1,"obj",3)==0){
        return new WaveFrontFileGenerator(fname);
    }
	// added by David Borland for loading Shape Analysis tubes
	else if (strncmp(fname + i + 1,"txt", 3) == 0) {
		return new TubeFileGenerator(fname);
	}
	// added by David Borland for creating spiders
	else if (strncmp(fname + i + 1,"spi", 3) == 0) {
		return new SpiderGenerator(fname);
	}
	// added by David Borland for creating projective texture object
	else if (strncmp(fname + i + 1,"ptx", 3) == 0) {
		return new ProjTextObjGenerator(fname);
	}
    //added by Jameson Miller for loading axis object for direct step
	else if(strncmp(fname + i + 1,"dsa", 3) == 0) {
		return new DsAxisGenerator(fname);
	}
    //added by Leila Plummer for loading objects from tube_foundry
    else if (strncmp(fname+i+1,"msi",3)==0){
        //It seemed best to create the structure for loading .msi files
        //as a class, since it is time-efficient for much of the .msi
        //file's data to be stored for later use.  It would be logical
        //to change the WaveFrontFiles to a similar format
        return new MSIFileGenerator(fname);
    }
    else{
        fprintf(stderr, "File type not recognized for %s\n", fname);
    }	
    return (FileGenerator*)NULL;
}

