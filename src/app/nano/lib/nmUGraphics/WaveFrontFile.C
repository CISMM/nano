#include "URender.h"

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

void BuildList(ifstream& in, URender *Pobject,GLuint dl, double *vx, double *vy, double *vz,
					      double *vnx, double *vny, double *vnz,
					      double *vt_u, double *vt_v);

//This function reads a wavefront object file  -- though a limited subset
//it does only tri and quad face configurations 
// the f,v,vn,vt tags and the g object tag
// it will create an openGL display list for each logical object
// as defined by the 'g' tag

int LoadWaveFrontFile(URender *Pobject, char *filename, GLuint *&Dlist_array)
{
        char buf[500];
        ifstream readfile;  int i;

	double *vx, *vy, *vz;		//vertex position      
	double *vt_u, *vt_v;            //texture coords
	double *vnx, *vny, *vnz;  	//vertex normals

        long vc,vnc,vtc;
        long facecount;
	long objcount;
        long vertcount;
	char **objname;
        GLuint dl;

        //open Data file
        readfile.open(filename);
        assert(readfile);

        if(readfile.bad()){
                cerr << "Unable to open input file " << buf << "\n";
                return 0;

        }

        //PHASE 1:    scan file to get vertex counts and facecounts
	objcount=0;facecount=0;
        vertcount=0;i=-1;
        vc=vnc=vtc=0;
        while(!readfile.eof()){
                readfile.getline(buf,500);
                if(buf[0]=='v' && buf[1]==' ') vertcount++;
                else if(buf[0]=='g') objcount++;
                else if(buf[0]=='v' && buf[1]=='t') vtc++;
                else if(buf[0]=='v' && buf[1]=='n') vnc++;
                else if(buf[0]=='f') facecount++;
                buf[0]='\0';
        }
        if(vnc>vertcount) vertcount=vnc;
        if(vtc>vertcount)vertcount=vtc;

	//reset to beginning of file
        readfile.seekg(0,ios::beg);
        readfile.clear();

        //allocate data structures
        vx=new double[vertcount];
        vy=new double[vertcount];
        vz=new double[vertcount];
        vt_u=new double[vertcount];
        vt_v=new double[vertcount];
        vnx=new double[vertcount];
        vny=new double[vertcount];
        vnz=new double[vertcount];
	objname=new char*[objcount];
        if(!(vx && vy && vz && vt_u && vt_v && vnx && vny && vnz && objcount)){
                cerr << "Unable to allocate sufficient memory store for WAVEFRONT file\n";
		//kill(getpid(),SIGINT);
		return 0;
        }


	//PHASE 2: store the vertex positions normals and texture coordinates
        i=-1;     
	//count for which object I'm on -- expect g <objname> to be first thing in a file
        vc=vnc=vtc=0;   //counters for how many vertexes,normals, and tex coord encountered
        while(!readfile.eof()){
                readfile.getline(buf,500);
                if(buf[0]=='g'){
                        i++;
                        objname[i]=new char[strlen(buf)+1];
                        if(objname[i]==NULL){
                                cerr << "Error allocating objectname\n";
                                //kill(getpid(),SIGINT);
				return 0;
                        }
                        strcpy(objname[i],buf+2);
                }
                else if(buf[0]=='v' && buf[1]==' '){
                        sscanf(buf,"v %lf %lf %lf",vx+vc,vy+vc,vz+vc);
                        vc++;
                }
                else if(buf[0]=='v' && buf[1]=='t'){
                        sscanf(buf,"vt %lf %lf",vt_u+vtc,vt_v+vtc);
                        vtc++;
                }
                else if(buf[0]=='v' && buf[1]=='n'){
                        sscanf(buf,"vn %lf %lf %lf",vnx+vnc,vny+vnc,vnz+vnc);
                        vnc++;
                }
                else if(buf[0]=='f'){
                        //skip f lines in your counting... get it on the next pass
                }

                else{
                        cerr << "Unexpected token -- " << buf << "\n";
                }
        }

	//reset file ptr
        readfile.seekg(0,ios::beg);
        readfile.clear();

	//set up display list id's
	Dlist_array=new GLuint[objcount];
        dl=glGenLists(objcount);
        if(dl==0 || Dlist_array==NULL){ cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
                   return 0;
	}

	//TO OPTIMIZE: could do some normal smoothing operations here, or 
	//vertex removal
	//perhaps try vertex array extension since the data is nominally in 
	//that format anyway

        for(i=0; i < objcount;i++){
		//BuildList actually builds the geometry from
		//the data structures previously built
                BuildList(readfile,Pobject,dl+i,vx,vy,vz,vnx,vny,vnz,vt_u,vt_v);
		//it will modify the readfile file ptr location as a side effect
		//which *should* leave it in an appropriate place for the next
		//object to be built
		Dlist_array[i]=dl+i;
        }

	//clean up after myself
        delete []vx, delete []vy; delete []vz;
        delete []vnx; delete []vny; delete []vnz;
        delete []vt_u; delete []vt_v;
        readfile.close();

        return objcount;
}

