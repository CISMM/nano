#ifndef NMG_GLOBALS_H
#define NMG_GLOBALS_H

// nmg_Globals

// File for anything that needs to be visible *outside this module*
// to use the module.


class nmg_Graphics;

extern nmg_Graphics * graphics;

enum RegMode { REG_NULL = 0, REG_TRANSLATE = 1, REG_SIZE = 2, 
               REG_SIZE_WIDTH = 4, REG_SIZE_HEIGHT = 8 } ; 

#endif  // NMG_GLOBALS_H

