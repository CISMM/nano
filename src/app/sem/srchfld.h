#ifndef SRCHFLD_H
#define SRCHFLD_H

extern "C" {
   typedef struct tagSTGLOCA {
		long x;
		long y;
		long z;
		long r;
		long t;
		long w;
		} STGLOCA;
   typedef struct tagSTGDEF {
		int model;          /* 1=GALIL,... */
		int isEnabled;	/* control:0=disabled,1=enabled */
		int ioComm;         /* 1=COMM1, 2=COMM2	*/
		int dataBits;
		int stopBits;
		int baudRate;
		char parity;
		char dummy;
		int timeOut;		/* timeout for stage Timer in millisec. */
		int pollCount;		/* maximum number of pollings	*/
		int param[4];		/* not used */
		STGLOCA limit1;
		STGLOCA limit2;
		STGLOCA error;      /* stage resolution [microns]	*/
		float usecPerLoop;
		float xmitDelay;   /* milliseconds */
		float recvDelay;   /* milliseconds */
		} STGDEF;
   typedef struct tagCLMLOCA {
		int iScopeLoca;		/* location in the scope location table */
		long kv;
		long mag;
		long crt;
		long spt;
		float t;	/* unused */
		float u;	/* unused */
		float v;	/* unused */
		float w;	/* unused */
		} CLMLOCA;
   typedef struct tagCLMDEF {
		int model;          /* 0=ABT_32,1=ABT_60,... */
		int isEnabled;	/* control:0=disabled,1=enabled */
		int ioComm;         /* 1=COMM1, 2=COMM2	*/
		int dataBits;
		int stopBits;
		int baudRate;
		char parity;
		char dummy;
		int timeOut;		/* timeout for column Timer in millisec. */
		float usecPerLoop;
		float xmitDelay;   /* milliseconds */
		float recvDelay;   /* milliseconds */
		} CLMDEF;
 typedef struct tagSRCHLOCA {
   float version;
   int lowThre;	/* lower gray level threshold for particle search */
   int uppThre;	/* upper gray level threshold for particle search */
   float minSize;	/* minimum particle size in microns */
   float maxSize;	/* maximum particle size in microns */
   float mppX;	    /* microns per pixel in X direction */
   float mppY;	    /* microns per pixel in Y direction */
   float preset;    /* preset time for EDS collection	*/
   int saveAllSpectra;
   int saveAllImages;
   int i1;			/* output format: 0=ZEP,1=CSV,2=ZEP&CSV */
   int i2;			/* search control: 0=disabled,1=enabled */
   int i3;	/* unused */
   int i4;	/* unused */
   float f1;	/* unused */
   float f2;	/* unused */
   float f3;	/* unused */
   float f4;	/* unused */
   } SRCHLOCA;
 typedef struct tagSRCHFLD {
	STGLOCA 	stgLoca;
	CLMLOCA 	clmLoca;
	SRCHLOCA 	srchLoca;
	} SRCHFLD;

};
#endif



