#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Topo.h"
#include "afmstm.h"

void TopoFile::parseReleaseString( char *release)
/*****************************************************************
* Assumes the .release is filled in.  Looks for #Rn.n# OFFSET
* Fills in .nRelease with n.nn (2.30 = 230, 3.00 = 300, etc)
* If .nRelease > 230, OFFSET is parsed.  This is the fixed offset
* provided in this file.  This will be fixed for each release.
* v3.00:        offset = 1024 bytes
* v3.01:        offset = 1536 bytes
*****************************************************************/
{       char    *rel, *p, rel1[N_RELEASE];
        INT     ver, ver1, offset;

    memcpy( rel1, release, N_RELEASE);          // copy of release string
    rel = &rel1[0];
    nRelease = 100;                            // assume a real old file!
    nOffset  = 0;                              // with no offset
    offset    = 0;
    if( *rel++ != '#')              return;
    if( *rel != 'R' && *rel != 'r') return;
    p = ++rel;
    while( *rel != '\0' && *rel != '.') rel++;
    if( *rel == '\0')   return;                 // no version #
    *rel = '\0';
    ver  = atoi( p);                            // major release number
    if( ver <= 0)   return;                     // versions 1 .... N
    p = ++rel;
    while( *rel != '\0' && *rel != '#') rel++;
    if( *rel == '\0')   return;                 // no valid release string
    *rel = '\0';
    if( strlen( p) == 1) {                      // make sure it is always two digits
        *rel++ = '0';
        *rel   = '\0';
        }
    ver1 = atoi( p);                            // minor release number
    if( ver1 < 0 || ver1 > 99)  return;         // invalid minor version!
    ver = ver * 100 + ver1;                     // our version
    if( ver > 230) {                            // get offset for files newer than 2.3
        p = ++rel;
        while( *p != '\0' && *p <= ' ') p++;    // skip spaces
        offset = atoi( p);
        if( offset < 0) offset = 0;
        }
    nRelease = ver;
    nOffset  = offset;
} // end of ParseFileReleaseString



int TopoFile::ParseHeader(const char *buf,long length){
	int i;
	char *temp;
	char s[100];
	int size=100;

	if(header!=buf){
		if(header!=NULL) delete header;
		header = new char[length];
		if(header==NULL){
			perror("Unable to allocate memory for header "
			       "buffer.  Aborting.\n");
	                 exit(-1);
		}
		memcpy(header,buf,length);
	}

	this->valid_header=1;
	temp=header;
	/*get release string*/
	i=0;
        while( (s[i]=header[i]) != '\n' && i < size) i++;
        s[i] = '\0';
	parseReleaseString(s);

	if(nRelease > 300){
		temp=header+256;
		sInfo.iRelease=nRelease;
		sInfo.iOffset=nOffset;
		if(nRelease < N_RELEASE) FixUpDocStruct();
		temp=ParseDocInfo(temp);
		temp=ParseScanParam(temp);
	}
	else{
		printf("Unable to parse version files less than 3.01\n");
		this->valid_header=0;
		exit(-1);
		
	}
	return(0);	
}

int TopoFile::ParseHeader(int in_descriptor){
        int     i;
	char     s[100];
	int size=100;

	printf("Parsing data file\n");
        if(lseek(in_descriptor, 0, SEEK_SET) == -1) {   /* Error */
                perror("TopoHeader::readTopometrixFile(): Can't seek to zero");
                return -1;
        }

	/* get release string */
	i=0;
        while( read( in_descriptor, &s[i], 1) > 0 && s[i] != '\n' && i < size) i++;
        s[i] = '\0';
	parseReleaseString(s);

	header=new char[nOffset];
	if(header==NULL){ perror("Unable to allocate room for header\n"); exit(-1);}

        if(lseek(in_descriptor, 0, SEEK_SET) == -1) {   /* Error */
                perror("TopoHeader::readTopometrixFile(): Can't seek to zero");
                return -1;
        }
	if (read(in_descriptor,header,sizeof(char)*nOffset) != nOffset*sizeof(char)) {
		fprintf(stderr,"ParseHeader: Error during read\n");
		return -1;
	}	
	ParseHeader(header,nOffset);	
	return 1;
}

int TopoFile::iSwap(char *i)
{
#ifdef	linux
	int	out;
	out=(int)*i;
#else
	static int out;
	char *o;

	o = (char*)(&out);
	*o = *(i+3);
	*(o+1) = *(i+2);
	*(o+2) = *(i+1);
	*(o+3) = *i;
#endif
	return(out);
}

