USES_LIBS := nmMP ImgFormat vrpn sdi quat
EXCLUDE_SRC := readNanoscopeFile.C tnv.t.C
ifndef VC
# Only include vc_dirent.c if we are compiling on VC++
EXCLUDE_SRC += vc_dirent.c
endif

include ../Makefile.share
