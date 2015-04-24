#pragma once

#ifndef LINESTRUCT_H_H_H
#define LINESTRUCT_H_H_H

#include <iostream>
#include "Point.h"
#include <vector>
#include <string>
#include <cstdlib>

class LineStruct
{
public:
	
	//vector<vector<Point>> path_block;
	vector<vector<Point>> path_block;
	string str;

	LineStruct(vector<vector<Point>> p, string s)
	{
		path_block = p;
		str = s;
	}

	~LineStruct(void)
	{
		for (size_t i = 0; i < path_block.size(); i ++)
		{
			path_block(i).clear();
		}
		path_block.clear();
		str = NULL;
 	}
};
#endif
