The purpose of this library is to provide the ability to display 
greyscale images and allow the user to interact with them in various
ways.

This is in its own library because it doesn't belong in the following which
one might consider putting it in:

nmg_Graphics since I want to use this code on platforms for which vogl
hasn't been compiled and this code doesn't depend on vogl anyway

nmUI because it doesn't use tcl/tk and we don't normally compile all the
tcl/tk and associated libraries for all the platforms on which this 
code is used

nmReg because its not only used for the registration feature 
(at one point it was but that has changed - now it is also used for the SEM and
for robot control)