double TopoFile::dSwap(char *i)
{
#ifdef	linux
	double out;
	out=(double)*i;
#else
	static	double out;
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

float	TopoFile::fSwap(char *i)
{
#ifdef	linux
	float	out;
	out=(float)*i;
#else
	static	float  out;
	char *o;

      	o = (char*)(&out);
	*o = *(i+3);
	*(o+1) = *(i+2);
	*(o+2) = *(i+1);
	*(o+3) = *i;
#endif
	return(out);
}

short	TopoFile::sSwap(char* i)
{
#ifdef	linux
	short	out;
	out=(short)*i;
#else
	static	short	out;
	char	*o;
	
	o = (char*)(&out);
	*o = *(i+1);
	*(o+1) = *i;
#endif
	return(out);
}

unsigned short TopoFile::usSwap(char *i)
{
#ifdef	linux
	unsigned short	out;
	out=(unsigned short) *i;
#else
	static unsigned short out;
	char   *o;

	o = (char*)(&out);
	*o = *(i+1);
	*(o+1) = *i;
#endif
	return(out);
}

char *TopoFile::ParseDocInfo(char *temp){
	int i;
	sInfo.iRelease = iSwap(temp); temp+=sizeof(int);
	sInfo.iOffset = iSwap(temp); temp+=sizeof(int);
	for(i=0; i< sizeof(sInfo.szRelease); i++){ sInfo.szRelease[i]= *(temp+i);} temp+=sizeof(sInfo.szRelease);
	for(i=0; i< sizeof(sInfo.szDatetime); i++){ sInfo.szDatetime[i]= *(temp+i);} temp+=sizeof(sInfo.szDatetime);
	for(i=0; i< sizeof(sInfo.szDescription); i++){ sInfo.szDescription[i]= *(temp+i);} temp+=sizeof(sInfo.szDescription);
	for (i = 0; i < N_MAX_GRAPHS; i++) {sInfo.fPosX[i] = fSwap(temp); temp+=sizeof(float);}
	for (i = 0; i < N_MAX_GRAPHS; i++) {sInfo.fPosY[i] = fSwap(temp); temp+=sizeof(float);}
	sInfo.iCurves = sSwap(temp); temp+=sizeof(short);
	sInfo.iRows = iSwap(temp);temp+=sizeof(int);
	sInfo.iCols = iSwap(temp);temp+=sizeof(int);
	sInfo.iDACmax = sSwap(temp);temp+=sizeof(short);
	sInfo.iDACmin = sSwap(temp);temp+=sizeof(short);
	sInfo.fXmin = fSwap(temp); temp+=sizeof(float);
	sInfo.fXmax = fSwap(temp); temp+=sizeof(float);
	sInfo.fYmin = fSwap(temp); temp+=sizeof(float);
	sInfo.fYmax = fSwap(temp); temp+=sizeof(float);
	sInfo.fDACtoWorld = fSwap(temp); temp+=sizeof(float);
	sInfo.fDACtoWorldZero = fSwap(temp); temp+=sizeof(float);
	sInfo.iDACtoColor = usSwap(temp); temp+=sizeof(unsigned short);
	sInfo.iDACtoColorZero = usSwap(temp); temp+=sizeof(unsigned short);
	sInfo.iWorldUnitType = sSwap(temp); temp+=sizeof(short);
	sInfo.iXYUnitType = sSwap(temp); temp+=sizeof(short);
	for(i=0; i< sizeof(sInfo.szWorldUnit);i++){ sInfo.szWorldUnit[i]=*(temp+i);}temp+=sizeof(sInfo.szWorldUnit);
	for(i=0; i< sizeof(sInfo.szXYUnit);i++){ sInfo.szXYUnit[i]=*(temp+i);}temp+=sizeof(sInfo.szXYUnit);
	for(i=0; i< sizeof(sInfo.szRateUnit);i++){ sInfo.szRateUnit[i]=*(temp+i);}temp+=sizeof(sInfo.szRateUnit);
	sInfo.iLayers = sSwap(temp);temp+=sizeof(short);
	sInfo.bHasEchem = sSwap(temp);temp+=sizeof(short);
	sInfo.bHasBkStrip = sSwap(temp);temp+=sizeof(short);
	for (i = 0; i < N_MAX_GRAPHS; i++){ sInfo.iPts[i] = sSwap(temp);temp+=sizeof(short);}
	sInfo.iXUnitType = sSwap(temp);temp+=sizeof(short);
	for(i=0; i< sizeof(sInfo.szXUnit);i++){ sInfo.szXUnit[i]=*(temp+i);}temp+=sizeof(sInfo.szXUnit);

	/* version 4.0 fixes */
	if(nRelease < 400){
		for(i=0; i< 510;i++){ sInfo.cFiller[i]=*(temp+i);} temp+=510;
	}
	else{
		sInfo.bHasAcqDisplay= sSwap(temp);temp+=sizeof(short);
		sInfo.iTilt = sSwap(temp);temp+=sizeof(short);
		sInfo.iScaleZ = sSwap(temp);temp+=sizeof(short);
		sInfo.iFilter = sSwap(temp);temp+=sizeof(short);
		sInfo.iShading = sSwap(temp);temp+=sizeof(short);
		for (i = 0; i < 8; i++) {sInfo.dTiltC[i] = dSwap(temp); temp+=sizeof(double);}
		sInfo.iDACDisplayZero= usSwap(temp);temp+=sizeof(unsigned short);
		sInfo.iDACDisplayRange= usSwap(temp);temp+=sizeof(unsigned short);
		sInfo.rRoi.left=sSwap(temp);temp+=sizeof(short);
		sInfo.rRoi.top=sSwap(temp);temp+=sizeof(short);
		sInfo.rRoi.right=sSwap(temp);temp+=sizeof(short);
		sInfo.rRoi.bottom=sSwap(temp);temp+=sizeof(short);
		for(i=0; i< 424;i++){ sInfo.cFiller[i]=*(temp+i);} temp+=424;
	}
	return temp;
}

char* TopoFile::ParseScanParam(char *temp){
        int i;

	sScanParam.iDataType = sSwap(temp); temp+=sizeof(short);
	sScanParam.iDataDir = sSwap(temp); temp+=sizeof(short);
	sScanParam.iDataMode = sSwap(temp); temp+=sizeof(short);
	sScanParam.fScanZmax = fSwap(temp);temp+=sizeof(float);
	sScanParam.fScanZmin = fSwap(temp);temp+=sizeof(float);
	sScanParam.fScanXmax = fSwap(temp);temp+=sizeof(float);
	sScanParam.fScanYmax = fSwap(temp);temp+=sizeof(float);
	sScanParam.fVtip = fSwap(temp);temp+=sizeof(float);
	sScanParam.fI = fSwap(temp);temp+=sizeof(float);
	sScanParam.fVz = fSwap(temp);temp+=sizeof(float);
	sScanParam.fRange = fSwap(temp);temp+=sizeof(float);
	sScanParam.fRate = fSwap(temp);temp+=sizeof(float);
	sScanParam.iGain = sSwap(temp); temp+=sizeof(short);
	sScanParam.fPro = fSwap(temp);temp+=sizeof(float);
	sScanParam.fInteg = fSwap(temp);temp+=sizeof(float);
	sScanParam.fDer = fSwap(temp);temp+=sizeof(float);
	sScanParam.iGainZ = sSwap(temp);temp+=sizeof(short);
	sScanParam.fRotation = fSwap(temp);temp+=sizeof(float);
	sScanParam.fModLevel = fSwap(temp);temp+=sizeof(float);
	sScanParam.fAveraging = fSwap(temp);temp+=sizeof(float);
	sScanParam.fSpCalFactor = fSwap(temp);temp+=sizeof(float);
	sScanParam.iCalibType = sSwap(temp);temp+=sizeof(short);
	sScanParam.iLaserIntensity = sSwap(temp);temp+=sizeof(short);
	sScanParam.iScaleFactorZ = usSwap(temp);temp+=sizeof(unsigned short);
	sScanParam.iDACminX = usSwap(temp);temp+=sizeof(unsigned short);
	sScanParam.iDACmaxX = usSwap(temp);temp+=sizeof(unsigned short);
	sScanParam.iDACminY = usSwap(temp);temp+=sizeof(unsigned short);
	sScanParam.iDACmaxY = usSwap(temp);temp+=sizeof(unsigned short);
	for(i=0; i< sizeof(sScanParam.cScanType);i++){ sScanParam.cScanType[i]=*(temp+i);}temp+=sizeof(sScanParam.cScanType);
	sScanParam.iProbeType = sSwap(temp);temp+=sizeof(short);
	sScanParam.iStageType = sSwap(temp);temp+=sizeof(short);
	sScanParam.iCalFileSource = sSwap(temp);temp+=sizeof(short);
	sScanParam.fOverscanX = fSwap(temp);temp+=sizeof(float);
	sScanParam.fOverscanY = fSwap(temp);temp+=sizeof(float);

	/* version 4.0 additions */
	if(nRelease < 400){
		for(i=0; i< 148;i++){ sScanParam.cFiller[i]=*(temp+i);} temp+=148;
		temp=ParseScanLayers(temp);
	}
	else{
		sScanParam.iSetPointUnits=sSwap(temp);temp+=sizeof(short);
		sScanParam.fNcRegAmp= fSwap(temp);temp+=sizeof(float);
		sScanParam.iGainXY= sSwap(temp);temp+=sizeof(short);
		sScanParam.iOffsetX = usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOffsetY = usSwap(temp);temp+=sizeof(unsigned short);
		for(i=0; i < 4; i++){ sScanParam.fHysteresisX[i]= fSwap(temp);temp+=sizeof(float);}
		for(i=0; i < 4; i++){ sScanParam.fHysteresisY[i]= fSwap(temp);temp+=sizeof(float);}
		sScanParam.iOffsetZ = usSwap(temp);temp+=sizeof(unsigned short);
		for(i=0; i < 4; i++){ sScanParam.fHysteresisZ[i]= fSwap(temp);temp+=sizeof(float);}
		sScanParam.fCrossTalkCoef= fSwap(temp);temp+=sizeof(float);
		sScanParam.fSensorResponse = fSwap(temp);temp+=sizeof(float);
		sScanParam.fKc = fSwap(temp);temp+=sizeof(float);
		sScanParam.iCantileverType = sSwap(temp);temp+=sizeof(short);
	 	for(i=0; i< 16;i++){ sScanParam.szScannerSerialNumber[i]=*(temp+i);}temp+=16;
		sScanParam.iZlinearizer = sSwap(temp);temp+=sizeof(short);
		sScanParam.iADC = iSwap(temp);temp+=sizeof(int);
		sScanParam.bNonContact = sSwap(temp);temp+=sizeof(short);
		sScanParam.CantileverType = sSwap(temp);temp+=sizeof(short);
		sScanParam.fDriveAmplitude= fSwap(temp);temp+=sizeof(float);
		sScanParam.fDriveFrequency= fSwap(temp);temp+=sizeof(float);
		sScanParam.iNonContactMode = sSwap(temp);temp+=sizeof(short);
		sScanParam.iNonContactPhase = sSwap(temp);temp+=sizeof(short);
	 	for(i=0; i< 34;i++){ sScanParam.cFiller[i]=*(temp+i);}
		temp+=34;
		temp=ParseScanLayers(temp);
	 	for(i=0; i<64;i++){ sScanParam.szStageType[i]=*(temp+i);} temp+=64;
	 	for(i=0; i<64;i++){ sScanParam.szStageName[i]=*(temp+i);} temp+=64;
	 	for(i=0; i<64;i++){ sScanParam.szStageText[i]=*(temp+i);} temp+=64;
		sScanParam.iOldOffsetX = usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldOffsetY = usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldDACminX = usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldDACmaxX= usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldDACminY = usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldDACmaxY= usSwap(temp);temp+=sizeof(unsigned short);
		sScanParam.iOldGainXY = sSwap(temp);temp+=sizeof(short);
	 	for(i=0; i< 370;i++){ sScanParam.cFiller1[i]=*(temp+i);}
		temp+=370;
	}
	
	return temp;
}

char *TopoFile::ParseScanLayers(char* temp){
        int i;
        SCANPARAMSLAYER *scan3d;
	scan3d=&sScanParam.scan3d;

	scan3d->fVzStart = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzStop = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzLimit = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzArray = fSwap(temp); temp+=sizeof(float);	/* XXX BOGUS - but was impossible to read pointer from a file anyway */
	scan3d->fVzSpeed1 = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzSpeed2 = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzSpeed3 = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzSpeed4 = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzPullback = fSwap(temp);temp+=sizeof(float);
	scan3d->iLayers = sSwap(temp); temp+=sizeof(short);
	scan3d->iHalfCycles = sSwap(temp);temp+=sizeof(short);
	scan3d->iAvgPoint = sSwap(temp);temp+=sizeof(short);
	scan3d->fDelayStart = fSwap(temp);temp+=sizeof(float);
	scan3d->fDelaySample = fSwap(temp);temp+=sizeof(float);
	scan3d->fDelayPullback = fSwap(temp);temp+=sizeof(float);
	scan3d->fDelayEstFeedbk = fSwap(temp);temp+=sizeof(float);
	scan3d->bFeedbkPoints = sSwap(temp);temp+=sizeof(short);
	scan3d->bFeedbkCurves = sSwap(temp);temp+=sizeof(short);
	scan3d->bVzRelative = sSwap(temp);temp+=sizeof(short);
	scan3d->fModFreq = fSwap(temp);temp+=sizeof(float);
	scan3d->fVzMod = fSwap(temp);temp+=sizeof(float);
	
	/* version 4.0 addtions */
	if(nRelease < 400){
		for(i=0; i < 184;i++){ scan3d->cFiller[i]=*(temp+i);}
		temp+=184;
	}
	else{
		scan3d->iExtLayer = sSwap(temp);temp+=sizeof(short);
		scan3d->bSpecialNCScan = sSwap(temp);temp+=sizeof(short);
		for(i=0; i < 180;i++){ scan3d->cFiller[i]=*(temp+i);}
		temp+=180;
	}
	return temp;

}
int TopoFile::PrintData(){
	int i,j;
	int offset;
	if(!valid_grid) return -1;
	for(i=0; i < sInfo.iRows; i++){
		offset=i*sInfo.iCols;
		for(j=0; j < sInfo.iCols; j++){
			printf("%u,",(unsigned short)griddata[offset+j]);
		}
		printf("\n");
	}
	return 1;
}




int TopoFile::PrintDocInfo(){

	if(!valid_header){
		printf("Not a valid header\n"); return -1;
	}

	char buf[500];
	int i;
	sprintf(buf,"Release: %d\n\t%s\nHeader Offset: %d\n",sInfo.iRelease, sInfo.szRelease,sInfo.iOffset);
	sprintf(buf,"%sDate: %s\nDescription: %s\n",buf,sInfo.szDatetime, sInfo.szDescription);
	printf("%s",buf);
	sprintf(buf,"");
	for(i=0;i<N_MAX_GRAPHS;i++) {sprintf(buf,"%sCurve(%d) <X,Y>: <%f,%f>\n",buf,i,sInfo.fPosX[i], sInfo.fPosY[i]);}
	sprintf(buf,"%sGrid Size <X,Y>: <%d,%d>\n",buf,sInfo.iRows, sInfo.iCols);
	printf("%s",buf);
	sprintf(buf,"Data Min/Max: <%d,%d>\n",sInfo.iDACmin,sInfo.iDACmax);
	sprintf(buf,"%sScan DataX Min/Max: <%f,%f>\n",buf,sInfo.fXmin,sInfo.fXmax);
	sprintf(buf,"%sScan DataY Min/Max: <%f,%f>\n",buf,sInfo.fYmin,sInfo.fYmax);
	printf("%s",buf);

	sprintf(buf,"DAC to World: %f\nDAC to World Zero: %f\nDAC to color: %d\nDAC to color Zero: %d\n", sInfo.fDACtoWorld, sInfo.fDACtoWorldZero, sInfo.iDACtoColor, sInfo.iDACtoColorZero);
	printf("%s",buf);
	sprintf(buf,"World Units: %s (%d)\nXY Units: %s (%d)\nRate unit: %s\n",sInfo.szWorldUnit, sInfo.iWorldUnitType, sInfo.szXYUnit, sInfo.iXYUnitType, sInfo.szRateUnit);
	printf("%s",buf);
	
	sprintf(buf,"Layers: %d\nEChem Data: %s\nbkStrip Data: %s\n", sInfo.iLayers,(sInfo.bHasEchem == 0 ? "FALSE" : "TRUE"), (sInfo.bHasBkStrip ? "FALSE" : "TRUE"));
	for(i=0; i < N_MAX_GRAPHS; i++){ sprintf(buf,"%spoints in Curve(%d): %d\n",buf,i,sInfo.iPts[i]); }
	sprintf(buf,"%sPhysical units in X: %s\n",buf,sInfo.szXUnit);
	printf("%s",buf);
	return 1;
}

int TopoFile::PrintScanParams(){
	if(!valid_header) return-1;

	char buf[500];

	sprintf(buf,"Raw Data Source (type): %d\nScan directions: %d\n",sScanParam.iDataType,sScanParam.iDataDir);
	sprintf(buf,"%sScan datamode: %f\nZ DAC max: %f\n",buf,sScanParam.iDataMode, sScanParam.fScanZmax);
	sprintf(buf,"%sZ DAC min: %f\nX piezo resol.: %f\n",buf,sScanParam.fScanZmin, sScanParam.fScanXmax);
	sprintf(buf,"%sY piezo resol.: %f\nSTM tip voltage (-10,10)volts: %f\n",buf,sScanParam.fScanYmax, sScanParam.fI);
	printf("%s",buf);

	sprintf(buf,"Scanned Image range: %f\nScan rate (unit/sec): %f\n",sScanParam.fRange,sScanParam.fRate);
	sprintf(buf,"%sADC gain flag: %d\nPID Pro: %f\n",buf,sScanParam.iGain,sScanParam.fPro);
	sprintf(buf,"%sPID Integ: %f\nPID Der: %f\n",buf,sScanParam.fInteg,sScanParam.fDer);
	sprintf(buf,"%sZ piezo gain flag: %d\nXY scan rotation (radians): %f\n",buf,sScanParam.iGainZ,sScanParam.fRotation);
	printf("%s",buf);

	sprintf(buf,"Modulation mode (angstroms): %f\n# points to average/img point: %f\n",sScanParam.fModLevel,sScanParam.fAveraging);
	sprintf(buf,"%sSP Calibration Factor: %f\nXY Calibration Type: %d\n",buf,sScanParam.fSpCalFactor,sScanParam.iCalibType);
	sprintf(buf,"%sLaserIntensity: %d\nZ scale factor: %d\n",buf,sScanParam.iLaserIntensity,sScanParam.iScaleFactorZ);
	printf("%s",buf);

	sprintf(buf,"piezo scan location min x: %d\npiezo scan max x: %f\n",sScanParam.iDACminX,sScanParam.iDACmaxX);
	sprintf(buf,"%spiezo scan location min y: %d\npiezo scan max y: %f\n",buf,sScanParam.iDACminY,sScanParam.iDACmaxY);
	sprintf(buf,"%sscantype %s: %d\nProbe type: %f\n",buf,sScanParam.cScanType,sScanParam.iProbeType);
	sprintf(buf,"%sStageType%s: %d\nCalibration file type: %d\n",buf,sScanParam.iStageType,sScanParam.iCalFileSource);
	sprintf(buf,"%sOverscan range in X (%%): %f\nOverscan range in Y (%%): %f\n",buf,sScanParam.fOverscanX,sScanParam.fOverscanY);
	printf("%s",buf);
	return 1;

}

int TopoFile::PrintScanLayers(){
	if(!valid_header) return -1;

	char buf[500];
	PSCANPARAMSLAYER p;
	p=&(sScanParam.scan3d);	

	sprintf(buf,"Voltage dist start: %f\nVoltate dist stop: %f\nForce limit: %f\n",p->fVzStart,p->fVzStop,p->fVzLimit);
	printf("%s",buf);

	sprintf(buf,"speed volt/dist ramp 1 (V/sec or micr/sec): %f\n",p->fVzSpeed1);
	sprintf(buf,"%sspeed volt/dist ramp 2 (V/sec or micr/sec): %f\n",buf,p->fVzSpeed2);
	sprintf(buf,"%sspeed volt/dist ramp 3 (V/sec or micr/sec): %f\n",buf,p->fVzSpeed3);
	sprintf(buf,"%sspeed volt/dist ramp 4 (V/sec or micr/sec): %f\n",buf,p->fVzSpeed4);
	sprintf(buf,"%spullback distance: %f\n",buf,p->fVzPullback);
	printf("%s",buf);

	sprintf(buf,"# Layers: %hd\n# of halfcycles (for=1, for+back=2): %d\n",p->iLayers,p->iHalfCycles);
	sprintf(buf,"%sAveragine num/layer point: %d\nDelay time before 1st sample (micr-sec): %f\n",buf,p->iAvgPoint,p->fDelayStart);
	sprintf(buf,"%sDelay time before sample (micr-sec): %f\nDelay time before pullback (micr-sec): %f\n",buf,p->fDelaySample,p->fDelayPullback);
	printf("%s",buf);

	sprintf(buf,"Delay time before feeback established (micr-sec): %f\nFeedBack enabled: %d\n",p->fDelayEstFeedbk,p->bFeedbkPoints);
	sprintf(buf,"%sFeedBack between curves enabled: %d\nVolt/dist values relative: %d\n",buf,p->bFeedbkCurves,p->bVzRelative);
	sprintf(buf,"%sModulation frequency: %f\n",buf,p->fVzMod);
	printf("%s",buf);
	return 1;
}

int TopoFile::ParseData(int handle){
        int     i,size;
	
	if(!valid_header) return -1;



	/* Seeking to where the data should start - this was not done in the
         * code from Topometrix. */
        printf("  Reading data starting at byte %d\n",sInfo.iOffset);
	if( lseek( handle, sInfo.iOffset, SEEK_SET) == -1) { /* Error */
                return NULL;
        }


        size = sInfo.iCols * sInfo.iRows * sInfo.iLayers * sizeof( unsigned short);
        griddata = ((unsigned short *)malloc(size));
	if(griddata == NULL) {
                fprintf(stderr,"TopoFile::ParseData(): Out of memory\n");
                fprintf(stderr,"  (Can't allocate %dx%d grid with %d layers)\n",
                        sInfo.iCols, sInfo.iRows, sInfo.iLayers);
		return -1;
        }

        /* Seeking to where the data should start - this was not done in the
         * code from Topometrix. */
        if( lseek( handle, sInfo.iOffset, SEEK_SET) == -1) { /* Error */
		printf("TopoFile::ParseData File seek error\n");
		free((unsigned short*)griddata);
		return -1;
		
        }

        if ( read(handle, (char*)griddata, size) != size){
		printf("TopoFile::ParseData didn't read all of the data -- error\n");
		free((short*)griddata);
		return -1;
        }

        for (i = 0; i < size/sizeof(unsigned short); i++) griddata[i] = usFix( &griddata[i] );
	valid_grid=1;

        return(0);
}


unsigned short   TopoFile::usFix(unsigned short *s)
{
#ifdef  linux
        return *s;
#else
        static  unsigned short   out;
        char    *i, *o;

        i = (char*)(s);
        o = (char*)(&out);
        *o = *(i+1);
        *(o+1) = *i;

        return(out);
#endif
}


int TopoFile::readTopoFile(const char* filename){
	FILE *handle;
	int in_descriptor;
	handle=fopen(filename,"r");
        if(handle==NULL){
        	printf("TopoFile:: readTopoFile Error in opening file %s\n",filename);
		exit(-1);
        }
        if ( (in_descriptor = fileno(handle)) == -1) {
                perror("TopoHeader::readTopometrixFile(): Can't get descriptor");
                return(-1);
        }
        ParseHeader(in_descriptor);
	ParseData(in_descriptor);
	fclose(handle);
	return 1;
}

int TopoFile::writeDocInfo(int handle){
	int i;
	short stemp;
	unsigned short ustemp;
	float ftemp;
	int itemp;
	double dtemp;
	int err;

	err=write(handle,header,sizeof(char)*256);
	itemp=iSwap((char*)&(itemp=sInfo.iRelease)); write(handle,(char*)&itemp,sizeof(int));
	itemp=iSwap((char*)&(itemp=sInfo.iOffset)); write(handle,(char*)&itemp,sizeof(int));
	err=write(handle,sInfo.szRelease,sizeof(sInfo.szRelease));	
	err=write(handle,sInfo.szDatetime,sizeof(sInfo.szDatetime));	
	err=write(handle,sInfo.szDescription,sizeof(sInfo.szDescription));	
	for (i = 0; i < N_MAX_GRAPHS; i++){
		ftemp=fSwap((char*)&(ftemp=sInfo.fPosX[i])); err=write(handle,(char*)&ftemp,sizeof(float));}
	for (i = 0; i < N_MAX_GRAPHS; i++){
	  ftemp=fSwap((char*)&(ftemp=sInfo.fPosX[i])); 
	  err=write(handle,(char*)&ftemp,sizeof(float));}
	stemp=sSwap((char*)&(stemp=sInfo.iCurves)); err=write(handle,(char*)&stemp,sizeof(short));
	itemp=iSwap((char*)&(itemp=sInfo.iCols)); err=write(handle,(char*)&itemp,sizeof(int));
	itemp=iSwap((char*)&(itemp=sInfo.iRows)); err=write(handle,(char*)&itemp,sizeof(int));
	stemp=sSwap((char*)&(stemp=sInfo.iDACmax)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sInfo.iDACmin)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=sInfo.fXmin)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sInfo.fXmax)); err=write(handle,(char*)&ftemp,sizeof(float));	
	ftemp=fSwap((char*)&(ftemp=sInfo.fYmin)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sInfo.fYmax)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sInfo.fDACtoWorld)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sInfo.fDACtoWorldZero)); err=write(handle,(char*)&ftemp,sizeof(float));
	ustemp=usSwap((char*)&(ustemp=sInfo.iDACtoColor)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	ustemp=usSwap((char*)&(ustemp=sInfo.iDACtoColorZero)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	stemp=sSwap((char*)&(stemp=sInfo.iWorldUnitType)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sInfo.iXYUnitType)); err=write(handle,(char*)&stemp,sizeof(short));
	err=write(handle,sInfo.szWorldUnit,sizeof(sInfo.szWorldUnit));	
	err=write(handle,sInfo.szXYUnit,sizeof(sInfo.szXYUnit));	
	err=write(handle,sInfo.szRateUnit,sizeof(sInfo.szRateUnit));	
	stemp=sSwap((char*)&(stemp=sInfo.iLayers)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sInfo.bHasEchem)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sInfo.bHasBkStrip)); err=write(handle,(char*)&stemp,sizeof(short));
	for (i = 0; i < N_MAX_GRAPHS; i++){ stemp=iSwap((char*)&(stemp=sInfo.iPts[i])); 
		err=write(handle,(char*)&stemp,sizeof(short));
	}
	stemp=iSwap((char*)&(stemp=sInfo.iXUnitType)); err=write(handle,(char*)&stemp,sizeof(short));
	err=write(handle,sInfo.szXUnit,sizeof(sInfo.szXUnit));	

	/* version 4.0 fixes */
	if(nRelease < 400){
		err=write(handle,sInfo.cFiller,510*sizeof(char));
	}
	else{
		stemp=iSwap((char*)&(stemp=sInfo.bHasAcqDisplay)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.iTilt)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.iScaleZ)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.iFilter)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.iShading)); err=write(handle,(char*)&stemp,sizeof(short));
		for (i = 0; i < 8; i++) {
			dtemp=dSwap((char*)&(dtemp=sInfo.dTiltC[i])); err=write(handle,(char*)&dtemp,sizeof(double));
		}
		stemp=iSwap((char*)&(stemp=sInfo.iDACDisplayZero)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.iDACDisplayRange)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.rRoi.left)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.rRoi.top)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.rRoi.right)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=iSwap((char*)&(stemp=sInfo.rRoi.bottom)); err=write(handle,(char*)&stemp,sizeof(short));
		err=write(handle,sInfo.cFiller,424*sizeof(char));
	}
	return 1;
}

