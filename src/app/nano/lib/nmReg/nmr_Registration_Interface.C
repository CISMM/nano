#include "nmr_Registration_Interface.h"
#include "nmb_Transform_TScShR.h"

nmr_Registration_Interface::nmr_Registration_Interface (const char * /*name*/,
                                                 vrpn_Connection * c)
{
  if (c) {
    d_SetImageParameters_type = c->register_message_type
                       ("nmr_Registration SetImageParameters");
    d_SetImageScanlineData_type = c->register_message_type
                       ("nmr_Registration SetImageScanlineData");
    d_SetTransformationOptions_type = c->register_message_type
                       ("nmr_Registration SetTransformationOptions");
    d_SetTransformationParameters_type = c->register_message_type
                       ("nmr_Registration SetTransformationParameters");
    d_SetResolutions_type = c->register_message_type
                       ("nmr_Registration SetResolutions");
    d_SetIterationLimit_type= c->register_message_type
                       ("nmr_Registration SetIterationLimit");
    d_SetStepSize_type= c->register_message_type
                       ("nmr_Registration SetStepSize");
    d_SetCurrentResolution_type= c->register_message_type
                       ("nmr_Registration SetCurrentResolution");

    d_AutoAlign_type = c->register_message_type
                       ("nmr_Registration AutoAlign");
    d_EnableGUI_type = c->register_message_type
                       ("nmr_Registration EnableGUI");
    d_EnableAutoUpdate_type = c->register_message_type
                       ("nmr_Registration EnableAutoUpdate");
    d_Fiducial_type = c->register_message_type
                       ("nmr_Registration Fiducial");

    d_ImageParameters_type = c->register_message_type
                       ("nmr_Registration ImageParameters");
    d_TransformationOptions_type = c->register_message_type
                       ("nmr_Registration TransformationOptions");
    d_RegistrationResult_type = c->register_message_type
                       ("nmr_Registration RegistrationResult");
  }
}

nmr_Registration_Interface::~nmr_Registration_Interface()
{
}

//static
char * nmr_Registration_Interface::encode_SetImageParameters
          (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 res_x, vrpn_int32 res_y,
           vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld,
           vrpn_bool flipX, vrpn_bool flipY)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32) + 
         2 * sizeof(vrpn_bool);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetImageParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which_image);
    vrpn_buffer(&mptr, &mlen, res_x);
    vrpn_buffer(&mptr, &mlen, res_y);
    vrpn_buffer(&mptr, &mlen, xSizeWorld);
    vrpn_buffer(&mptr, &mlen, ySizeWorld);
    vrpn_buffer(&mptr, &mlen, flipX);
    vrpn_buffer(&mptr, &mlen, flipY);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_SetImageParameters
          (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *res_x, vrpn_int32 *res_y,
           vrpn_float32 *xSizeWorld, vrpn_float32 *ySizeWorld,
           vrpn_bool *flipX, vrpn_bool *flipY)
{
  if (vrpn_unbuffer(buf, which_image) ||
      vrpn_unbuffer(buf, res_x) ||
      vrpn_unbuffer(buf, res_y) ||
      vrpn_unbuffer(buf, xSizeWorld) ||
      vrpn_unbuffer(buf, ySizeWorld) ||
      vrpn_unbuffer(buf, flipX) ||
      vrpn_unbuffer(buf, flipY)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_SetImageScanlineData
          (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 row, vrpn_int32 line_length,
           vrpn_float32 *data)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32) + line_length * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetImageScanlineData: "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which_image);
    vrpn_buffer(&mptr, &mlen, row);
    vrpn_buffer(&mptr, &mlen, line_length);
    int i;
    for (i = 0; i < line_length; i++){
        vrpn_buffer(&mptr, &mlen, data[i]);
    }
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_SetImageScanlineData
          (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *row, vrpn_int32 *line_length,
           vrpn_float32 *data, vrpn_int32 max_expected_line_length)
{
  if (vrpn_unbuffer(buf, which_image) ||
      vrpn_unbuffer(buf, row) ||
      vrpn_unbuffer(buf, line_length)) {
    return -1;
  }
  int i;
  if (*line_length > max_expected_line_length) {
      fprintf(stderr, "nmr_Registration_Interface::decode_SetImageScanlineData"
          ": Error, expected length <= %d, actual length: %d\n",
          max_expected_line_length, line_length);
      // unbuffer the remainder of our message anyway
      // (i'm not sure if this is necessary but it shouldn't hurt)
      vrpn_float32 temp;
      for (i = 0; i < *line_length; i++){
          if (vrpn_unbuffer(buf, &temp)){
              return -1;
          }
      }
      return -1;
  }
  for (i = 0; i < *line_length; i++) {
      if (vrpn_unbuffer(buf, &(data[i]))) {
          return -1;
      }
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_SetTransformationOptions (
           vrpn_int32 *len,
           vrpn_int32 transformType)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
           "nmr_Registration_Interface::encode_SetTransformationOptions:  "
           "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, transformType);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_SetTransformationOptions (
           const char **buf,
           vrpn_int32 *transformType)
{
  if (vrpn_unbuffer(buf, transformType)) {
    return -1;
  }
  return 0;
}

//static 
char * nmr_Registration_Interface::encode_SetTransformationParameters (
           vrpn_int32 *len,
           vrpn_float32 *transformParameters)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len =  nmb_numTransformParameters * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
           "nmr_Registration_Interface::encode_SetTransformationParameters:  "
           "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    int i;
    for (i = 0; i < nmb_numTransformParameters; i++) {
      vrpn_buffer(&mptr, &mlen, transformParameters[i]);
    }
  }

  return msgbuf;
}

