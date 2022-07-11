// slot_machine_win_check.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "node.h"
#include <stack>

using namespace std;

int amountOfSymbols; //amount of symbols
int Scatter;
int Wild;
int rows, columns;
vector<vector<int>> reels;
vector<vector<int>> paylines;
vector<vector<int>> paytable;
vector<int> random;
int activePaylines;
vector<vector<int>> output;
vector<vector<int>> winCombo; // first: number of recurrence, second: sign, third: payline
int payout;
int payoutTree;

vector<node> root;
int minWinRe;	//minimum amount of recurrences of any sig, so there is still a winning

int returnActivePaylines()
{
	//return 800;
	return paylines.size();
}

void readFromFile(string fileName)
{
	fstream file;
	file.open(fileName);
	if (file.good())
		cout << "Opened successfully\n";
	string line;
	getline(file, line); //dummy line
	getline(file, line); //amount of symbols
	amountOfSymbols = atoi(line.c_str());

	getline(file, line); //dummy line
	getline(file, line); //wild
	if (line != "-1")
		Wild = atoi(line.c_str());
	getline(file, line); //dummy line
	getline(file, line); //scatter
	if (line != "-1")
		Scatter = atoi(line.c_str());

	getline(file, line); //dummy line
	getline(file, line);
	rows = atoi(line.c_str());
	getline(file, line);
	columns = atoi(line.c_str());

	getline(file, line); //dummy line
	getline(file, line); //first reel
	vector<int> oneReel;

	for (int i; line != "//paylines"; getline(file, line))	//reels
	{
		stringstream ss(line);
		while (ss >> i)
			oneReel.push_back(i);
		reels.push_back(oneReel);
		oneReel.clear();
	}

	getline(file, line); //first payline
	vector<int> onePayline;

	for (int i; line != "//paytable"; getline(file, line))		//paylines
	{
		stringstream ss(line);
		while (ss >> i)
			onePayline.push_back(i);
		paylines.push_back(onePayline);
		onePayline.clear();
	}


	activePaylines = returnActivePaylines();

	paytable.resize(amountOfSymbols);
	for (int i = 0; i < amountOfSymbols; i++)
		paytable[i].resize(columns + 1, 0);


	minWinRe = columns;
	int index2, value2;
	for (int index; line != "//end";)
	{
		getline(file, line); //index  of sign
		index = atoi(line.c_str());
		while (line != "//next" && line != "//end")
		{
			getline(file, line);
			stringstream ss(line);
			ss >> index2; //number of repetitions
			ss >> value2; //winning da moniez
			paytable[index][index2] = value2;
			if (index2 < minWinRe)
				minWinRe = index2;
		}
	}
}

void showPaytable()
{
	for (int i = 0; i < paytable.size(); i++)			//paytable
	{
		cout << endl << i << ":\n";
		for (int j = 0; j < paytable[i].size(); j++)
			cout << j << " " << paytable[i][j] << endl;
	}
}

void spinReels()
{
	random.clear();
	for (int i = 0; i < columns; i++)		//one spin
	{
		int b = i;
		random.push_back(rand() % reels[i].size());
	}

}

void buildOutput()
{
	output.clear();
	output.resize(rows);
	for (int i = 0; i < rows; i++)		//writing in matrix for easier future calculation
	{
		output[i].resize(columns);
		for (int j = 0; j < columns; j++)
			output[i][j] = reels[j][(random[j] + i) % reels[j].size()];
	}
}

void showOutput()
{
	for (int i = 0; i < output.size(); i++)		//displayed
	{
		for (int j = 0; j < output[i].size(); j++)
			cout << output[i][j] << "\t";
		cout << endl;
	}
	cout << endl;
}

