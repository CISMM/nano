//============================================================================
// ppm.cpp : Portable Pixel Map image format module
//============================================================================

#include <stdio.h>

//----------------------------------------------------------------------------
// READS AN IMAGE IN FROM A PPM FILE. RETURNS THE COLOR RGB ARRAY AND DIMENSIONS
// PERFORMS AUTO-ALLOCATION OF Color ARRAY IF SET TO NULL BEFORE CALLING; OTHERWISE
// ASSUMES THAT COLOR HAS BEEN PRE-ALLOCED.
//----------------------------------------------------------------------------
void LoadPPM(char *FileName, unsigned char* &Color, int &Width, int &Height)
{
  FILE* fp = fopen(FileName, "rb");
  if (fp==NULL) 
    { printf("ERROR: unable to open %s!\n",FileName);
      Color=NULL; Width=0; Height=0; return; }
  int c,s;
  do{ do { s=fgetc(fp); } while (s!='\n'); } while ((c=fgetc(fp))=='#');
  ungetc(c,fp);
  fscanf(fp, "%d %d\n255\n", &Width, &Height);
  printf("Reading %dx%d Texture [%s]. . .\n", Width, Height, FileName);
  long NumComponents = (long)Width*Height*3;
  if (Color==NULL) Color = new unsigned char[NumComponents];

  // this effectively reads the image in upside down.
  for(int i=0; i < Height; i++)
  {
	fread(Color + (long)i*(Width)*3,Width*3,1,fp);
  }

  fclose(fp);
}

//----------------------------------------------------------------------------
// Writes an unsigned byte RGB color array out to a PPM file.
//----------------------------------------------------------------------------
void WritePPM(char *FileName, unsigned char* Color, int Width, int Height)
{
  FILE* fp = fopen(FileName, "wb");
  if (fp==NULL) { printf("Error (WritePPM) : unable to open %s!\n",FileName); return; }
  fprintf(fp, "P6\n%d %d\n255\n", Width, Height);
  fwrite(Color,1,(long)Width*Height*3,fp);
  fclose(fp);
}