void BuildList(ifstream& in, URender *Pobject,GLuint dl, double *vx, double *vy, double *vz,
					      double *vnx, double *vny, double *vnz,
					      double *vt_u, double *vt_v)
{
        int mode=0;             //mode=0 uninitialized, 1=triangles, 2=quads
        char buf[200];
        char sbuf1[30],sbuf2[30],sbuf3[30],sbuf4[30];
        int j;
        long id1,id2,id3;

	//mode bit is used to track if I am processing a quad or a triangle
	//and to glBegin/End the appropriate type when necessary


	//file structure should be  
	// g <name>
	// v <x> <y> <z>
	//	.
	//	.
	// vt <u> <v>		(these can be any order)
	//	.
	//	.
	// vn <nx> <ny> <nz>
	//	.
	// 	.
	// f A/B/C D/E/F G/H/I	(A-I are indexes into the v,vt,vn arrays)
	//      . 
	//	.
	//      .
	// g <name>
	//      . 
	//	.
	//      .

        //seek to a face group marked with an 'f' tag
        buf[0]='\0';
        while(buf[0]!='f' && !in.eof()) in.getline(buf,100);

        glNewList(dl,GL_COMPILE);	//init display list
        while(buf[0]=='f' && !in.eof()){
                j=sscanf(buf,"f %s %s %s %s",sbuf1,sbuf2,sbuf3,sbuf4);
                if(j==3){
                        if(mode==0){glBegin(GL_TRIANGLES); mode=1;}
                        else if(mode==2){glEnd();glBegin(GL_TRIANGLES);mode=1;}
                        sscanf(sbuf1,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 1
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);
                        sscanf(sbuf2,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 2
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);
                        sscanf(sbuf3,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 3
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);
                }
                else if(j==4){
                        if(mode==0){glBegin(GL_QUADS);mode=2;}
                        if(mode==1){glEnd();glBegin(GL_QUADS);mode=2;}

                        sscanf(sbuf1,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 1
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);

                        sscanf(sbuf2,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 2
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);

                        sscanf(sbuf3,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 3
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);

                        sscanf(sbuf4,"%ld/%ld/%ld",&id1,&id2,&id3);                     //vertex 4
                        glNormal3f(vnx[id3-1],vny[id3-1],vnz[id3-1]);
                        glTexCoord2d(vt_u[id2-1],vt_v[id2-1]);
                        glVertex3d(vx[id1-1],vy[id1-1],vz[id1-1]);
                        Pobject->UpdateBoundsWithPoint(vx[id1-1],vy[id1-1],vz[id1-1]);
                }
                else{
                        cerr << "Unexpected number of points in face\n";
                }
                in.getline(buf,200);
        }
        glEnd();
        glEndList();
        return;

}