int TopoFile::writeScanParam(int handle){
	short stemp;
	unsigned short ustemp;
	float ftemp;
	int itemp;
	int err,i;
	
	stemp=sSwap((char*)&(stemp=sScanParam.iDataType)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sScanParam.iDataDir)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sScanParam.iDataMode)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fScanZmax)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fScanZmin)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fScanXmax)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fScanYmax)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fVtip)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fI)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fVz)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fRange)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fRate)); err=write(handle,(char*)&ftemp,sizeof(float));
	stemp=sSwap((char*)&(stemp=sScanParam.iGain)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fPro)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fInteg)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fDer)); err=write(handle,(char*)&ftemp,sizeof(float));
	stemp=sSwap((char*)&(stemp=sScanParam.iGainZ)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fRotation)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fModLevel)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fAveraging)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fSpCalFactor)); err=write(handle,(char*)&ftemp,sizeof(float));
	stemp=sSwap((char*)&(stemp=sScanParam.iCalibType)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sScanParam.iLaserIntensity)); err=write(handle,(char*)&stemp,sizeof(short));
	ustemp=usSwap((char*)&(ustemp=sScanParam.iScaleFactorZ)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	ustemp=usSwap((char*)&(ustemp=sScanParam.iDACminX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	ustemp=usSwap((char*)&(ustemp=sScanParam.iDACmaxX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	ustemp=usSwap((char*)&(ustemp=sScanParam.iDACminY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	ustemp=usSwap((char*)&(ustemp=sScanParam.iDACmaxY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
	err=write(handle,sScanParam.cScanType,sizeof(sScanParam.cScanType));	
	stemp=sSwap((char*)&(stemp=sScanParam.iProbeType)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sScanParam.iStageType)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=sScanParam.iCalFileSource)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fOverscanX)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=sScanParam.fOverscanY)); err=write(handle,(char*)&ftemp,sizeof(float));

	/* version 4.0 additions */
	if(nRelease < 400){
		err=write(handle,sScanParam.cFiller,148);
		err=writeScanLayers(handle); 
	}
	else{
		stemp=sSwap((char*)&(stemp=sScanParam.iSetPointUnits)); err=write(handle,(char*)&stemp,sizeof(short));
		ftemp=fSwap((char*)&(ftemp=sScanParam.fNcRegAmp)); err=write(handle,(char*)&ftemp,sizeof(float));
		stemp=sSwap((char*)&(stemp=sScanParam.iGainXY)); err=write(handle,(char*)&stemp,sizeof(short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOffsetX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOffsetY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		for(i=0; i < 4; i++){
			ftemp=fSwap((char*)&(ftemp=sScanParam.fHysteresisX[i])); err=write(handle,(char*)&ftemp,sizeof(float));
		}
		for(i=0; i < 4; i++){
			ftemp=fSwap((char*)&(ftemp=sScanParam.fHysteresisY[i])); err=write(handle,(char*)&ftemp,sizeof(float));
		}
		stemp=sSwap((char*)&(stemp=sScanParam.iOffsetZ)); err=write(handle,(char*)&stemp,sizeof(short));
		for(i=0; i < 4; i++){
			ftemp=fSwap((char*)&(ftemp=sScanParam.fHysteresisZ[i])); err=write(handle,(char*)&ftemp,sizeof(float));
		}
		ftemp=fSwap((char*)&(ftemp=sScanParam.fCrossTalkCoef)); err=write(handle,(char*)&ftemp,sizeof(float));
		ftemp=fSwap((char*)&(ftemp=sScanParam.fSensorResponse)); err=write(handle,(char*)&ftemp,sizeof(float));
		ftemp=fSwap((char*)&(ftemp=sScanParam.fKc)); err=write(handle,(char*)&ftemp,sizeof(float));
		stemp=sSwap((char*)&(stemp=sScanParam.iCantileverType)); err=write(handle,(char*)&stemp,sizeof(short));
		err=write(handle,sScanParam.szScannerSerialNumber,16);
		stemp=sSwap((char*)&(stemp=sScanParam.iZlinearizer)); err=write(handle,(char*)&stemp,sizeof(short));
		itemp=iSwap((char*)&(itemp=sScanParam.iADC)); err=write(handle,(char*)&itemp,sizeof(int));
		stemp=sSwap((char*)&(stemp=sScanParam.bNonContact)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=sSwap((char*)&(stemp=sScanParam.CantileverType)); err=write(handle,(char*)&stemp,sizeof(short));
		ftemp=fSwap((char*)&(ftemp=sScanParam.fDriveAmplitude)); err=write(handle,(char*)&ftemp,sizeof(float));
		ftemp=fSwap((char*)&(ftemp=sScanParam.fDriveFrequency)); err=write(handle,(char*)&ftemp,sizeof(float));
		stemp=sSwap((char*)&(stemp=sScanParam.iNonContactMode)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=sSwap((char*)&(stemp=sScanParam.iNonContactPhase)); err=write(handle,(char*)&stemp,sizeof(short));
		err=write(handle,sScanParam.cFiller,34);
		err=writeScanLayers(handle); 
		err=write(handle,sScanParam.szStageType,64);
		err=write(handle,sScanParam.szStageName,64);
		err=write(handle,sScanParam.szStageText,64);
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldOffsetX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldOffsetY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldDACminX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldDACmaxX)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldDACminY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		ustemp=usSwap((char*)&(ustemp=sScanParam.iOldDACmaxY)); err=write(handle,(char*)&ustemp,sizeof(unsigned short));
		stemp=sSwap((char*)&(stemp=sScanParam.iOldGainXY)); err=write(handle,(char*)&stemp,sizeof(short));
		err=write(handle,sScanParam.cFiller1,370);
	}
	return 1;

}

int TopoFile::writeScanLayers(int handle){
	short stemp;
	float ftemp;
	int err;
        SCANPARAMSLAYER *scan3d;
	scan3d=&sScanParam.scan3d;

	ftemp=fSwap((char*)&(ftemp=scan3d->fVzStart)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzStop)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzLimit)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzArray)); err=write(handle,(char*)&ftemp,sizeof(float));
       	ftemp=fSwap((char*)&(ftemp=scan3d->fVzSpeed1)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzSpeed2)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzSpeed3)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzSpeed4)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzPullback)); err=write(handle,(char*)&ftemp,sizeof(float));
	stemp=sSwap((char*)&(stemp=(short)scan3d->iLayers)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=(short)scan3d->iHalfCycles)); err=write(handle,(char*)&stemp,sizeof(short));
       	stemp=sSwap((char*)&(stemp=(short)scan3d->iAvgPoint)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=scan3d->fDelayStart)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fDelaySample)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fDelayPullback)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fDelayEstFeedbk)); err=write(handle,(char*)&ftemp,sizeof(float));
       	stemp=sSwap((char*)&(stemp=(short)scan3d->bFeedbkPoints)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=(short)scan3d->bFeedbkCurves)); err=write(handle,(char*)&stemp,sizeof(short));
	stemp=sSwap((char*)&(stemp=(short)scan3d->bVzRelative)); err=write(handle,(char*)&stemp,sizeof(short));
	ftemp=fSwap((char*)&(ftemp=scan3d->fModFreq)); err=write(handle,(char*)&ftemp,sizeof(float));
	ftemp=fSwap((char*)&(ftemp=scan3d->fVzMod)); err=write(handle,(char*)&ftemp,sizeof(float));

	/* version 4.0 addtions */
	if(nRelease < 400){
		err=write(handle,scan3d->cFiller,184);
	}
	else{
		stemp=sSwap((char*)&(stemp=(short)scan3d->iExtLayer)); err=write(handle,(char*)&stemp,sizeof(short));
		stemp=sSwap((char*)&(stemp=(short)scan3d->bSpecialNCScan)); err=write(handle,(char*)&stemp,sizeof(short));
		err=write(handle,scan3d->cFiller,180);
	}
        return 1;
}

