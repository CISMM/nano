#include "BCDebug.h"
#include "BCGrid.h"
#include "GridList.h"

BCGridList* inputGridList;
BCGridList* snapshotGridList;
BCGridList* outputGridList;


main(int argc, char* argv[])
{
    BCDebug debug("main", GRID_LIST_CODE);
    
    int num_x = 100;
    int num_y = 102;

    double x_min = 0;
    double x_max = 200;
    double y_min = 0;
    double y_max = 202;

    double rate = 0.5; 
    int read_mode = 0;

    char file_name[255];
    strcpy(file_name,argv[1]);
    read_mode = READ_FILE;
    
    cout << "creating input list... " << endl;

    inputGridList = new BCGridList(num_x, num_y,
				   x_min, x_max, y_min, y_max,
				   rate, read_mode, file_name);

    if (inputGridList->empty())
    {
	fprintf(stderr,"inputGridList empty\n");
    }

    cout << "creating snapshot and output list..." << endl;

    snapshotGridList = new BCGridList(num_x, num_y,
				     x_min, x_max, y_min, y_max);

    outputGridList = new BCGridList(num_x, num_y,
				    x_min, x_max,
				    y_min, y_max);

    BCGrid* grid;

    BCGrid* grid2;

    cout << "a" << endl;
    
    grid = inputGridList->getGridByName("height");

    cout << grid;

    inputGridList->decimate(100, 100);

        grid = inputGridList->getGridByName("height");

    cout << grid;
    
    grid2 = grid;
    // add to the outputGridList two copies of the height grid stored in the input grid list
    cout << "about to add a copy of height to outputGridList" << endl;
    grid = outputGridList->addGridCopy(grid);

    cout << outputGridList;

    cout << "about to rename height foo" << endl;
    grid->rename("foo");

    cout << grid;
	
    cout << "about to add a copy of foo to outputGridList..." << endl;
    grid = outputGridList->addGridCopy(grid);

    cout << outputGridList;    
    // rename the second copy added "color"
            cout << "d" << endl;
    grid->rename("color");

    cout << grid2;

    inputGridList->deleteHead();
    
    if (inputGridList->empty())
    {
	fprintf(stderr,"inputGridList empty\n");
    }
    
    cout << grid;

    cout << outputGridList;

    outputGridList->deleteHead();
    
    cout << outputGridList;

    grid = outputGridList->head();

    cout << grid;
}





