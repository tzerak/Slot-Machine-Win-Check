#include "stdafx.h"
#include "node.h"


node::node()
{
}

node::node(int r, int c, int rows)
{
	row = r;
	column = c;
	kids.resize(rows, NULL);
}

void node::addDummyKids(int rows)
{
	kids.resize(rows, NULL);
}

void node::insertPayline(int p)
{
	if (hasPaylines.size() == 0)
		hasPaylines.push_back(p);
	else
	{
		int i = 0;
		while(i < hasPaylines.size())
		{
			if (hasPaylines[i] > p)
				break;
			i++;
		}
		hasPaylines.insert(hasPaylines.begin() + i, p);
	}
}


node::~node()
{
}