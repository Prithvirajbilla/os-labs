#include <bits/stdc++.h>
using namespace std;
#include "asgn5.cpp"
struct process_compare_vector
{
  bool operator()(process c1, process c2) {
    return c1.admission < c2.admission;
  }
};

class Scheduler
{
	
	vector<process> processes;
	map<sc_level,queue<process> >  processInQueue;
	//time slice and queue
	list<process> remainingProcesses;
	scheduler schd;

public:
	Scheduler()
	{
		process_proc_file();
		processes = process_list;
		sort(processes.begin(), processes.end(),process_compare_vector());
		copy(process_list.begin(),process_list.end(),back_inserter(remainingProcesses));
		process_scheduler_file();
		schd = my_scheduler;
	}

	void print()
	{
		cout<<"###############################################################\n";

		map<sc_level,queue<process> > just_a_copy = processInQueue;
		for(map<sc_level,queue<process> > :: iterator it = just_a_copy.begin(); it!=just_a_copy.end(); it++)
		{
			queue<process> just_queue = it->second;
			while(!just_queue.empty())
			{
				process p = just_queue.front();
				cout<<"PROCESS "<<p.p_id<<" "<<p.start_priority<<" "<<p.admission<<endl;
				  for (list<process_phase>::iterator it = p.phases.begin(); it != p.phases.end(); it++)
				  	cout<<it->iterations<<" "<<it->cpu_time<<" "<<it->io_time<<endl;
					just_queue.pop();
			}
		}
		cout<<"###############################################################\n";
	}
	sc_level get_sc_level(int priority)
	{
		for(list<sc_level> :: iterator it = schd.levels.begin();it!=schd.levels.end();it++)
		{
			if(it->priority == priority)
			{
				return *it;
			}
		}
		sc_level sc;
		return sc;
	}
	void schedule()
	{
			int time_spent = 0;
			int present_time=0;

			while(true)
			{
				list<process> rmp = remainingProcesses;
				list<process>::iterator it;
				for(it = rmp.begin();it!=rmp.end();it++)
				{
					if(it->admission <= present_time)
					{
						sc_level l = get_sc_level(it->start_priority);

						processInQueue[l].push(*it);
						remainingProcesses.pop_front();
						break;
					}
					else
						break;
				}
				cout<<"PRESENT TIME::: "<<present_time<<endl;
				print();
				process p1;
				sc_level l ;
				if(processInQueue.empty())
				{
					if(remainingProcesses.size()!=0)
					{
						p1 = remainingProcesses.front();
						remainingProcesses.pop_front();
						l = get_sc_level(p1.start_priority);
						processInQueue[l].push(p1);
						present_time=p1.admission;
						continue;
					}
					else
						break;
				}
				else
				{
					map<sc_level,queue<process> > :: iterator it1 = processInQueue.begin();
					process p1 = it1->second.front();
					if(present_time == 0)
						present_time = p1.admission;
					it1->second.pop();
					list<process_phase> pp = p1.phases;
					process_phase f = pp.front();
					pp.pop_front();
					if(f.cpu_time > l.time_slice)
					{
						if(f.iterations > 1)
						{
							cout<<p1.p_id<<"\n"<<f.iterations<<" "<<f.cpu_time<<" "<<f.io_time<<endl;
							f.iterations--;
							pp.push_front(f);
						}
						process_phase  np;
						np.iterations =1;
						np.cpu_time = f.cpu_time - l.time_slice;
						np.io_time = f.io_time;
						cout<<p1.p_id<<"\n"<<np.iterations<<" "<<np.cpu_time<<" "<<np.io_time<<endl;
						pp.push_front(np);
						p1.phases = pp;
						it1->second.push(p1);
						present_time+=l.time_slice;
					}
					else	
					{
						cout<<p1.p_id<<"\n"<<f.iterations<<" "<<f.cpu_time<<" "<<f.io_time<<endl;
						if(f.iterations != 1)
						{
							f.iterations--;
							pp.push_front(f);
						}
						time_spent+=f.cpu_time;
						p1.phases = pp;
						present_time+=f.cpu_time;
						cout<<"Last time slot : "<<p1.admission<<endl;
						if(pp.size() >0)
							it1->second.push(p1);
					}

				}
			}
	}

};