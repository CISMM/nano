#include "patternFile.h"

PatternFile::PatternFile()
{

}

void PatternFile::setPattern(list<PatternShape> &pattern)
{
  d_pattern = pattern;
}

list<PatternShape> &PatternFile::getPattern()
{
  return d_pattern;
}

int PatternFile::writeToFile(const char *filename)
{
  FILE *fout = fopen(filename, "w");
  if (!fout) {
    return -1;
  }
  int result = writeToFile(d_pattern, fout);
  fclose(fout);
  return result;
}

int PatternFile::writeToFile(list<PatternShape> &shapes, FILE *fout)
{
  list<PatternShape>::iterator shape;
  for (shape = shapes.begin(); shape != shapes.end(); shape++) {
    if ((*shape).d_type == PS_COMPOSITE) {
      fprintf(fout, "COMPOSITE_BEGIN\n");
      writeToFile((*shape).d_shapes, fout);
      fprintf(fout, "COMPOSITE_END\n");
    } else {
      if ((*shape).d_type == PS_POLYGON) {
        fprintf(fout, "POLYGON_BEGIN\n");
        fprintf(fout, "%g\n%g\n", (*shape).d_lineWidth_nm,
                                  (*shape).d_exposure_uCoulombs_per_square_cm);
      } else if ((*shape).d_type == PS_POLYLINE) {
        fprintf(fout, "POLYLINE_BEGIN\n");
        fprintf(fout, "%g\n%g\n", (*shape).d_lineWidth_nm,
                                  (*shape).d_exposure_uCoulombs_per_square_cm);
      } else if ((*shape).d_type == PS_DUMP) {
        fprintf(fout, "DUMP_BEGIN\n");
      }
      list<PatternPoint>::iterator point;
      for (point = (*shape).pointListBegin(); 
           point != (*shape).pointListEnd(); point++) {
        fprintf(fout, "%g %g\n", (*point).d_x, (*point).d_y);
      }
      if ((*shape).d_type == PS_POLYGON) {
        fprintf(fout, "POLYGON_END\n");
      } else if ((*shape).d_type == PS_POLYLINE) {
        fprintf(fout, "POLYLINE_END\n");
      } else if ((*shape).d_type == PS_DUMP) {
        fprintf(fout, "DUMP_END\n");
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
  int result = readFromFile(d_pattern, fin);
  fclose(fin);
  return result;
}

int PatternFile::readFromFile(list<PatternShape> &shapes, FILE *fin)
{
  char line[128];
  int lineSize = 128;
  PatternShape shape;
  double width, exposure;
  double x, y;
  while (fgets(line, lineSize, fin)) {
    if (!strncmp(line, "COMPOSITE_BEGIN", strlen("COMPOSITE_BEGIN"))) {
      shape = PatternShape(0, 0, PS_COMPOSITE);
      if (readFromFile(shape.d_shapes, fin)) {
        return -1;
      }
    } else if (!strncmp(line, "COMPOSITE_END", strlen("COMPOSITE_END"))) {
      return 0;
    } else if (!strncmp(line, "POLYGON_BEGIN", strlen("POLYGON_BEGIN"))) {
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &width);
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &exposure);
      shape = PatternShape(width, exposure, PS_POLYGON);
      while (fgets(line, lineSize, fin)) {
        if (!strncmp(line, "POLYGON_END", strlen("POLYGON_END"))) {
          break;
        }
        sscanf(line, "%lf %lf", &x, &y);
        shape.addPoint(x, y);
      }
    } else if (!strncmp(line, "POLYLINE_BEGIN", strlen("POLYLINE_BEGIN") )) {
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &width);
      if (!fgets(line, lineSize, fin)) {
        return -1;
      }
      sscanf(line, "%lf", &exposure);
      shape = PatternShape(width, exposure, PS_POLYLINE);
      while (fgets(line, lineSize, fin)) {
        if (!strncmp(line, "POLYLINE_END", strlen("POLYLINE_END"))) {
          break;
        }
        sscanf(line, "%lf %lf", &x, &y);
        shape.addPoint(x, y);
      }
    } else if (!strncmp(line, "DUMP_BEGIN", strlen("DUMP_BEGIN") )) {
      shape = PatternShape(width, exposure, PS_DUMP);
      while (fgets(line, lineSize, fin)) {
        if (!strncmp(line, "DUMP_END", strlen("DUMP_END"))) {
          break;
        }
        sscanf(line, "%lf %lf", &x, &y);
        shape.addPoint(x, y);
      }
    }

    shapes.push_back(shape);
  }
  return 0;
}