//static 
vrpn_int32 nmr_Registration_Interface::decode_SetTransformationParameters (
           const char **buf,
           vrpn_float32 *transformParameters)
{
  int i;
  for (i = 0; i < nmb_numTransformParameters; i++) {
    if (vrpn_unbuffer(buf, &(transformParameters[i]))) {
      return -1;
    }
  }
  return 0;
}

//static 
char * nmr_Registration_Interface::encode_SetResolutions (vrpn_int32 *len,
           vrpn_int32 numLevels, vrpn_float32 *std_dev)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32) + numLevels * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetResolutions: "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numLevels);
    int i;
    for (i = 0; i < numLevels; i++){
        vrpn_buffer(&mptr, &mlen, std_dev[i]);
    }
  }

  return msgbuf;
}

//static 
vrpn_int32 nmr_Registration_Interface::decode_SetResolutions (const char **buf,
           vrpn_int32 *numLevels, vrpn_float32 *std_dev)
{
  if (vrpn_unbuffer(buf, numLevels)) {
    return -1;
  }
  int i;
  for (i = 0; i < *numLevels; i++) {
      if (vrpn_unbuffer(buf, &(std_dev[i]))) {
          return -1;
      }
  }
  return 0;

}
//static 
char * nmr_Registration_Interface::encode_SetIterationLimit (vrpn_int32 *len,
           vrpn_int32 maxIterations)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetIterationLimit:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, maxIterations);
  }

  return msgbuf;
}

//static 
vrpn_int32 nmr_Registration_Interface::decode_SetIterationLimit (
           const char **buf,
           vrpn_int32 *maxIterations)
{
  if (vrpn_unbuffer(buf, maxIterations)) {
    return -1;
  }
  return 0;
}

//static 
char * nmr_Registration_Interface::encode_SetStepSize (vrpn_int32 *len,
           vrpn_float32 stepSize)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetStepSize:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}

//static 
vrpn_int32 nmr_Registration_Interface::decode_SetStepSize (const char **buf,
           vrpn_float32 *stepSize)
{
  if (vrpn_unbuffer(buf, stepSize)) {
    return -1;
  }
  return 0;
}

//static 
char * nmr_Registration_Interface::encode_SetCurrentResolution (vrpn_int32 *len,
           vrpn_int32 resolutionLevel)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_SetCurrentResolution:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, resolutionLevel);
  }

  return msgbuf;
}

