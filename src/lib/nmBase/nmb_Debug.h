#ifndef NMB_DEBUG_H
#define NMB_DEBUG_H

extern int spm_verbosity;

#define VERBOSE(level,msg) \
    if (spm_verbosity >= level) fprintf(stderr,"%s\n",msg)


#endif  // NMB_DEBUG_H