void checkForWinnings()
{
	winCombo.clear();
	int findThisSign, check;
	bool isWild;
	vector<int> oneCombo;  //0: numberOfRecurrences,  1: sign,  2: Payline
	int numberOfRecurrences;
	for (int currentPayline = 0; currentPayline < activePaylines; currentPayline++)
	{
		numberOfRecurrences = 1;
		findThisSign = output[paylines[currentPayline][0]][0];
		for (int i = 0; findThisSign == Wild && i < columns; i++)		//to find what sign are we looking for
		{
			if (output[paylines[currentPayline][i]][i] != Wild)
			{															//if first is wild, we check second, etc..., until we come to end of line
				findThisSign = output[paylines[currentPayline][i]][i];	//if last one is also wild, we are checking just for wild
				break;
			}
		}

		for (int currentColumn = 0; currentColumn < columns - 1; currentColumn++)
		{
			check = output[paylines[currentPayline][currentColumn + 1]][currentColumn + 1];
			if (findThisSign == check || check == Wild)
				numberOfRecurrences++;
			else break;
		}
		if (paytable[findThisSign][numberOfRecurrences]>0)
		{
			oneCombo.resize(3);
			oneCombo[0] = numberOfRecurrences;
			oneCombo[1] = findThisSign;
			oneCombo[2] = currentPayline;
			winCombo.push_back(oneCombo);
			oneCombo.clear();
		}
	}
}

void showWinnings()
{
	for (int i = 0; i < winCombo.size(); i++)
		cout << "sign: " << winCombo[i][1] << "  recurrences: " << winCombo[i][0] << "  payline: " << winCombo[i][2] << "  payout: " << paytable[winCombo[i][1]][winCombo[i][0]] << endl;
}

void addPayouts()
{
	for (int i = 0; i < winCombo.size(); i++)
		payout += paytable[winCombo[i][1]][winCombo[i][0]];
}

void addPayoutsTree()
{
	for (int i = 0; i < winCombo.size(); i++)
		payoutTree += paytable[winCombo[i][1]][winCombo[i][0]];
}


//	------------------------ TREE

void buildTree(node *n, int k)
{
	int check;
	for (int i = 0; i < n->hasPaylines.size(); i++)
	{
		check = paylines[n->hasPaylines[i]][n->column + 1];
		if (n->kids[check] == NULL)
		{
			n->kids[check] = new node;
			n->kids[check]->row = check;
			n->kids[check]->column = n->column + 1;
			n->kids[check]->addDummyKids(rows);
		}
		n->kids[check]->insertPayline(n->hasPaylines[i]);
	}
	if (n->column != columns - 2)
		for (int i = 0; i < n->kids.size(); i++)
			if (n->kids[i] != NULL)
				buildTree(n->kids[i], k + 1);
}

void buildTreeWithLessPaylines(node *n)
{
	int check;
	for (int i = 0; i < n->hasPaylines.size(); i++)
	{
		check = paylines[n->hasPaylines[i]][n->column + 1];
		if (n->kids[check] == NULL)
		{
			n->kids[check] = new node;
			n->kids[check]->row = check;
			n->kids[check]->column = n->column + 1;
			n->kids[check]->addDummyKids(rows);
		}
		n->kids[check]->insertPayline(n->hasPaylines[i]);
	}
	if (n->column != columns - 2)
		for (int i = 0; i < n->kids.size(); i++)
			if (n->kids[i] != NULL)
				buildTreeWithLessPaylines(n->kids[i]);
}

void showTree(node *n)
{
	if (n == NULL)
		return;

	cout << "reel: " << n->column << " row: " << n->row << " paylines: ";
	for (int j = 0; j < n->hasPaylines.size(); j++)
		cout << n->hasPaylines[j] << " ";
	cout << "\n";
	for (int i = 0; i < n->kids.size(); i++)
		showTree(n->kids[i]);
}

