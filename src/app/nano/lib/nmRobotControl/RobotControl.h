#include "imageViewer.h"

#include "nmm_MicroscopeRemote.h"
#include "nmb_Dataset.h"


class RobotControl
{
   private:
      static int eventHandler(const ImageViewerWindowEvent &event, void *ud);

      static int displayHandler(const ImageViewerDisplayData &data, void *ud);

   public:
      RobotControl(nmm_Microscope_Remote *microscope, nmb_Dataset *dataset);

      void mainloop(void);

      void show(void);

      void hide(void);
};
