// draw_core_list.h
// Mark Foskey

#ifndef DRAW_CORE_LIST_HWRAP_UNC_1998
#define DRAW_CORE_LIST_HWRAP_UNC_1998

#define D_LongImage
#define D_CoreList
#define D_cores

#include <imprelud.h>

void draw_core_list(const CoreList& core_list, 
		    const LongImage& im, 
		    double tick_spacing,
		    const IntensRange& range);

void draw_core(const Core2* core, double tick_spacing);

#endif // DRAW_CORE_LIST_HWRAP_UNC_1998
