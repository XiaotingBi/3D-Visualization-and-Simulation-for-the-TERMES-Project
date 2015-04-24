#include "StdAfx.h"
#include "GetData.h"

using namespace std;

// store a path
vector<Point> points;

GetData::GetData(string file_name)
{
	LENGTH = 1024;
	column = 0;
	row = 0;

	entrance_x = 0;
	entrance_y = 0;
	
	// "F://WorkSpace//JAVAworkspace//jonnys-visualization//test1.txt"
	this->get_input_file(file_name);
}

GetData::~GetData(void)
{
	path_block.clear();
}

// block operations : put down a block to a certain position 
void GetData::put_down_block(Point position, vector<Point> ps)
{
//cout << "put down" << endl;
	path_block.push_back(ps);
	block_location.push_back(position);
}

// split the first row to get the column and row number                 
void GetData::split_first_row(string str)
{
//cout << "split first row: " << str << endl;
	vector<string> tokens;
	boost::split(tokens, str, boost::is_any_of(" "));   

	column = atoi(tokens[0].c_str());
	row = atoi(tokens[1].c_str());
}

// split rows in the plan                             
void GetData::split_plan_row(string str)
{
//cout << "split plan row: " << str << endl;
	vector<string> tokens;
	boost::split(tokens, str, boost::is_any_of(" ")); 

	points.clear();

	// store the position of the next block
	Point pos;	

	vector<string> sub_tokens;
	// the second item in tokens is the position of the next block
	boost::split(sub_tokens, tokens[1], boost::is_any_of(","));
	pos.x = atoi(sub_tokens[0].c_str());
	pos.y = atoi(sub_tokens[1].c_str());	

	Point pt;

	// pre process
	Point extra_p;
	extra_p.x = entrance_x;
	extra_p.y = entrance_y;
	if (entrance_x == 0)
	{
		extra_p.x = -4;
		points.push_back(extra_p);
		extra_p.x = -3;
		points.push_back(extra_p);
		extra_p.x = -2;
		points.push_back(extra_p);
		extra_p.x = -1;
		points.push_back(extra_p);
	} 
	else
	{
		extra_p.y = -4;
		points.push_back(extra_p);
		extra_p.y = -3;
		points.push_back(extra_p);
		extra_p.y = -2;
		points.push_back(extra_p);
		extra_p.y = -1;
		points.push_back(extra_p);		
	}

	// path
	for (size_t i = 2; i < tokens.size(); i ++)
	{
		sub_tokens.clear();

		boost::split(sub_tokens, tokens[i], boost::is_any_of(","));
		pt.x =  atoi(sub_tokens[0].c_str());
		pt.y = atoi(sub_tokens[1].c_str());		

		points.push_back(pt);		
	}

	Point temp;
	temp.x = 1;
	temp.y = 0;
	// determine the operation
	if (tokens[0].at(0) == 'a')	// a -> put down a block in that location
	{
		// the last Point(1,0) means the action of putting down a block
		temp.x = 1;
		temp.y = 0;	
	} else if (tokens[0].at(0) == 'r') // r -> remove a block in that location
	{
		// the last Point(-1,0) means the action of remove a block
		temp.x = -1;
		temp.y = 0;	
	} else // e -> exit
	{
		return;
	}
	points.push_back(temp);
	put_down_block(pos, points);
}

// get the entrance of the matrix
void GetData::get_matrix_entrance(string str)
{
//cout << "matrix entrance" << endl;
	vector<string> tokens;
	boost::split(tokens, str, boost::is_any_of(" ")); 
	// store the position of the next block
	//Point pos;	

	vector<string> sub_tokens;
	// the second item in tokens is the position of the next block
	boost::split(sub_tokens, tokens[2], boost::is_any_of(","));
	
	entrance_x = atoi(sub_tokens[0].c_str());
	entrance_y = atoi(sub_tokens[1].c_str());
}

// input file            
void GetData::get_input_file(string file_name)
{
//cout << "get input file" << endl;
	char buffer[1024];
	ifstream fin;
	fin.open(file_name, ios::in);

	// get the first row to obtain the size of the matrix
	fin.getline(buffer, LENGTH);
	split_first_row(buffer);
	// the second row
	fin.getline(buffer, LENGTH);		
	// add extra path to the reservoir
	get_matrix_entrance(buffer);
	// add real path in the matrix
	split_plan_row(buffer);

	while (!fin.eof())
	{
		fin.getline(buffer, LENGTH);		
		split_plan_row(buffer);
	}
	fin.close();
}