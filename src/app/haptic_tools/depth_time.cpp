/*

Kent Rosenkoetter

*/


#include "common.h"


#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;


int main(int argc, char ** argv)
{
	string subject("Subject");
	string session("_Session");
	string s1("session_one_shapes");
	string s2("session_two_shapes");
	string shape("shape");

	string alexandra("alexandra");
	string Alexandra("Alexandra");
	string atsuko("atsuko");
	string Atsuko("Atsuko");
	string tom("tom");
	string Tom("Tom");
	string trace("_explore_viewer_trace_");
	string txt(".txt");
	double threshold(2);
	double dead_time(3);
	unsigned int first_subject(1);
	unsigned int last_subject(111);
	unsigned int s;

	vector<string> people_list;
	people_list.push_back(alexandra);
	people_list.push_back(Alexandra);
	people_list.push_back(atsuko);
	people_list.push_back(Atsuko);
	people_list.push_back(tom);
	people_list.push_back(Tom);

	vector<transformed_shape_type> shapes(21);
	vector<double> pivots(23);

	for (s = 1; s <= 10; s++) {
		string filename(s1 + '/' + shape + to_string(s) + txt);
		shapes[s-1] = read_shape(filename);
		cout << filename << " = " << flush;
		pivots[s-1] = find_pivot(shapes[s-1].first.first);
		cout << pivots[s-1] << endl;
	}
	for (s = 1; s <= 11; s++) {
		string filename(s2 + '/' + shape + to_string(s) + txt);
		shapes[s+9] = read_shape(filename);
		cout << filename << " = " << flush;
		pivots[s+9] = find_pivot(shapes[s+9].first.first);
		cout << pivots[s+9] << endl;
	}
	pivots[21] = pivots[22] = 0;

	ofstream depth_time_out;
	ofstream path_length_out;
	ofstream time_touching_out;

	depth_time_out.open("depth_time.txt");
	path_length_out.open("path_length.txt");
	time_touching_out.open("time_touching.txt");

	depth_time_out << "### subject";
	path_length_out << "### subject";
	time_touching_out << "### subject";
	for (s = 1; s <= 10; s++) {
		depth_time_out << "\tsession 1 shape " << s;
		path_length_out << "\tsession 1 shape" << s;
		time_touching_out << "\tsession 1 shape" << s;
	}
	for (s = 1; s <= 13; s++) {
		depth_time_out << "\tsession 2 shape " << s;
		path_length_out << "\tsession 2 shape" << s;
		time_touching_out << "\tsession 2 shape" << s;
	}
	depth_time_out << endl;
	path_length_out << endl;
	time_touching_out << endl;

	for (unsigned int sub = first_subject; sub <= last_subject; sub ++) {
		stringstream depth_time_buffer;
		stringstream path_length_buffer;
		stringstream time_touching_buffer;

		string dir(subject + to_string(sub) + session);

		depth_time_buffer << sub;
		path_length_buffer << sub;
		time_touching_buffer << sub;

		bool output1(false);
		bool output2(false);

		string dir1(dir + to_string(1) + '/');
		for (s = 1; s <= 10; s++) {
			datum_list path;
			vector<string>::const_iterator pp(people_list.begin());
			while (pp != people_list.end()) {
				string person(*pp++);
				string filename(dir1 + person + trace + to_string(s) + txt);
				try {
					datum_list temp_path(read_path(filename, dead_time));
					cout << filename << " = " << temp_path.size() << endl;
					output1 = true;
					path = temp_path;
					break;
				} catch (string st) {
					if (st.compare(string("not open")) != 0)
						cerr << "failed to read file " << filename << " : " << st << endl;
				}
			}
			if (output1) {
				//cout << 'a' << endl;
				depth_time_buffer << '\t' << depth_time(path, pivots[s-1]);
				//cout << 'b' << endl;
				path_length_buffer << '\t' << path_length(path);
				//cout << 'c' << endl;
				time_touching_buffer << '\t' << time_touching_shape(path, shapes[s-1], threshold);
				//cout << 'd' << endl;
			} else {
				depth_time_buffer << '\t' << 'x';
				path_length_buffer << '\t' << 'x';
				time_touching_buffer << '\t' << 'x';
			}
		}

		string dir2(dir + to_string(2) + '/');
		for (s = 1; s <= 13; s++) {
			datum_list path;
			vector<string>::const_iterator pp(people_list.begin());
			while (pp != people_list.end()) {
				string person(*pp++);
				string filename(dir2 + person + trace + to_string(s) + txt);
				try {
					datum_list temp_path(read_path(filename, dead_time));
					cout << filename << " = " << temp_path.size() << endl;
					output2 = true;
					path = temp_path;
					break;
				} catch (string st) {
					if (st.compare(string("not open")) != 0)
						cerr << "failed to read file " << filename << " : " << st << endl;
				}
			}
			if (output2) {
				//cout << 'e' << endl;
				depth_time_buffer << '\t' << depth_time(path, pivots[s+9]);
				//cout << 'f' << endl;
				path_length_buffer << '\t' << path_length(path);
				//cout << 'g' << endl;
				if (s <= 11)
					time_touching_buffer << '\t'
										 << time_touching_shape(path,
																shapes[s+9],
																threshold);
				else if (s == 12)
					time_touching_buffer << '\t'
										 << time_touching_sphere(path,
																 vector<double>(3),
																 45,
																 threshold);
				else if (s == 13)
					time_touching_buffer << '\t'
										 << time_touching_torus(path,
																vector<double>(3),
																20, 40,
																threshold);
				//cout << 'h' << endl;
			} else {
				depth_time_buffer << '\t' << 'x';
				path_length_buffer << '\t' << 'x';
				time_touching_buffer << '\t' << 'x';
			}
		}

		depth_time_buffer << flush;
		path_length_buffer << flush;
		time_touching_buffer << flush;

		if (output1 && output2) {
			cout << "Outputting data for subject " << sub << endl;
			//cout << depth_time_buffer.str() << endl
			//	 << path_length_buffer.str() << endl
			//	 << time_touching_buffer.str() << endl;
			depth_time_out << depth_time_buffer.str() << endl;
			path_length_out << path_length_buffer.str() << endl;
			time_touching_out << time_touching_buffer.str() << endl;
		} else {
			cout << "Skipping data for subject " << sub << endl;
		}
	}

	depth_time_out.close();
	path_length_out.close();
	time_touching_out.close();

	return 0;
}
