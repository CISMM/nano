#ifndef SCANLINE_H
#define SCANLINE_H

#include <stdio.h>
#include <string>
using namespace std;

class   Scanline_data;
class   Scanline_results;

const   int     MAX_SCANLINE_CHANNELS = 32;

class Scanline_data
{
  friend class Scanline_results; // Scanline_results need access to d_next

  public:

    /// Name and units of the data set that this value stores.
    inline string*    name (void) { return &d_dataset; }
    inline string*    units (void) { return &d_units; }
    inline void         rename (string new_name) { d_dataset = new_name; }
    inline void		setunits (string units) {d_units = units;}

    inline int length() const {return d_length;}
    inline float value(int i) const {return d_value[i];}
    inline void setValue(int i, const float value) {
	d_value[i] = value;
    }

  protected: // accessible by subclasses of Scanline_value and their friends
    Scanline_data (string name, string units, int length);
    Scanline_data (Scanline_data *value);
    ~Scanline_data (void) {};
    int resize(int newlength);


    short d_length;
    string d_dataset;
    string d_units;
    Scanline_data *d_next;
    float *d_value;
};

class Scanline_results
{
  public:
    Scanline_results(void) {
	d_num_values = 0; d_sec = d_usec = 0; 
	d_x0 = d_y0 = d_x1 = d_y1 = -1.0; d_length = 0;
    }
    Scanline_results(const Scanline_results &scr);
    ~Scanline_results();

    Scanline_data *addChannel(string dataset, string units);

    inline void setTime(long sec, long usec) { d_sec = sec; d_usec = usec; }
    inline void setEndpoints(double x0, double y0, double z0,
			 double x1, double y1, double z1) 
	{ d_x0 = x0; d_y0 = y0; d_x1 = x1; d_y1 = y1;
	d_z0 = z0; d_z1 = z1;}
    int setLength(int length);
    int clearChannels();

    inline int length() const {return d_length;}
    inline int num_values() const {return d_num_values;}
    inline float value(int value_index, int dataset_index)
	const {return d_data[dataset_index]->value(value_index);}
    inline void setValue(int value_index, int dataset_index, float value)
	{d_data[dataset_index]->setValue(value_index, value);}
    inline double x0() const {return d_x0;}
    inline double y0() const {return d_y0;}
    inline double x1() const {return d_x1;}
    inline double y1() const {return d_y1;}
    inline long sec() const { return d_sec; }
    inline long usec() const { return d_usec; }
    inline double x(int i) 
	{return d_x0 + (double)i/(double)d_length*(d_x1 - d_x0);}
    inline double y(int i)
	{return d_y0 + (double)i/(double)d_length*(d_y1 - d_y0);}
    inline string *name(int dataset_index) const
	{return d_data[dataset_index]->name();};
    inline string *units(int dataset_index) const
	{return d_data[dataset_index]->units();};

  private:
    int d_num_values;
    Scanline_data *d_data[MAX_SCANLINE_CHANNELS];
 
    short d_length;
    double d_x0, d_y0, d_z0, d_x1, d_y1, d_z1;
    long d_sec, d_usec;
};

#endif
