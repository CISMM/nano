#ifndef NMG_GLOBALS_H
#define NMG_GLOBALS_H

// nmg_Globals

// File for anything that needs to be visible *outside this module*
// to use the module.


class nmg_Graphics;

extern nmg_Graphics * graphics;

enum RegMode { REG_NULL = 0, 
               REG_TRANSLATE = 1, REG_PREP_TRANSLATE = 2, 
               REG_SIZE = 4, REG_PREP_SIZE = 8, 
               REG_SIZE_WIDTH = 16, REG_PREP_SIZE_WIDTH = 32, 
               REG_SIZE_HEIGHT = 64, REG_PREP_SIZE_HEIGHT = 128,
               REG_DEL = 256} ; 

#endif  // NMG_GLOBALS_H

