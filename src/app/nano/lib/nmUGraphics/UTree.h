#ifndef UTREE_H
#define UTREE_H

class UTree;
class URender;
#include "URender.h"
#include "Xform.h"

#define CHILDCHUNK 10			//# of slots to add if I overflow the child array
#define MAX_TREE_DEPTH 200		//maximum levels of the tree

typedef int (URender::*IteratorFunction)(void *userdata);
class UTree{
	private:
		//main graphics data members
		URender *contents;  	//ptrs to URender node

		//main tree structure data members
		UTree **children;	//array to hold children ptrs
		int size;		//size of children array
		int num_children;	//# of filled slots in children array
		UTree *parent;		//ptr to parent node

		//helper functions
		int TGetPathByName(const char *node_name,int depth,UTree **scratch );
					//node_name name to find
					//depth is to track the recursion depth
					//scratch is to build a list of visited node
					//	during the traversal
	public:
		//constructors & destructors
		UTree(URender *r=NULL);
		~UTree();

		//io functions -- not yet fully implemented ... these are just
		//stubs
		void TSaveTreeConfiguration(char *filename);
		void TLoadTreeConfiguration(char *filename);
		friend ostream& operator<< (ostream&,const UTree& t);

		//render functions
		void TRender();

		//selection Functions
		/*TSelectByPoint();
		TSelectByRay();
		TClearSelection();
		TMarkSelections();*/

		//tree management functions
		void TPruneBranch(UTree *branch_to_prune);
		void TGraftBranch(UTree *graft_point, UTree *branch_to_graft);
		int  TReparent(const char *node_name,
		               const char *new_parent_name);
		void TAddTreeNode(UTree *);
		void TAddNode(URender *renderable, char *name);
		void TRemoveTreeNode(UTree *);

		//set functions
		void TSetName(char *newname);
		void TSetContents(URender *r){if(contents!=NULL) delete r;
						contents=r;
		}

		//iterators
	        int Do(IteratorFunction ifunc,void *userdata=NULL);

		//get functions
		const char* TGetName();
		UTree *TGetNodeByName(const char *nodename);	//retrieve a node by name
		UTree *TGetParent();
		Xform TGetXformByName(const char *from_node, const char* to_node);
							//traverse the tree between
							//two nodes and build the matrix
		URender& TGetContents(){return *contents;}
		UTree& TGetChild(int i){if(i < num_children); return *children[i];}
};


//GLOBAL VARIABLES FOR NANO
extern UTree World;
extern UTree Textures;

#endif

