
/*****************************************************************************/
// cnt_glue.cpp
/*****************************************************************************/
#include <string>

#ifdef _WIN32
  #include <iostream>
  #include <fstream>
  #include <strstream>	
  using namespace std;
#else
  #include <iostream.h>
  #include <fstream.h>
  #include <strstream.h>	// sstream (newest) vs strstream: sstream not supported by cygwin
#endif

#include <vector>


#include "cnt_glue.h"
#include "cnt_ia.h"
#include "robot.h"


/*****************************************************************************/
class Token {
public:
	string name;

	Token( void )     : name("") {}
	Token( string s ) : name(s)  {}
};

//	vector<string> fruit;		// Why doesn't this work? (p452 Stroustrup3e)
//	fruit.pushBack( "apple" );	// wouldn't need Token class if I could do this


/**************************************************************************************/
// Read a line from the input file and parse it into words (separated
// by whitespace).  Return a vector of tokens (the words).
vector<Token>
parseLine( ifstream& inputFile )
{
	// Get one newline-terminated (or EOF-terminated) line from file.
	string line;
	getline( inputFile, line, '\n' );
			//	cout << "Input:  " << line << "\n";

	// Treat the line as a stream: parse into whitespace-separated words.
	istrstream lineStream( line.c_str() );
//	const char* pCharArray = line.c_str();
//	istrstream lineStream( pCharArray );

	// Read words of line into vector of tokens (strings).
	vector<Token> tokenVec(0);	// starts empty
	do {
		string word;
		lineStream >> word;
		tokenVec.push_back( Token(word) );
	} while( ! lineStream.eof() );

	// Return the vector of tokens (parse of line from input file).
	return tokenVec;
}


/**************************************************************************************/
vector<OB> 
extractFoundTubesFromFile( char *FileName ) 
{
	// Tube parameters from recognition algorithm:
	double X;	// X coord of tube center
	double Y;	// Y coord of tube center
	double A;	// orientation of tube (angle w.r.t X axis)
	double L;	// length of tube (full length, counting end spheres)
	double W;	// width of tube (diameter)
	
	// Create an empty vector of tubes.
	// Add a tube to the vector for each tube param block in input file.
	vector<OB> foundTubeVec( 0 );
	
	// Set up file stream for input file.
	ifstream inputFile( "param.dat" );
	if( !inputFile ) {
		error( "Can't open param.dat (output from image analysis) file" );
				//cout << "*****Can't open file." << endl;
				//return foundTubeVec;
	}
	else             {
//		cout << "*****Opened file." << endl;         
	}
 

	// Repeat processing tube data blocks until end of input file.
	while( ! inputFile.eof() ) {
		// Define vector of white-space separated tokens.
		vector<Token> tok(0);

		// First line of tube parameter block.
		// Example: ===== CNT ID = 2193 =====
		//          00000 111 22 3 4444 55555
		tok = parseLine( inputFile );
		if( tok.size() == 0  ||  !(tok[0].name == "=====") )   
			break;   // nothing left, so quit

		// Second line of tube parameter block.
		// Example: Position : X =   32.367 Y =   37.605
		//          00000000 1 2 3   444444 5 6   777777
		tok = parseLine( inputFile );
		string tokX = tok[4].name;
		string tokY = tok[7].name;

		// Third line of tube parameter block.
		// Example: Orientation :   -0.545 degree
		//          00000000000 1   222222 333333
		tok = parseLine( inputFile );
		string tokA = tok[2].name;

		// Fourth line of tube parameter block.
		// Example: Length =   36.528 Width =    7.971
		//          000000 1   222222 33333 4    55555
		tok = parseLine( inputFile );
		string tokL = tok[2].name;
		string tokW = tok[5].name;

		// Fifth line of tube parameter block.
		// blank line
		tok = parseLine( inputFile );


		// Treat tokens as streams: convert tokens to numeric values.
		istrstream tokStreamX( tokX.c_str() );   tokStreamX >> X;
		istrstream tokStreamY( tokY.c_str() );   tokStreamY >> Y;
		istrstream tokStreamA( tokA.c_str() );   tokStreamA >> A;
		istrstream tokStreamL( tokL.c_str() );   tokStreamL >> L;
		istrstream tokStreamW( tokW.c_str() );   tokStreamW >> W;

		// Convert angle to proper units.
		A *= ( 3.141592 / 180. );   // degrees to radians
				//cout << "X:" << X << " ";
				//cout << "Y:" << Y << " ";
				//cout << "A:" << A << " ";
				//cout << "L:" << L << " ";
				//cout << "W:" << W << " ";
				//cout << "\n";

		// Construct tube pose.
		OB foundTube;
		foundTube.pos   = Vec2d( X, Y );
		foundTube.angle = A;
		foundTube.leng  = L - W;  // tube.leng is length of axis, not full length
		foundTube.diam  = W;

		// Push tube just found onto tube vector;
		foundTubeVec.push_back( foundTube );
	}

//	cout << "*****End of file\n\n";

	return foundTubeVec;
}


/**************************************************************************************/
	// This was previously main.cpp of the stand-alone image analysis code.
void
findTubesInImage( void )
{
//	foundTubeCount = 0;

	// The sim writes image data to file "simout.ppm".
	cntRec.cnt_image_read("simout.ppm", 0.0, 0.0, 0.0, 1.5, 2.0, 150.0, 0.6);

	cntRec.cnt_image_flat();
	cntRec.cnt_image_filter();
	cntRec.cnt_image_medial();
	cntRec.cnt_image_fit();
	cntRec.cnt_image_label();
	cntRec.cnt_image_select();

	// write out PPM files to show intermediate results (for debugging).
	cntRec.cnt_image_write("blur.ppm", cntRec.cnt_image_Blr);
	cntRec.cnt_image_write("medial.ppm", cntRec.cnt_image_Med);
	cntRec.cnt_image_write("mask.ppm", cntRec.cnt_image_Msk);

	// Tube parameters for recognized tubes are written to text file 'param.dat".

}






