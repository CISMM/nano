//============================================================================
// ppm.hpp : Portable Pixel Map image format module
//============================================================================

#ifndef PPM_H_GUARD
#define PPM_H_GUARD

void LoadPPM(char *FileName, unsigned char* &Color, int &Width, int &Height);
void WritePPM(char *FileName, unsigned char* Color, int Width, int Height);

#endif // PPM_H_GUARD