//static 
vrpn_int32 nmr_Registration_Interface::decode_SetCurrentResolution (
           const char **buf,
           vrpn_int32 *resolutionLevel)
{
  if (vrpn_unbuffer(buf, resolutionLevel)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_AutoAlign (vrpn_int32 *len,
           vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_AutoAlign:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_AutoAlign
          (const char **buf,
           vrpn_int32 *enable)
{
  if (vrpn_unbuffer(buf, enable)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_EnableGUI (vrpn_int32 *len,
           vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_EnableGUI:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_EnableGUI
          (const char **buf,
           vrpn_int32 *enable)
{
  if (vrpn_unbuffer(buf, enable)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_EnableAutoUpdate (vrpn_int32 *len,
           vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_EnableAutoUpdate:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_EnableAutoUpdate
          (const char **buf,
           vrpn_int32 *enable)
{
  if (vrpn_unbuffer(buf, enable)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_Fiducial (vrpn_int32 *len,
           vrpn_int32 replace, vrpn_int32 num,
           vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
           vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  int numPacked = num;
  if (numPacked > NMR_MAX_FIDUCIAL) {
    fprintf(stderr, "encode_Fiducial: Warning, truncating list of fiducials"
                    " to the first %d points\n", NMR_MAX_FIDUCIAL);
    numPacked = NMR_MAX_FIDUCIAL;
  }

  *len = numPacked * 6 * sizeof(vrpn_float32) + 2*sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_Fiducial:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, replace);
    vrpn_buffer(&mptr, &mlen, numPacked);
    int i;
    for (i = 0; i < numPacked; i++) {
      vrpn_buffer(&mptr, &mlen, x_src[i]);
      vrpn_buffer(&mptr, &mlen, y_src[i]);
      vrpn_buffer(&mptr, &mlen, z_src[i]);
      vrpn_buffer(&mptr, &mlen, x_tgt[i]);
      vrpn_buffer(&mptr, &mlen, y_tgt[i]);
      vrpn_buffer(&mptr, &mlen, z_tgt[i]);
    }
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_Fiducial (const char **buf,
           vrpn_int32 *replace, vrpn_int32 *num,
           vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
           vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt)
{
  if (vrpn_unbuffer(buf, replace) ||
      vrpn_unbuffer(buf, num)) {
    return -1;
  }
  int i;
  for (i = 0; i < (*num); i++) {
    if (vrpn_unbuffer(buf, &(x_src[i])) ||
        vrpn_unbuffer(buf, &(y_src[i])) ||
        vrpn_unbuffer(buf, &(z_src[i])) ||
        vrpn_unbuffer(buf, &(x_tgt[i])) ||
        vrpn_unbuffer(buf, &(y_tgt[i])) ||
        vrpn_unbuffer(buf, &(z_tgt[i]))) {
      return -1;
    }
  }
  return 0;
}

// server-->client
//static
char * nmr_Registration_Interface::encode_ImageParameters (vrpn_int32 *len,
           vrpn_int32 which_image,
           vrpn_int32 res_x, vrpn_int32 res_y,
           vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld,
           vrpn_bool flipX, vrpn_bool flipY)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32) +
         2 * sizeof(vrpn_bool);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_ImageParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which_image);
    vrpn_buffer(&mptr, &mlen, res_x);
    vrpn_buffer(&mptr, &mlen, res_y);
    vrpn_buffer(&mptr, &mlen, xSizeWorld);
    vrpn_buffer(&mptr, &mlen, ySizeWorld);
    vrpn_buffer(&mptr, &mlen, flipX);
    vrpn_buffer(&mptr, &mlen, flipY);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_ImageParameters (const char **buf,
           vrpn_int32 *which_image,
           vrpn_int32 *res_x, vrpn_int32 *res_y,
           vrpn_float32 *xSizeWorld, vrpn_float32 *ySizeWorld,
           vrpn_bool *flipX, vrpn_bool *flipY)
{
  if (vrpn_unbuffer(buf, which_image) ||
      vrpn_unbuffer(buf, res_x) ||
      vrpn_unbuffer(buf, res_y) ||
      vrpn_unbuffer(buf, xSizeWorld) ||
      vrpn_unbuffer(buf, ySizeWorld) ||
      vrpn_unbuffer(buf, flipX) ||
      vrpn_unbuffer(buf, flipY)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_TransformationOptions (
           vrpn_int32 *len,
           vrpn_int32 transformType)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_TransformationOptions: "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, transformType);
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_TransformationOptions (
           const char **buf,
           vrpn_int32 *transformType)
{
  if (vrpn_unbuffer(buf, transformType)) {
    return -1;
  }
  return 0;
}

//static
char * nmr_Registration_Interface::encode_RegistrationResult (vrpn_int32 *len,
                                    vrpn_int32 which, vrpn_float64 *matrix44)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1*sizeof(vrpn_int32) + 16 * sizeof(vrpn_float64);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmr_Registration_Interface::encode_RegistrationResult:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    for (int i = 0; i < 16; i++){
        vrpn_buffer(&mptr, &mlen, matrix44[i]);
    }
  }

  return msgbuf;
}

//static
vrpn_int32 nmr_Registration_Interface::decode_RegistrationResult (
                                    const char **buf,
                                    vrpn_int32 *which,
                                    vrpn_float64 *matrix44)
{
  int i;
  if (vrpn_unbuffer(buf, which)) {
    return -1;
  }
  for (i = 0; i < 16; i++) {
      if (vrpn_unbuffer(buf, &(matrix44[i]))) {
          return -1;
      }
  }
  return 0;
}
