Doxygen is a tool for automatically creating documentation for a
source tree. This directory includes the files nano-doc.dox - a
configuration file for doxygen. 

Doxygen will probably be installed in /usr/contrib/mod/bin before too
long, but until it is, it lives in 
/afs/unc/proj/stm/src/doxygen-1.0.0-pc   and
/afs/unc/proj/stm/src/doxygen-1.0.0-sgi

To make documentation in your tree, cd to this directory
(nano/autodoc), then (on a PC):
   /afs/unc/proj/stm/src/doxygen-1.0.0-pc/bin/doxygen nano-doc.dox

It will create an html directory with all the docs extracted from your
tree. 

Check the /afs/unc/proj/stm/src/doxygen-1.0.0-pc/html directory for
doxygen's manual. 

3/11/00   Aron Helser