int TopoFile::writeHeader(int in_descriptor){

        writeDocInfo(in_descriptor);
	writeScanParam(in_descriptor);
	return 1;
}

int TopoFile::writeData(int handle){
        int     i,size;
	

	if(!valid_grid) return -1;
        size = sInfo.iCols * sInfo.iRows * sInfo.iLayers * sizeof( unsigned short);
        for (i = 0; i < size/sizeof(unsigned short); i++) griddata[i] = usFix( &griddata[i] );
 
        /* Seeking to where the data should start - this was not done in the
         * code from Topometrix. */
        if( lseek( handle, sInfo.iOffset, SEEK_SET) == -1) { /* Error */
		printf("TopoFile::ParseData File seek error\n");
		return -1;
        }

        if ( write(handle, (char*)griddata, size) != size){
		printf("TopoFile::WriteData didn't write all of the data -- error\n");
		return -1;
        }
        return(0);        
}


int TopoFile::writeTopoFile(char* filename){
	FILE *handle;
	int in_descriptor;
	handle=fopen(filename,"w");
        if(handle==NULL){
        	fprintf(stderr, "TopoFile:: writeTopoFile Error in opening file %s\n",filename);
		return(-1);
        }
        if ( (in_descriptor = fileno(handle)) == -1) {
                perror("TopoHeader::writeTopometrixFile(): Can't get descriptor");
                return(-1);
        }
	writeHeader(in_descriptor);
	writeData(in_descriptor);
	fclose(handle);
	return 1;
}

