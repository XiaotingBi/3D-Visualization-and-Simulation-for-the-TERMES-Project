#pragma once

#ifndef GETDATA_H_H_H_H
#define GETDATA_H_H_H_H

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include "Point.h"

using namespace std;

class GetData
{
public:
	// size of buffer
	int LENGTH;
	// matrix size
	int column;
	int row;

	// matrix entrance
	int entrance_x;
	int entrance_y;

	// path to reach the location of a block
	vector<vector<Point>> path_block;
	vector<Point> block_location;

	GetData(string file_name);

	~GetData(void);

	// block operations : put down a block to a certain position 
	void put_down_block(Point position, vector<Point> ps);

	// split the first row to get the column and row number                 
	void split_first_row(string str);

	// split rows in the plan                             
	void split_plan_row(string str);

	// input file
	void get_input_file(string file_name);

	// get the entrance of the matrix
	void get_matrix_entrance(string str);
};
#endif
