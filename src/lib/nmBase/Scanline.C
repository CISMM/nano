#include "Scanline.h"

Scanline_data::Scanline_data (BCString name, BCString units, int length)
{
    d_dataset = name;
    d_units = units;
    d_value = new float[length];
    d_length = length;
    int i;
    for (i = 0; i < length; i++)
	d_value[i] = 0.0;
}

Scanline_data::Scanline_data (Scanline_data *value)
{
    d_dataset = value->d_dataset;
    d_units = value->d_units;
    d_length = value->d_length;
    d_value = new float[d_length];
    for (int i = 0; i < d_length; i++)
	d_value[i] = value->d_value[i];
}

int Scanline_data::resize(int newlength) 
{
    if (newlength != d_length){
    	if (d_value) delete d_value;
    	d_value = new float[newlength];
    	if (!d_value) return -1;
    }
    int i;
    for (i = 0; i < newlength; i++)
	d_value[i] = 0.0;
    d_length = newlength;
    return 0;
}

Scanline_results::Scanline_results(const Scanline_results &scr)
{

    d_num_values = scr.d_num_values;
    d_length = scr.d_length;
    d_x0 = scr.d_x0; d_y0 = scr.d_y0; d_x1 = scr.d_x1; d_y1 = scr.d_y1;
    d_z0 = scr.d_z0; d_z1 = scr.d_z1;
    d_sec = scr.d_sec; d_usec = scr.d_usec;

    int i;
    for (i = 0; i < scr.d_num_values; i++){
	d_data[i] = new Scanline_data(scr.d_data[i]);
    }
}

Scanline_results::~Scanline_results()
{
    int i;
    for (i = 0; i < d_num_values; i++){
        delete d_data[i];
    }
}

Scanline_data *Scanline_results::addChannel(BCString dataset, BCString units)
{
    if (d_num_values == MAX_SCANLINE_CHANNELS-1) return NULL;
    d_data[d_num_values] = new Scanline_data(dataset, units, d_length);
    d_num_values++;
    return 0;
}

int Scanline_results::setLength(int length)
{
    int i;
    for (i = 0; i < d_num_values; i++){
        if (d_data[i]->resize(length)) return -1;
    }
    d_length = length;
    return 0;
}

int Scanline_results::clearChannels()
{
    int i;
    for (i = 0; i < d_num_values; i++){
	delete d_data[i];
	d_data[i] = NULL;
    }
    printf("cleared\n");
    d_num_values = 0;
    return 0;
}