int TopoFile::writeTopoFile(FILE* handle){

	int in_descriptor;
        if(handle==NULL){
        	printf("TopoFile:: writeTopoFile NULL file pointer\n");
		return(-1);
        }
        if ( (in_descriptor = fileno(handle)) == -1) {
                perror("TopoHeader::writeTopometrixFile(): Can't get descriptor");
                return(-1);
        }
	writeHeader(in_descriptor);
	writeData(in_descriptor);
	fclose(handle);
	return 1;
}

void TopoFile::FixUpDocStruct(){

	if(nRelease > 300){

		if(nRelease < 305){
			if(sScanParam.iStageType==12){ //MODEL_TOPOCRON # used for ver < 3.05
				if(sScanParam.iProbeType==PROBE_AFM) sScanParam.iStageType=9; //MODEL_TOPOCRON_AFM
				if(sScanParam.iProbeType==PROBE_STM) sScanParam.iStageType=10; //MODEL_TOPOCRON_STM
			}
		}
		if(nRelease < 307){
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
		if(nRelease < 306){
			sScanParam.fSensorResponse=1;	//in nA/nm
			sScanParam.fKc=0.032;		//in N/m
			sInfo.rRoi.left=0;
			sInfo.rRoi.top=0;
			sInfo.rRoi.right=sInfo.iCols-1;
			sInfo.rRoi.bottom=sInfo.iRows-1;
		}

		if(nRelease < 307){
			if(sInfo.rRoi.bottom==0 || sInfo.rRoi.right==0){
				sInfo.rRoi.left=0;
				sInfo.rRoi.top=0;
				sInfo.rRoi.right=sInfo.iCols-1;
				sInfo.rRoi.bottom=sInfo.iRows-1;
			}
		}
	}
	else{
		perror("Unable to process Topo file earlier than 3.01");
		exit(-1);
	}

	return;
}

int TopoFile::TopoDataToGrid(BCGrid* G, const char* filename){
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

        switch (sInfo.iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %d (%s)\n",
                        sInfo.iXYUnitType, sInfo.szXYUnit);
                return(-1);
        }

        switch (sInfo.iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %d (%s)\n",
                        sInfo.iWorldUnitType, sInfo.szWorldUnit);
                return(-1);
        }

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        G->_num_x = sInfo.iCols;
        G->_num_y = sInfo.iRows;
        G->_min_x = sInfo.fXmin * XYunits2nm;
        G->_max_x = sInfo.fXmax * XYunits2nm;
        G->_min_y = sInfo.fYmin * XYunits2nm;
        G->_max_y = sInfo.fYmax * XYunits2nm;

        printf("  min_x %f max_x %f\n",G->_min_x,G->_max_x);
        printf("  min_y %f max_y %f\n",G->_min_y,G->_max_y);
        printf("  Units in X/Y: %s (type %d)\n",sInfo.szXYUnit,
                sInfo.iXYUnitType);
        printf("  Units in Z: %s (type %d)\n",sInfo.szWorldUnit,
                sInfo.iWorldUnitType);

        /*
         * Make it possible to read multiple planes.  They won't
         * always be height, but I think they will all be the same
         * type.
         */

        for (layer = 0; layer < sInfo.iLayers; layer++) {

                /*
                 * Allocate a plane to hold the height information
                 */

                switch (data_type) {
                  case HEIGHT:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "nm", NOT_TIMED);
                        break;
                  case AUX:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "V", NOT_TIMED);
                        break;
                  case DEFLECTION:
                        G->findUniquePlaneName(filename, &name);
                        plane = G->addNewPlane(name, "nA", NOT_TIMED);
                        break;
                  case FORCE_MODULATION_DATA:
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
                Zscale = sInfo.fDACtoWorld;
                Zoffset = sInfo.fDACtoWorldZero;
        	printf("  Zscale=%f Zoffset=%f\n",Zscale,Zoffset);
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
        printf("  min_z %f max_z %f\n",min_z,max_z);
        return(0);
}

