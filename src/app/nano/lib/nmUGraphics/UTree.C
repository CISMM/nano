#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include "UTree.h"
#include "microscape.h"
#include "v.h"		//vlib functions for get hand and head in world

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

/*======================= CONST/DESTR FUNCTIONS ===============================*/
UTree::UTree(URender *r){


	//initialize tree contents
	contents=r;
	
	//set up tree relations
	children=NULL;
	num_children=size=0;
	parent=this;

	return;	
}

UTree::~UTree(){
	return;
}

/*======================= IO FUNCTIONS ===============================*/
ostream& operator<< ( ostream& co, const UTree& t )
{
      co << t.contents << "\n";
      return co ;
}

/*======================= SET FUNCTIONS ===============================*/
void UTree::TSetName(char *newname)
{
  if(contents==NULL) return;
  if(contents->name!=NULL) delete [](contents->name);
  contents->name = new char[strlen(newname)+1];
  strcpy(contents->name,newname);
  return;
}

/*======================= GET FUNCTIONS ===============================*/
const char* UTree::TGetName(){ 
	if(contents==NULL) return NULL; 
	else return (const char*)contents->name;
}

UTree* UTree::TGetNodeByName(const char *nodename)
{
  int i;
  UTree *temp;

  if(strcmp(nodename,contents->name)==0){
        return this;
  }

  for(i=0; i < num_children;i++){
        temp=children[i]->TGetNodeByName(nodename);
        if(temp!=NULL){
          return temp;
        }
  }
  return NULL;
}

int UTree::TGetPathByName(const char *node_name, int depth, UTree** scratch ){ 
  int i,temp;

  scratch[depth]=this;

  if(depth >= MAX_TREE_DEPTH){
        cerr << "TREE HAS EXCEEDED A LOGICAL LIMIT FOR TRANSFORM OPERATIONS --\n";
        cerr << "CHANGE MAX_TREE_DEPTH CONSTANT IN UTREE.H AND RECOMPILE\n";
	kill(getpid(),SIGINT);
	return -1;
  }

  if(strcmp(node_name,contents->name)==0){		//found the node end recurse
        return depth+1;
  }

  for(i=0; i < num_children;i++){	//recurse
        temp=children[i]->TGetPathByName(node_name,depth+1,scratch);
        if(temp!=-1){
          return temp;
        }
  }
  return -1;

}

Xform UTree::TGetXformByName(const char *from_node_name, const char *to_node_name){
  int depth1, depth2;
  int i,j,mindepth;
  Xform cum,x;
  UTree **scratch_pad1;
  UTree **scratch_pad2;

  scratch_pad1=new UTree *[MAX_TREE_DEPTH];		//build scratch space
  scratch_pad2=new UTree *[MAX_TREE_DEPTH];
  if(scratch_pad1 == NULL || scratch_pad2==NULL){
	cerr << "Unable to allocate scratch space for search -- GetPathByName\n";
	cerr << "Critical failure\n";
	kill(getpid(),SIGINT);	//send a signal
	return Xform();		//return identity 
  }
  depth1=TGetPathByName(from_node_name,0,scratch_pad1);
  depth2=TGetPathByName(to_node_name,0,scratch_pad2);

  if(depth1==-1 ){
        cerr << "Couldn't find path to node " << from_node_name << "\n";
	delete []scratch_pad1;
	delete []scratch_pad2;
        return Xform();
  }
  if(depth2==-1){
        cerr << "Couldn't find path to node " << to_node_name << "\n";
	delete []scratch_pad1;
	delete []scratch_pad2;
        return Xform();
  }
  if(depth1<depth2) mindepth=depth1;	//get the minumum node
  else mindepth=depth2;

  for(i=0; i < mindepth; i++){ if(scratch_pad1[i]!=scratch_pad2[i]) break; }
  //i should be the index of the Least Common Ancestor in the two path lists
  i--;

  if(i==-1){
        cerr << "This should NEVER HAPPEN... did you getXformByName from a node other than";
        cerr << "the root node?\n";
	delete []scratch_pad1;
	delete []scratch_pad2;
        return Xform();		//return identity in this case
  }

  //traverse the scratch pad lists and build the xformation

  //TO OPTIMIZE:  doing lots of inverts here... should probably traverse in the opposite
  //direction and invert at the end
  for(j=depth1-1; j>i; j--){
        x=scratch_pad1[j]->contents->lxform;
        x.invert();
        cum=cum*x;
  }

  for(j=i; j<depth2; j++){
                cum=cum*scratch_pad2[j]->contents->lxform;
  }
  cum.invert();
  delete []scratch_pad1;
  delete []scratch_pad2;
  return cum;

}


