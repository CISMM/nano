#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>
#include <vrpn_Shared.h>  // for vrpn_unbuffer()
#include <vrpn_Ohmmeter.h>  // for ohmmeter status types

#include <stdlib.h>
#ifdef sgi
#include <unistd.h>
#endif
#include <stdio.h>

vrpn_int32 first_time_sec = 0L;


// SCAN AND POINT

DECLARE STRUCTURES ReportScanDatasets
DECLARE STRUCTURES ReportPointDatasets
DECLARE STRUCTURES ForceCurveData

HANDLE InContactMode
  printf("InContactMode (%g, %g, %g, %g)\n", P, I, D, setpoint);

  return 0;
}

HANDLE InOscillatingMode
  printf ("InOscillatingMode (%g, %g, %g, %g, %g)\n", P, I, D,
          setpoint, amplitude);

  return 0;
}


HANDLE ReportScanrateNM
  printf ("ReportScanrateNM (%g)\n", rate);

  return 0;
}


// SCAN


HANDLE HelloMessage
  printf("HelloMessage (%s, %s, %d, %d,)\n", nm, scopeName,
         majorVersion, minorVersion);

  return 0;
}

HANDLE ReportPID
  printf ("ReportPID (%g, %g, %g)\n", P, I, D);

  return 0;
}
HANDLE ReportGridSize
  printf ("ReportGridSize (%d, %d)\n", x, y);

  return 0;
}


HANDLE ReportScanRegionNM
  printf("ReportScanRegionNM (%g, %g, %g, %g, %g)\n",
         xmin, ymin, xmax, ymax, angle);
  return 0;
}

HANDLE ReportMaxScanRangeNM
  printf("ReportMaxScanRangeNM (%g, %g, %g, %g, %g, %g)\n",
         xmin, ymin, zmin, xmax, ymax, zmax);
  return 0;
}


HANDLE SlowScanPause
  printf("SlowScanPause ()\n");
  return 0;
}


HANDLE SlowScanResume
  printf("SlowScanResume ()\n");
  return 0;
}


HANDLE ReportScanWindow
  printf("ReportScanWindow (%d, %d, %d, %d)\n",
         grid_xmin, grid_ymin, grid_xmax, grid_ymax);
  return 0;
}


HANDLE ReportScanDatasets
  int i;

  printf("ReportScanDatasets (%d)\n", count);
  
  for (i = 0; i < count; i++) {
    printf("  (%s, %s, %g, %g)\n", datasets[i].name,
           datasets[i].units, datasets[i].offset,
           datasets[i].scale);
  }
  return 0;
}

HANDLE WindowLineData
  int i, j;

  printf ("WindowLineData (%d, %d, %d, %d, %d, %d, %d:%d)\n",
          x, y, dx, dy, reports, fields, sec-first_time_sec, usec);

  for (i = 0; i < reports; i++) {
    printf("  (");
    for (j = 0; j < fields; j++) {
      printf("%g ", data[reports][fields]);
    }
    printf(")");
  }
  printf("\n");

  return 0;
}


// POINT


HANDLE ReportPointDatasets
  int i;
  printf("ReportPointDatasets (%d)\n", count);
  
  for (i = 0; i < count; i++) {
    printf("  (%s, %s, %d, %g, %g)\n", datasets[i].name, datasets[i].units,
           datasets[i].numSamples, datasets[i].offset, datasets[i].scale);
  }
  return 0;
}


HANDLE InModifyMode
      printf("InModifyMode ()\n");

  return 0;
}

HANDLE InImageMode
      printf("InImageMode ()\n");

  return 0;
}


HANDLE InSharpStyle
  printf("InSharpStyle ()\n");
  return 0;
}


HANDLE InSewingStyle
      printf ("InSewingStyle (%g, %g, %g, %g, %g, %g, %g)\n",
              setpoint, bottomDelay, topDelay, pullBackDistance,
              moveDistance, moveRate, maxDistanceToApproach);

  return 0;
}


HANDLE InForceCurveStyle
      printf("InForceCurveStyle (%f, bunch-o-parameters)\n", setpoint);

  return 0;
}


HANDLE InDirectZControl
  printf("InDirectZControl (%g, %g, %g, %g, %g, %g, %g)\n",
         max_z_step, max_xy_step, min_setpoint, max_setpoint,
         max_lateral_force, freespace_norm_force, freespace_lat_force);
  return 0;
}


HANDLE PointData
  int i;

  printf ("PointData (%g, %g, %u:%u, %d)\n", 
          x, y, sec-first_time_sec, usec, reports);

  printf("  (");
  for (i = 0; i < reports; i++) {
    printf("%g ", data[i]);
  }
  printf(")\n");

  return 0;
}


HANDLE ForceCurveData
  int i, j;

  printf("ForceCurveData (%d samples, %d halfcycles, %u:%u at "
         "%g,%g)\n",
         numSamples, numHalfcycles, sec-first_time_sec,usec, x, y);

  for (i = 0; i < numSamples; i++) {
    printf("%f -->", samples[i].z);
    for (j = 0; j < numHalfcycles; j++)
      printf(" %f", samples[i].d[j]);
    printf("\n");
  }

  return 0;
}


// OTHER


HANDLE ReportRelaxTimes
  printf("ReportRelaxTimes (%d, %d)\n", minTime, sepTime);

  return 0;
}

HANDLE StartingToRelax
  printf ("StartingToRelax (%u:%u)\n", sec-first_time_sec, usec);

  return 0;
}



HANDLE TopoFileHeader
  printf("TopoFileHeader (%d, %s)\n", length, header);

  return 0;
}










int main (int argc, char ** argv) {

  vrpn_Connection * connection;
  vrpn_File_Connection * fc;

  connection = vrpn_get_connection_by_name (argv[1]);

  // register all the message types and handlers

REGISTER InContactMode;
REGISTER InOscillatingMode;
REGISTER ReportScanrateNM;
REGISTER HelloMessage;
REGISTER ReportPID;
REGISTER ReportGridSize;
REGISTER ReportScanRegionNM;
REGISTER ReportMaxScanRangeNM;
REGISTER SlowScanPause;
REGISTER SlowScanResume;
REGISTER ReportScanWindow;
REGISTER ReportScanDatasets;
REGISTER WindowLineData;
REGISTER ReportPointDatasets;
REGISTER InModifyMode;
REGISTER InImageMode;
REGISTER InSharpStyle;
REGISTER InSewingStyle;
REGISTER InForceCurveStyle;
REGISTER InDirectZControl;
REGISTER PointData;
REGISTER ForceCurveData;
REGISTER ReportRelaxTimes;
REGISTER StartingToRelax;
REGISTER TopoFileHeader;

  fc = connection->get_File_Connection();

  while (fc && !fc->eof()) {
    connection->mainloop();
  }

}

