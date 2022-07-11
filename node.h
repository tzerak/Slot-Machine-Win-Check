#pragma once

#include <vector>

using namespace std;

class node
{
public:
	int row;
	int column;
	vector<int> hasPaylines;
	vector<node*> kids;
	int findThisSign;


	node();
	node(int r, int c, int rows);
	~node();

	void insertPayline(int p);
	void addDummyKids(int row);
};