void checkForWinningsTreeR(node *n, int findThisSign)
{
	vector<int> oneCombo;   //0: numberOfRecurrences,  1: sign,  2: Payline
	for (int i = 0; i < rows; i++)
	{
		node *CurrentKid = n->kids[i];
		if (CurrentKid != NULL && CurrentKid->hasPaylines[0] < activePaylines)  //if there is a kid AND  payline with smallest index is in that kid
		{
			int columnOfKidI = CurrentKid->column;
			int out = output[n->row][n->column];
			int outNew = output[CurrentKid->row][CurrentKid->column];
			if (out != Wild)
				findThisSign = out;
			if (outNew == Wild || findThisSign == Wild || findThisSign == outNew)
			{
				if (columnOfKidI == columns - 1)	// last node, full payline is winning payline
				{
					for (int j = 0; j < CurrentKid->hasPaylines.size() && CurrentKid->hasPaylines[j] < activePaylines; j++)
						if (paytable[findThisSign][rows]>0)
						{
							oneCombo = { columns ,findThisSign,CurrentKid->hasPaylines[j] };
							winCombo.push_back(oneCombo);
							oneCombo.clear();
						}
				}
				else //keep checking
					checkForWinningsTreeR(CurrentKid, findThisSign);
			}
			else
			{
				for (int j = 0; j < CurrentKid->hasPaylines.size() && CurrentKid->hasPaylines[j] < activePaylines; j++)
				{
					if (paytable[findThisSign][columnOfKidI] > 0)
					{
						oneCombo = { columnOfKidI ,findThisSign,CurrentKid->hasPaylines[j] };
						winCombo.push_back(oneCombo);
						oneCombo.clear();
					}
				}
			}
		}
	}
}

void checkForWinningsTreeWithLessPaylinesR(node *n, int findThisSign)
{
	vector<int> oneCombo;   //0: numberOfRecurrences,  1: sign,  2: Payline
	for (int i = 0; i < rows; i++)
	{
		node *CurrentKid = n->kids[i];
		if (CurrentKid != NULL)  //if there is a kid 
		{
			int columnOfKidI = CurrentKid->column;
			int out = output[n->row][n->column];
			int outNew = output[CurrentKid->row][columnOfKidI];
			if (out != Wild)
				findThisSign = out;
			if (outNew == Wild || findThisSign == Wild || findThisSign == outNew)
			{
				if (columnOfKidI == columns - 1)	// last node, full line is winning payline
				{
					for (int j = 0; j < CurrentKid->hasPaylines.size(); j++)
						if (paytable[findThisSign][rows]>0)
						{
							oneCombo = { columns ,findThisSign,CurrentKid->hasPaylines[j] };
							winCombo.push_back(oneCombo);
							oneCombo.clear();
						}
				}
				else //keep checking
					checkForWinningsTreeWithLessPaylinesR(CurrentKid, findThisSign);
			}
			else
				for (int j = 0; j < CurrentKid->hasPaylines.size(); j++)
					if (columnOfKidI >= minWinRe)
						if (paytable[findThisSign][columnOfKidI] > 0)
						{
							oneCombo = { columnOfKidI ,findThisSign,CurrentKid->hasPaylines[j] };
							winCombo.push_back(oneCombo);
							oneCombo.clear();
						}
		}
	}
}

void checkForWinningsTreeWithLessPaylinesNonRecursive(node *n)
{
	vector<int> oneCombo;   //0: numberOfRecurrences,  1: sign,  2: Payline
	stack<node*> s;
	n->findThisSign = output[n->row][n->column];
	s.push(n);
	node *CurrentKid;
	int columnOfKidI;
	int out;
	int outNew;
	int findThisSign;

	while (s.size() != 0)
	{
		n = s.top();
		s.pop();
		for (int i = 0; i < rows; i++)
		{
			CurrentKid = n->kids[i];
			if (CurrentKid != NULL)  //if there is a kid 
			{
				findThisSign = n->findThisSign;
				columnOfKidI = CurrentKid->column;
				out = output[n->row][n->column];
				outNew = output[CurrentKid->row][columnOfKidI];

				if (outNew == Wild || findThisSign == Wild || findThisSign == outNew)
				{
					if (columnOfKidI == columns - 1)	// last node, full line is winning payline
					{
						for (int j = 0; j < CurrentKid->hasPaylines.size(); j++)
							if (paytable[findThisSign][rows]>0)
							{
								oneCombo = { columns,findThisSign,CurrentKid->hasPaylines[j] };
								winCombo.push_back(oneCombo);
								oneCombo.clear();
							}
					}
					else //keep checking
					{
						CurrentKid->findThisSign = findThisSign;
						s.push(CurrentKid);
					}
				}
				else
				{
					for (int j = 0; j < CurrentKid->hasPaylines.size(); j++)
					{
						if (paytable[findThisSign][columnOfKidI] > 0)
						{
							oneCombo = { columnOfKidI ,findThisSign,CurrentKid->hasPaylines[j] };
							winCombo.push_back(oneCombo);
							oneCombo.clear();
						}
					}
				}
			}
		}
	}
}