int TopoFile::GridToTopoData(BCGrid* G, BCPlane *P){
        int     row, col;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        int     data_type;
        BCPlane* plane;         // A new plane of data to be filled
        BCString name;          // Name assigned to the new plane

	if(valid_header == 0) return -1;
	if(griddata != NULL) delete griddata;

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

	sInfo.iLayers=G->numPlanes();
        switch (sInfo.iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %d (%s)\n",
                        sInfo.iXYUnitType, sInfo.szXYUnit);
                return(-1);
        }

        switch (sInfo.iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %d (%s)\n",
                        sInfo.iWorldUnitType, sInfo.szWorldUnit);
                return(-1);
        }

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        sInfo.iCols=G->_num_x;
        sInfo.iRows=G->_num_y;
        sInfo.fXmin=G->_min_x / XYunits2nm;
        sInfo.fXmax=G->_max_x/ XYunits2nm;
        sInfo.fYmin=G->_min_y / XYunits2nm;
        sInfo.fYmax=G->_max_y / XYunits2nm;
       	sInfo.iLayers=1;
	plane=P;

        printf("  Units in X/Y: %s (type %d)\n",sInfo.szXYUnit,
                sInfo.iXYUnitType);
        printf("  Units in Z: %s (type %d)\n",sInfo.szWorldUnit,
                sInfo.iWorldUnitType);
	
                Zscale = sInfo.fDACtoWorld;
                Zoffset = sInfo.fDACtoWorldZero;

		griddata=new unsigned short[sInfo.iCols*sInfo.iRows];
		if(griddata == NULL){ perror("Unable to allocate griddata\n"); return -1;}

		// Data is stored in the Topo file in column-major order
                // starting from the lower left corner.
                // the factor col*_num_y + row gets you to the right
                // element
                // The Y elements are stored backwards with respect to
                // the Plane structure, so Y needs to be inverted.
	
       	         for (col = 0; col < sInfo.iRows; col++) {
                  for (row = 0; row < sInfo.iCols; row++) {
                        int y = (sInfo.iRows - 1) - col;
                        griddata[(col)*sInfo.iRows+ row] = \
				(unsigned short)(
					(
						((float)plane->value(row,y)/(float)Zunits2nm)\
						-(float)Zoffset\
					)/(float)Zscale
				);
                  }
                 }
	valid_grid = 1;
	return(0);
}


