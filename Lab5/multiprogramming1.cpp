#include <bits/stdc++.h>
using namespace std;
#include "asgn5.cpp"

struct process_compare_vector
{
  bool operator()(process c1, process c2) {
    return c1.admission < c2.admission;
  }
};

class process_compare_priority {
	  bool reverse;

public:

  bool operator()(process c1, process c2) {
    return c1.start_priority > c2.start_priority;
  }
};

class MultiScheduler
{
	list<process> Allprocesses;
	priority_queue<process,vector<process>,process_compare_priority> FIFO;
	int present_time;

public:
	MultiScheduler()
	{
		process_proc_file();
		copy(process_list.begin(),process_list.end(),back_inserter(Allprocesses));
		Allprocesses.sort(process_compare_vector());
		present_time = 0;
	}
	void print()
	{
		cout<<"#################################################################\n";
		priority_queue<process,vector<process>,process_compare_priority> just_a_copy = FIFO;
		while(!just_a_copy.empty())
		{
			process p = just_a_copy.top();
			cout<<"PROCESS "<<p.p_id<<" "<<p.start_priority<<" "<<p.admission<<endl;
			  for (list<process_phase>::iterator it = p.phases.begin(); it != p.phases.end(); it++)
			  	cout<<it->iterations<<" "<<it->cpu_time<<" "<<it->io_time<<endl;
				just_a_copy.pop();
		}
		cout<<"#################################################################\n";
	}
	bool clearResults(list<pair<int,int> >* l,int present_time,int pid)
	{
		for(list<pair<int,int> >:: iterator it=l->begin();it!=l->end();it++)
		{
			if(it->first == pid)
			{
				if(present_time > it->second)
					return it->second - present_time;
				else 
					return -1;
			}


		}

		return -1;
	}
	void schedule()
	{
		list<pair<int,int> > clearProcess;
		//processid and the time;

		while(true)
		{
			print();
			process p1;
			if(FIFO.empty())
			{
				if(Allprocesses.size() == 0)
					break;
				else
				{
					p1 = Allprocesses.front();
					Allprocesses.pop_front();
					FIFO.push(p1);
					present_time+=p1.admission;
					continue;
				}
			}
			else
			{
				if(Allprocesses.size() > 0)
				{
					process p2 = Allprocesses.front();
					if(p2.admission == present_time)
					{
						FIFO.push(p2);
						Allprocesses.pop_front();
					}
				}
				p1 = FIFO.top();
				FIFO.pop();

				if(p1.phases.size() > 0)
				{
					process_phase np = p1.phases.front();
					p1.phases.pop_front();

					np.cpu_time--;
					if(np.cpu_time > 0)
					{
						p1.phases.push_front(np);
						FIFO.push(p1);
					}
					else if(np.cpu_time == 0)
					{
						p1.phases.push_front(np);
						FIFO.push(p1);
						clearProcess.push_back(pair<int,int>(p1.p_id,present_time+np.io_time));
					}
					else
					{	
						int pp;
						if((pp=clearResults(&clearProcess,present_time,p1.p_id)) != -1)
						{

							present_time +=pp;
							continue;
						}

					}
				}

			}
			present_time++;
		}
	}

};