int main()
{
	cout << "\n\t PARTIAL TREE\n";
	readFromFile("slotMachine-5x5.txt");  //for better effect try slotMachine-5x5no2.txt
	cout << "grid size: " << rows << "x" << columns << "\n";
	//showPaytable();
	payout = 0;
	payoutTree = 0;
	clock_t start, allofthem, loops;
	int timeChecking = 0;
	int timeCheckingTree = 0;
	int timeCheckingTreeNONR = 0;
	int timeCheckingTreeSmaller = 0;
	int timeSpin = 0;
	int timebuildOutput = 0;
	int payoutsCalculatingTime = 0;

	int numberOfIterations = 1000;
	cout << "activePaylines: " << returnActivePaylines() << endl;
	cout << "iterations: " << numberOfIterations << endl;

	start = clock();
	for (int i = 0; i < rows; i++)
	{
		node n(i, 0, rows);
		root.push_back(n);
		for (int j = 0; j < activePaylines; j++)
			if (paylines[j][0] == i)
				root[i].insertPayline(j);
		//buildTree(&root[i], 0);					//building a full tree
		buildTreeWithLessPaylines(&root[i]); //building a tree with only paylines < activePaylines
	}


	cout << "time building tree: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";

	cout << "working...  0%";
	int k = 0;
	allofthem = clock();
	for (int i = 0; i < numberOfIterations; i++)
	{
		if ((i + 1) / (numberOfIterations / 20) != k)
			cout << '\r' << "working...  " << ++k * 10 / 2 << "%";

		start = clock();
		spinReels();
		timeSpin += (clock() - start);

		start = clock();
		buildOutput();
		timebuildOutput += (clock() - start);

		//output[0][0] = Wild;
		//showOutput();

		start = clock();
		checkForWinnings();								// older ckecking algorithm
		timeChecking += (clock() - start);
		//cout << "winnings using backtracking alg:\n";
		//showWinnings();
		start = clock();
		addPayouts();
		payoutsCalculatingTime += clock() - start;


		//winCombo.clear();								//ONE UNIVERSAL TREE
		//start = clock();
		//for (int i = 0; i < root.size(); i++)
			//checkForWinningsTreeR(&root[i], Wild);
		//timeCheckingTree += clock() - start;


		winCombo.clear();								//TREE BASED ON ACTIVE_PAYLINES
		start = clock();
		for (int i = 0; i < root.size(); i++)
			checkForWinningsTreeWithLessPaylinesR(&root[i], Wild);
		timeCheckingTreeSmaller += clock() - start;


		//winCombo.clear();								//NONRECURSIVE
		//start = clock();
		//for (int i = 0; i < root.size(); i++)
			//checkForWinningsTreeR(&root[i], Wild);
			//checkForWinningsTreeWithLessPaylinesR(&root[i], Wild);
			//checkForWinningsTreeWithLessPaylinesNonRecursive(&root[i]);
		//timeCheckingTreeNONR += (clock() - start);
		//cout << "winnings using tree alg:\n";
		//showWinnings();
		start = clock();
		addPayoutsTree();
		payoutsCalculatingTime += clock() - start;


	}
	cout << "\n\npayout of backtracking algorithm: " << payout << endl;
	cout << "payout of tree algorithm: " << payoutTree;



	cout << "\n\nall: " << (clock() - allofthem) / (double)CLOCKS_PER_SEC << "s" << endl;
	cout << "time spinning: " << timeSpin / (double)CLOCKS_PER_SEC << "s (" << timeSpin * 100 / (clock() - allofthem) << "%)" << endl;
	cout << "time building: " << timebuildOutput / (double)CLOCKS_PER_SEC << "s (" << timebuildOutput * 100 / (clock() - allofthem) << "%)" << endl;
	cout << "time adding payouts: " << payoutsCalculatingTime / (double)CLOCKS_PER_SEC << "s (" << payoutsCalculatingTime * 100 / (clock() - allofthem) << "%)" << endl;
	cout << "time checking with backtracking alg: " << timeChecking / (double)CLOCKS_PER_SEC << "s (" << timeChecking * 100 / (clock() - allofthem) << "%)" << endl;
	cout << "time checking with tree: " << timeCheckingTreeSmaller / (double)CLOCKS_PER_SEC << "s (" << timeCheckingTreeSmaller * 100 / (clock() - allofthem) << "%)" << endl;


	cout << "\n\nRelatively: ";
	cout << "\ntime checking with backtracking: " << timeChecking * 100 / timeChecking << "%\n";
	//cout << "checking with tree: " << timeCheckingTree * 100 / timeChecking << "%\n";
	cout << "time checking with tree: " << timeCheckingTreeSmaller * 100 / timeChecking << "%\n";
	//cout << "checking with smaller tree nonrecursive: " << timeCheckingTreeNONR * 100 / timeCheckingTreeSmaller << "%\n";


	cin.ignore();
	cin.get();
	return 0;
}
















