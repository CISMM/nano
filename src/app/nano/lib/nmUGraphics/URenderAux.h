
struct BBOX {double xmin,ymin,zmin,xmax,ymax,zmax;};

//SELECTION SET -- keeps track of a list of objects that were selected by a Pick
//operation

#define MAXSELECT 30
class URender;
class SelectionSet{
	public:
	
	double p1[3],p2[3];	
        URender *sset[MAXSELECT];
        int numselected;
        SelectionSet(){numselected=0;}
        ~SelectionSet(){}
        void AddSelection(URender *r);
};