/*======================= TREE FUNCTIONS ===============================*/
int UTree::TReparent(const char *nodename, const char *parentname)	//returns 1 on success, 0 on failure
{
  //find node to move
  UTree *node,*parentnode;

  node=TGetNodeByName(nodename);
  if(node==NULL){
        cerr << "Could not find node by name " << nodename << "\n";
        return 0;
  }

  parentnode=TGetNodeByName(parentname);
  if(parentnode==NULL){
        cerr << "Could not find parentnode by name " << parentname << "\n";
        return 0;
  }
  if(node->parent==parentnode){
        return 1;                                       //already a child of this node
							//don't reparent again
  }
  else{
        (node->parent)->TRemoveTreeNode(node);
        parentnode->TAddTreeNode(node);
        return 1;
  }

}


void UTree::TAddNode(URender *r, char *name){

	if(r==NULL || name==NULL) return;
	
	UTree *nt;
	nt=new UTree;
	if(nt==NULL){cerr << "Memory fault! Add Render Node\n"; kill(getpid(),SIGINT);}
	nt->TSetName(name);
	nt->TSetContents(r);
	TAddTreeNode(nt);
	return;
}

void UTree::TAddTreeNode(UTree* nt)
{
	UTree **temp;
	int i;

	if(num_children == size){      //expand the list size if necessary
		temp=new UTree*[size+CHILDCHUNK];
		if(temp==NULL){
			cerr << "Could not allocate new child node memory error\n";
			kill(getpid(),SIGINT);	//send a signal
		}

		for(i=0; i < size; i++){  //copy old members over to new larger list
			temp[i]=children[i];
		}
		delete []children;              //clean up obsolete list
		children=temp;
		size+=CHILDCHUNK;               //note change in list size
	}

	//update list with new node
	children[num_children]=nt;
        nt->parent=this;
        num_children++;
        return;
}

void UTree::TRemoveTreeNode(UTree *nt)
{
 
  int i,j;
 
  if(nt->parent!=this){
        cerr << "Unexpectedly asked to remove a child from myself\n"
                 << "when the child claims a different parent\n";
        return;
  }

  for(i=0; i < num_children; i++){
        if(children[i]==nt) break;
  }

  if(i==num_children){
        cerr << "Unexcpectedly asked to remove a child which I don't think is mine\n";
	return;
  }

  nt->parent=NULL;			
  for(j=i; j < num_children-1; j++){  //shift remaining child nodes over
        children[j]=children[j+1];
  }
  children[num_children-1]=NULL;	     //update blank position and reduce child count		
  num_children--;		    

  return;

}

#if 0
#define SELECT_EPSILON 0.5
UTree* UTree::Select(URender& root)
{
  Xform x;
  int i;
  URender *temp;
  temp=NULL;
  if(obj_type==URPOLYGON){
        x=root.GetXformByName("Hand",this->name,1);
        const q_vec_type &p = x.GetTrans();
        if(bounds.xmin-SELECT_EPSILON<= p[0] && p[0]<=bounds.xmax+SELECT_EPSILON &&
           bounds.ymin-SELECT_EPSILON<= p[1] && p[1]<=bounds.ymax+SELECT_EPSILON &&
           bounds.zmin-SELECT_EPSILON<= p[2] && p[2]<=bounds.zmax+SELECT_EPSILON){
       selected=1;
          return this;
        }
  }
  for(i=0; i < numchildren; i++){
        temp=children[i]->Select(root);
        if(temp!=NULL) return temp;
  }
  return NULL;
}
#endif

/*======================= ITERATOR FUNCTIONS ===============================*/
int UTree::Do(IteratorFunction ifunc,void *data){
	int i;
	int err;    
	
    //check this nodes return value and decide to continue on its children
    //based on that

    // XXX THE PUSH AND POP SHOULD NOT BE HERE -- THIS IS A GENERALIZED ITERAATOR

    if(contents){
	glPushMatrix();
	contents->GetLocalXform().Push_As_OGL();
        err=(contents->*ifunc)(data);
    }
    else {cerr << "Unexpected empty contents\n"; err=ITER_ERROR; }

    switch(err){
        case ITER_ERROR:
        	cerr << "Encountered error operating on tree\n";
		glPopMatrix();
		return err;
	case ITER_STOP:
		glPopMatrix();
		return err;
	case ITER_CONTINUE:
		break;
	default:
        	cerr << "You should not be here -- UTREE:DO\n";
                break;
    }


    for(i=0; i < num_children; i++){
    	children[i]->Do(ifunc,data);
    }

    glPopMatrix();	//you should never get here unless you had valid contents

    return err;
}