/*

//2.a
void vrniGlobinoR(vozlisce v, int &maxGlobina)
{
	if (v == NULL)
		return;
	if (v->stevec > v->imenovalec)   //globina je ve�ji od �tevca & imenovalca
	{
		if (v->stevec > maxGlobina)
			maxGlobina = v->stevec;
	}
	else
	{
		if (v->imenovalec > maxGlobina)
			maxGlobina = v->imenovalec;
	}
	vrniGlobinoR(v->levi, maxGlobina);
	vrniGlobinoR(v->desni, maxGlobina);
}

int vrniGlobino()
{
	int maxGlobina = 0;
	vrniGlobinoR(koren, maxGlobina);
	return maxGlobina;
}

//2.b prvi na�in
void prestejVozlisca(vozlisce v, int &stevilo)
{
	if (v == NULL)
		return;
	if (v->stevec / v->imenovalec > 1 / 2)   //globina je ve�ji od �tevca & imenovalca
		stevilo++;

	prestejVozlisca(v->levi, stevilo);
	prestejVozlisca(v->desni, stevilo);
}

int vrniPrestejVozlisca()
{
	int stevilo = 0;
	prestejVozlisca(koren, stevilo);
	return stevilo;
}


//2.b drugi na�in
int prestejVozlisca(vozlisce v)
{
	if (v == NULL)
		return 0;
	if (v->stevec / v->imenovalec > 1 / 2)   //globina je ve�ji od �tevca & imenovalca
		return 1 + prestejVozlisca(v->levi) + prestejVozlisca(v->desni);
	else
		return prestejVozlisca(v->levi) + prestejVozlisca(v->desni);
}

//2.c
void jeLevoPoravnano(vozlisce v, int trenutnaGlobina, bool jePoravnano)
{
	if (v == NULL)
		return;

}


// 3.
void izpisNazajZLinearnimProstorom()
{
	Element v = zacetek;
	seznam nov;
	Element x;
	while (v != NULL)
	{
		x->kljuc = v->kljuc;		// vstavljanje na
		x->naslednji = nov->zacetek;// zacetek novega seznama
		nov->zacetek = x;			//
		v = v->naslednji;
	}
	v = nov->zacetek;
	while (v != NULL)
		cout << v->kljuc << " ";
}

// 3.
void izpisNazajSKonstantnimProstorom()
{
	Element v1 = zacetek;
	Element v2, v3;
	while (v3 != NULL)
	{
		v2 = v1->naslednji;
		v3 = v2->naslednji;
		
		v2->naslednji = zacetek;
		v1->naslednji = v3;
		zacetek = v2;
	}
	v1 = zacetek;
	while (v1 != NULL)
		cout << v1->kljuc << " ";
}





*/