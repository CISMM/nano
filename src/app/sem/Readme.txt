This directory contains code that deals directly with the EDAX system for
controlling the SEM. 

=== some notes:
vertical retrace has a small effect but its hard to tell exactly what its doing 
  seems to add a positive offset to the y-scan as it is increased
horizontal retrace does change the x-scan: but doesn't change much below 10 usec
probably because 10usec is pretty short relative to the length of the scan

interpixel delay can be set to 0 but doesn't differ from when it is set to
1000 nsec (1usec), larger values do have the expected effect
pixel integration also has the expected effect (min is 100nsec)

point scanning seems to have some kind of overhead that forces dwell time to
be at least 1 msec - a dwell time of 0 gives about 1 msec, a dwell time of
.5 msec gives about 1.5 msec 

in between points, scan seems to switch to center of scan region for 100 usec

==================
Here are the responses I've gotten to my questions to
EDAX about their SDK.

=======================================================================

text of an email from Mike Lupu (software guy at EDAX) with regard to
delays in scanning:

The hardware speed limitation comes from:

 (1) Moving the beam and allowing time at each location for the beam to
settle - dependent on microscope performance.
        Range: ~2-4 microsecs for the locations on the same scan line, ~500
microsecs for a new line.

 (2) Reading/digitizing one signal is ~100 nanoseconds. The integration
factor selectable in the user interface defines how many ADC reads per pixel
are used in averaging. Selecting 0 has a special meaning: data are buffered
and transferred as one byte (the most significant) per pixel, instead of
two.

 (3) Data transfer to the host.


*****************************
email from Dean SanFilippo (DSanFilippo@edax.com) with regard to getting
16-bit data

Adam,

Currently the SDK only returns 8 bit data.  The data that is returned to you
is the upper 8 bits of the 16 bit data.  I will provide a call for you after
the 3.3 release that is about to be finalized.  For now you can shift the
data up by 8.

The 8 bit collection Mike was talking about is controlled by
SgSetScanParams(LONG ScanType, LONG TransferType);   There are two scan
modes available.  Normal scan and an optimized high speed scan that only
uses ADC1 and only does one sample of that ADC.  The optimized scan mode is
typically used for applications that want to collect very large images as
fast as possible.   There are three data transfer modes:  Disabled, Byte and
Word.   Disabled is used for beam movement without collection.  The Byte
mode is used to optimize data transfer on the PCI bus.  Since only the upper
8 bits of data are transfered we can pack two pixels into each transfer to
double the bus efficiency.

The beam settling time is controlled by SpDwel(ipd, 0, dwell).  The first
parameter is the interpixel delay.  You can set this value between 1 and 16K
microseconds.  The third parameter is the beam dwell time for the background
scan.

Dean

*********************
another email about eliminating the settling time:

Adam,

You can use a beam settling time of 0 with an EDI-1 based system.  With and
EDI-2 the smallest beam settling time is 100ns.  

The optimized scan mode has less overhead because you are restricted to one
ADC with only 1 read from that ADC. Your quickest image collection would be
via:
        // Set IPD to zero

        SpDwel(0,0,1);

        // Fast / Byte

        SgSetScanParams(1, 1); 

You can write a test program to time the following combinations of scan mode
and data size:

        SgSetScanParams(0, 1)   // Normal / Byte
        SgSetScanParams(0, 2)   // Normal / Word
        SgSetScanParams(1, 1)   // Fast / Byte
        SgSetScanParams(1, 2)   // Fast /Word

You can control the Horizontal and Vertical retrace delays via
SgSetRetrace(LONG HDelay, LONG VDelay).  The delay on and EDI-1 is from 1 to
1600 us.

We have seen several things cause the distortion at the beginning of the
line.
The retrace delay being to short could be one of the issues.  The other
issues are related to column hysterisis, possible scan coil non-linearity
over a certain voltage range, the physical properties of accelerating the
beam from rest to the scan speed across the line, and the distance the beam
is moving from point to point.  You will probably find that increasing your
beam settling time will remove the distortion.  Also smaller image matrix
sizes require larger beam settling time than larger image matrix sizes.  

Dean



