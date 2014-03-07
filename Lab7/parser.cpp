#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

struct process
{
	int number;
	int size;
	int nn;
	int mm;
};

class parser
{
	string filename;
	vector<process> process_list;
public:
	parser(string file)
	{
		filename = file;
	};
	void parse()
	{
		ifstream fs(filename.c_str());
		string line;
		if(fs.is_open())
		{
			while(getline(fs,line))
			{
				istringstream iss(line);
				string token;
				vector<string> tokens;
				while(getline(iss,token,' '))
				{	
					tokens.push_back(token);
				}
				process p;
				const char * c1 = tokens[1].c_str();
				p.number = atoi(c1);
				const char * c2 = tokens[2].c_str();
				p.size = atoi(c2);

				string page_frame;
				istringstream iss1(tokens[3]);
				vector<string> frames;
				while(getline(iss1,page_frame,'-'))
				{
					frames.push_back(page_frame);
				}
				const char * c3 = frames[0].c_str();
				const char * c4 = frames[1].c_str();
				p.nn = atoi(c3);
				p.mm = atoi(c4);
				process_list.push_back(p);
			}
		}
	}
	vector<process> getProcessList()
	{
		parse();
		return process_list;
	}
	
};
struct action
{
	bool mode;
	int page_no;
	int word_no;
};
// mode = True access
// mode = False modify
class spec_parser
{
	string filename;
	vector<action> action_list;
public:
	spec_parser(string file);
	{
		filename =file;
	}
	void parse()
	{
		ifstream fs(filename.c_str());
		string line;
		if(fs.is_open())
		{
			while(getline(fs,line))
			{
				istringstream iss(line);
				string token;
				vector<string> tokens;
				while(getline(iss,token,' '))
				{	
					tokens.push_back(token);
				}
				if(tokens[0].compare("end")==0)
					break;

				action p;
				if(tokens[0].compare("access") == 0)
					p.mode = true;
				else
					p.mode = false;
				const char * c1 = tokens[1].c_str();
				p.page_no = atoi(c1);
				const char * c2 = tokens[2].c_str();
				p.word_no = atoi(c2);
				action_list.push_back(p);
			}
		}
	}
	vector<process> getActionList()
	{
		parse();
		return action_list;
	}
};