int TopoFile::GridToTopoData(BCGrid* G){
        int     row, col, layer;
        double  Zscale, Zoffset;
        double  Zunits2nm;
        double  XYunits2nm;
        int     data_type;
        BCPlane* plane;         // A new plane of data to be filled
        BCString name;          // Name assigned to the new plane

	if(valid_header == 0) return -1;
	if(griddata != NULL) delete griddata;

        /*
         * Find out the scale factors that take the X/Y and Z units to
         * nanometers, which is the value that uncb files store.
         */

	sInfo.iLayers=G->numPlanes();
        switch (sInfo.iXYUnitType) {

            case 2: /* Nanometers */
                XYunits2nm = 1.0;
                break;

            case 3: /* Micrometers */
                XYunits2nm = 1000.0;
                break;

            default:
                fprintf(stderr,"Unrecognized XY units, type %d (%s)\n",
                        sInfo.iXYUnitType, sInfo.szXYUnit);
                return(-1);
        }

        switch (sInfo.iWorldUnitType) {

            case 1: /* Nanometers */
                Zunits2nm = 1.0;
                data_type = HEIGHT;
                break;

            case 6: /* Measuring Volts, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = AUX;
                break;

            case 7: /* Measuring NanoAmps, not Nanometers at all */
                Zunits2nm = 1.0;
                data_type = DEFLECTION;
                break;

            case 8: // Measuring force modulation data
                Zunits2nm = 1.0;
                data_type = FORCE_MODULATION_DATA;
                break;

            default:
                fprintf(stderr,"Unrecognized Z units, type %d (%s)\n",
                        sInfo.iWorldUnitType, sInfo.szWorldUnit);
                return(-1);
        }

        /*
         * Fill in the parameters we can before seeing the data
         */

        /*XXX Find out the proper orientation (depends on iDataDir?) */
        sInfo.iCols=G->_num_x;
        sInfo.iRows=G->_num_y;
        sInfo.fXmin=G->_min_x / XYunits2nm;
        sInfo.fXmax=G->_max_x/ XYunits2nm;
        sInfo.fYmin=G->_min_y / XYunits2nm;
        sInfo.fYmax=G->_max_y / XYunits2nm;
       	sInfo.iLayers=G->numPlanes();
	plane=G->head();

        printf("  Units in X/Y: %s (type %d)\n",sInfo.szXYUnit,
                sInfo.iXYUnitType);
        printf("  Units in Z: %s (type %d)\n",sInfo.szWorldUnit,
                sInfo.iWorldUnitType);
	
                Zscale = sInfo.fDACtoWorld;
                Zoffset = sInfo.fDACtoWorldZero;


		griddata=new unsigned short[sInfo.iLayers*sInfo.iCols*sInfo.iRows];
		if(griddata == NULL){ perror("Unable to allocate griddata\n"); return -1;}

		// Data is stored in the Topo file in column-major order
                // starting from the lower left corner.
                // the factor layer*_num_x*_num_y gets you to the start
                // of a layer.
                // the factor col*_num_y + row gets you to the right
                // element in that layer.
                // The Y elements are stored backwards with respect to
                // the Plane structure, so Y needs to be inverted.
	
		for(layer =0; layer < sInfo.iLayers; layer++){
       	         for (col = 0; col < sInfo.iRows; col++) {
                  for (row = 0; row < sInfo.iCols; row++) {
                        int y = (sInfo.iRows - 1) - col;
                        griddata[(layer*sInfo.iCols+ col)*sInfo.iRows+ row] = \
				(unsigned short)(
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

int BCGrid::readTopometrixFile(TopoFile &TGF, const char *filename){

	TGF.readTopoFile(filename);
	TGF.TopoDataToGrid(this,filename);	
	return 1;	
	
}

