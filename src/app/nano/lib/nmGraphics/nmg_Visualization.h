#ifndef NMG_VISUALIZATIONS_H
#define NMG_VISUALIZATIONS_H

#include <vrpn_Types.h>
#include <nmb_Interval.h>

class BCPlane;
class nmb_Dataset;
class nmb_PlaneSelection;

struct Vertex_Struct;

#define NUMBER_OF_VISUALIZATIONS 4

////////////////////////////////////////////////////////////////////
// 	     Class : nmg_Visualization
// Description : Abstract class for the various forms of visualizing
//               the data
////////////////////////////////////////////////////////////////////
class nmg_Visualization {
public:
	nmg_Visualization(nmb_Dataset *dataset);
	virtual ~nmg_Visualization();

	virtual void renderSurface() = 0;

	virtual int rebuildGrid() = 0;
	virtual int rebuildInterval(int low_row, int high_row, int strips_in_x) = 0;
	virtual int initVertexArrays(int x, int y) = 0;

	void setMinHeight(float min_height);
	void setMaxHeight(float max_height);	
	void setControlPlane(BCPlane *control);
	void setAlpha(float alpha);
	void changeDataset(nmb_Dataset *dataset);

	float getMinHeight();
	float getMaxHeight();
	float getAlpha();

	Vertex_Struct ** getBaseSurface();

protected:
	float d_maxHeight, d_minHeight;  //Height's specified in percentage of
	                                 //max height
	float d_alpha;
	BCPlane *d_control;
	unsigned char *d_texture;
	int d_tex_width, d_tex_height;
	int d_VertexArrayDim;

	nmb_Dataset *d_dataset;
	
	//Each of these should be created by the child classes, as each
	//may require a different number of passes.  Since it is assumed
	//that each is created as an array, the deletion is done in the
	//parent class.
	unsigned int *d_list_base;
	int *d_num_lists;
	nmb_Interval *last_marked;
	nmb_Interval *update;
	nmb_Interval *todo;
	Vertex_Struct *** d_vertexPtr;


    //State variables that get automatically changed that we need to
    //be able to restore for multi-pass to work right
    vrpn_bool d_just_color;

	void setUpdateAndTodo(int low_row, int high_row, int stride, 
						  int num, nmb_Interval &last_marked,
						  nmb_Interval &update, nmb_Interval &todo,
                          bool save_mark);

	void ensureMaskPlane(nmb_PlaneSelection &planes);
	void ensureTransparentPlane(nmb_PlaneSelection &planes);
	void buildMaskPlane(nmb_PlaneSelection &planes);
	void buildTransparentPlane(nmb_PlaneSelection &planes);
    
    void SaveState(); //Since we are doing multi-pass, and some variables
                      //are automatically turned off after being used, we need
                      //to be able to save them beforehand
    void RestoreState();

	int determineVertexDim(int x, int y);
	void setTexture();
	virtual void cleanUp();
	void drawLists(int base, int num);
};


////////////////////////////////////////////////////////////////////
// 	     Class : nmg_Viz_Opaque
// Description : Normal method of visualizing the data.  Where
//               everything is always displayed opaque
////////////////////////////////////////////////////////////////////
class nmg_Viz_Opaque : public nmg_Visualization {
public:
	nmg_Viz_Opaque(nmb_Dataset *dataset);
	virtual ~nmg_Viz_Opaque();

	virtual int rebuildGrid();
	virtual void renderSurface();
	virtual int rebuildInterval(int low_row, int high_row, int strips_in_x);
	virtual int initVertexArrays(int x, int y);
};


////////////////////////////////////////////////////////////////////
// 	     Class : nmg_Viz_Transparent
// Description : Visualization method that allows for portions of 
//               the data to be displayed Transparent.  Allows a user
//               to see through the data.  May have problems seeing
//               the shape though
////////////////////////////////////////////////////////////////////
class nmg_Viz_Transparent : public nmg_Visualization {
public:
	nmg_Viz_Transparent(nmb_Dataset *dataset);
	virtual ~nmg_Viz_Transparent();

	virtual int rebuildGrid();
	virtual int rebuildInterval(int low_row, int high_row, int strips_in_x);
	virtual int initVertexArrays(int x, int y);
	virtual void renderSurface();
};

////////////////////////////////////////////////////////////////////
// 	     Class : nmg_Viz_Transparent
// Description : Visualization method that allows for portions of 
//               the data to be displayed in Wire Frame mode.  This
//               should allow for the shape to be seen, while still
//               being able to see through it.
////////////////////////////////////////////////////////////////////
class nmg_Viz_WireFrame : public nmg_Visualization {
public:
	nmg_Viz_WireFrame(nmb_Dataset *dataset);
	virtual ~nmg_Viz_WireFrame();

	virtual int rebuildGrid();
	virtual int rebuildInterval(int low_row, int high_row, int strips_in_x);
	virtual int initVertexArrays(int x, int y);
	virtual void renderSurface();
};

////////////////////////////////////////////////////////////////////
// 	     Class : nmg_Viz_OpaqueTexture
// Description : 
////////////////////////////////////////////////////////////////////
class nmg_Viz_OpaqueTexture : public nmg_Visualization {
public:
	nmg_Viz_OpaqueTexture(nmb_Dataset *dataset);
	virtual ~nmg_Viz_OpaqueTexture();

	virtual int rebuildGrid();
	virtual int rebuildInterval(int low_row, int high_row, int strips_in_x);
	virtual int initVertexArrays(int x, int y);
	virtual void renderSurface();

private:
	void build_spots_at_vertices();
};

nmg_Visualization* create_new_visualization(int choice, nmb_Dataset *dataset);

#endif
