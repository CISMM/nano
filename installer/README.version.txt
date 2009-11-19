This program requires the NSRG runtime to be installed.

Versions:
---------
12.5:  (November 16, 2009)
	- includes the ability to read igor .ibw files from asylum.
	- cory quammen's registration additions from summer '08.
	  these never made it into a released version before.
12.4:  (March 11, 2008)
	- resetting the direct-step axis will now return it to the
	  lower-left corner of the current heightplane.
12.3:  (May 19, 2007)
	- added "-microscope_type" flag to provide native support for
	  Asylum microscope.
12.2:  (June 22, 2006)
	- added a "keep stepping" option to the direct-step tool
	  that will cause it to keep stepping as if a button
	  were repeatedly clicked.
12.1:  (May 21, 2006)
	- disables logging of SEM streams.  For the combined
	  AFM-optical systems, this is done on the video 
	  server.
12.0b:  (April 28, 2006)
	- an interrim version that should work with the Omni 
	  phantoms and the newest libraries from Sensable.
11.6:  (August 13, 2005)
	- fixes the phantom-button-as-toggle functionality
	- adds a command-line option -noautorescan to have auto-
	  rescan be off at program start-up.
11.5:  (August 7, 2005)
	- fixes a bug with the direst-step tools when stepping in Z
	  without using the adjustable axes.
	- will not have topo start scanning on connect if "auto-
	  rescan" is deselected.
11.4:  (October 1, 2004)
	- Makes the "direction of projected texture" icon invisible by
	  default, so that we don't see it when loading tiny regions.
	- Puts things where they need to be to make the ImageMagick
	  file reads and writes work.
11.3H: (July 7, 2004)
	- Released for folks in Hamburg, including updates by Russ since May
	  to enable the use of Imager plug-ins
11.3:  (May 10, 2004) 
	- fixed a bug that caused nano to crash when doing force curves.
	  also, nano should no longer print lots of point-result error
	  messages.
	- mathematical morphology tool for dilating and eroding a surface
	  with an image of a tip.
	- added ability to read files of the STM-format used at the 
	  University of Hamburg Dept. of Physics.


11.2:  (Jan. 19, 2004) This begins the beta-only releases of nano.  The
	last fully tested release was Aug. 2002; this release incorporates
	all changes since then.

  Some of the many changes:
	- exported mod files no long clipped to 10,000 points
	- ability to use a joystick instead of a Phantom
	- ability to read WSxM files
	- ability to capture video of nano working
	- more reliable stereo display
	- overhead-view button
	- ability to display the haptic grid
	- ability to import and display objects (.obj files)
	- lots of changes and improvements to the SEM tools, including
	  general texture projection and alignment of images with 
	  scans and models.

