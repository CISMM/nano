#include "patternFile.h"

PatternFile::PatternFile()
{

}

void PatternFile::setPattern(ExposurePattern &pattern)
{
  d_pattern = pattern;
}

ExposurePattern &PatternFile::getPattern()
{
  return d_pattern;
}

int PatternFile::writeToFile(const char *filename)
{
  FILE *fout = fopen(filename, "w");
  if (!fout) {
    return -1;
  }
  int result = writeToFile(d_pattern.getSubShapes(), fout);
  fclose(fout);
  return result;
}

int PatternFile::writeToFile(list<PatternShapeListElement> &shapes, FILE *fout)
{
  PolylinePatternShape *polyline;
  PolygonPatternShape *polygon;
  CompositePatternShape *composite;
  DumpPointPatternShape *dump;

  PatternShape *shapePtr;
  list<PatternPoint>::iterator point;

  list<PatternShapeListElement>::iterator shape;
  for (shape = shapes.begin(); shape != shapes.end(); shape++) {
    shapePtr = (*shape).d_shape; 
    polyline = (PolylinePatternShape *)shapePtr;
    polygon = (PolygonPatternShape *)shapePtr;
    composite = (CompositePatternShape *)shapePtr;
    dump = (DumpPointPatternShape *)shapePtr;

    if (shapePtr->type() == PS_COMPOSITE) {
      fprintf(fout, "COMPOSITE_BEGIN\n");
      writeToFile(composite->getSubShapes(), fout);
      fprintf(fout, "COMPOSITE_END\n");
    } else if (shapePtr->type() == PS_DUMP) {
      fprintf(fout, "DUMP_BEGIN\n");
      fprintf(fout, "%g %g\n", dump->d_location.d_x, dump->d_location.d_y);
      fprintf(fout, "DUMP_END\n");
    } else {
      if (shapePtr->type() == PS_POLYGON) {
        fprintf(fout, "POLYGON_BEGIN\n");
        fprintf(fout, "%g\n", polygon->d_exposure_uCoulombs_per_square_cm);
        for (point = polygon->d_points.begin();  
             point != polygon->d_points.end(); point++) {
          fprintf(fout, "%g %g\n", (*point).d_x, (*point).d_y);
        }
        fprintf(fout, "POLYGON_END\n");
      } else if (shapePtr->type() == PS_POLYLINE) {
        fprintf(fout, "POLYLINE_BEGIN\n");
        fprintf(fout, "%g\n%g\n%g\n", polyline->d_lineWidth_nm,
                                polyline->d_exposure_pCoulombs_per_cm,
                                polyline->d_exposure_uCoulombs_per_square_cm);
        for (point = polygon->d_points.begin();
             point != polygon->d_points.end(); point++) {
          fprintf(fout, "%g %g\n", (*point).d_x, (*point).d_y);
        }
        fprintf(fout, "POLYLINE_END\n");
      }
    }
  }
  return 0;
}

int PatternFile::readFromFile(const char *filename)
{
  FILE *fin = fopen(filename, "r");
  if (!fin) {
    return -1;
  }
  int result = readFromFile(d_pattern.getSubShapes(), fin);
  
  fclose(fin);
  return result;
}

int PatternFile::readFromFile(list<PatternShapeListElement> &shapes, FILE *fin)
{
  char line[128];
  int lineSize = 128;
  PatternShape *shape;
  double width, line_exposure, area_exposure;
  double x, y;

  PolylinePatternShape *polyline;
  PolygonPatternShape *polygon;
  CompositePatternShape *composite;
  DumpPointPatternShape *dump;

  while (fgets(line, lineSize, fin)) {
    if (!strncmp(line, "COMPOSITE_BEGIN", strlen("COMPOSITE_BEGIN"))) {
      composite = new CompositePatternShape();
      shape = composite;
      if (readFromFile(composite->getSubShapes(), fin)) {
        return -1;
      }
    } else if (!strncmp(line, "COMPOSITE_END", strlen("COMPOSITE_END"))) {
      return 0;
    } else if (!strncmp(line, "POLYGON_BEGIN", strlen("POLYGON_BEGIN"))) {
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &area_exposure);
      polygon = new PolygonPatternShape(area_exposure);
      shape = polygon;
      while (fgets(line, lineSize, fin)) {
        if (!strncmp(line, "POLYGON_END", strlen("POLYGON_END"))) {
          break;
        }
        sscanf(line, "%lf %lf", &x, &y);
        polygon->addPoint(x, y);
      }
    } else if (!strncmp(line, "POLYLINE_BEGIN", strlen("POLYLINE_BEGIN") )) {
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &width);
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &line_exposure);
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &area_exposure);

      polyline = new PolylinePatternShape(width, line_exposure, area_exposure);
      shape = polyline;
      while (fgets(line, lineSize, fin)) {
        if (!strncmp(line, "POLYLINE_END", strlen("POLYLINE_END"))) {
          break;
        }
        sscanf(line, "%lf %lf", &x, &y);
        polyline->addPoint(x, y);
      }
    } else if (!strncmp(line, "DUMP_BEGIN", strlen("DUMP_BEGIN") )) {
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf %lf", &x, &y);
      dump = new DumpPointPatternShape(x, y);
      shape = dump;
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      if (!strncmp(line, "DUMP_END", strlen("DUMP_END"))) {
        return -1;
      }
    }

	shape->setParent(&d_pattern);
    shapes.push_back(PatternShapeListElement(shape));
  }
  return 0;
}
