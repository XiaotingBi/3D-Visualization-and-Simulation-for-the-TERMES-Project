// RobotVis3D.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include "Point.h"

using namespace std;

// size of buffer
int LENGTH = 1024;
// matrix size
int column = 0, row = 0;

// block operations : put down a block to a certain position 
void put_down_block(Point position, vector<Point> ps)
{
	cout << "put down: " << position.x << "," << position.y << endl;
}

// block operations : remove a block from a certain position   
void remove_block(Point position, vector<Point> ps)
{
	cout << "remove: " << position.x << "," << position.y << endl;
}

// split the first row to get the column and row number                 
void split_first_row(string str)
{
	vector<string> tokens;
	boost::split(tokens, str, boost::is_any_of(" "));   
	
	column = atoi(tokens[0].c_str());
	row = atoi(tokens[1].c_str());
}

// split rows in the plan                             
void split_plan_row(string str)
{
	vector<string> tokens;
	boost::split(tokens, str, boost::is_any_of(" ")); 
	
	// store the position of the next block
	Point pos;
	// store the path
	vector<Point> points;
	
	vector<string> sub_tokens;
	
	// the second item in tokens is the position of the next block
	boost::split(sub_tokens, tokens[1], boost::is_any_of(","));
	pos.x = atoi(sub_tokens[0].c_str());
	pos.y = atoi(sub_tokens[1].c_str());

	Point pt;
	// path
	for (size_t i = 2; i < tokens.size(); i ++)
	{
		sub_tokens.clear();

		boost::split(sub_tokens, tokens[i], boost::is_any_of(","));
		pt.x =  atoi(sub_tokens[0].c_str());
		pt.y = atoi(sub_tokens[1].c_str());		
		points.push_back(pt);
	}

	// determine the operation
	if (tokens[0].at(0) == 'a')	// a -> put down a block in that location
	{
		put_down_block(pos, points);	
	} else if (tokens[0].at(0) == 'r') // r -> remove a block in that location
	{
		remove_block(pos, points);
	} else // e -> exit
	{
		return;
	}
}

// input file            
void get_input_file(string file_name)
{
	char buffer[1024];
	ifstream fin;
	fin.open(file_name, ios::in);

	// get the first row to obtain the size of the matrix
	fin.getline(buffer, LENGTH);
	split_first_row(buffer);

	while (!fin.eof())
	{
		fin.getline(buffer, LENGTH);		
		split_plan_row(buffer);
	}
	fin.close();
}

/*
int main(int argc, char ** argv)
{
	get_input_file("F://WorkSpace//JAVAworkspace//jonnys-visualization//test1.txt");

	return 0;
}
*/
