#include <bits/stdc++.h>
using namespace std;
#include "asgn5.cpp"
class process_compare {
	  bool reverse;

public:

  bool operator()(process c1, process c2) {
    return c1.admission > c2.admission;
  }
};
struct process_compare_vector
{
  bool operator()(process c1, process c2) {
    return c1.admission < c2.admission;
  }
};
class TimeSharing
{
	int timeSlot;
	vector<process> processes;
	list<process> remainingProcesses;
	queue<process> FIFO;
	int last_time_slot;

	public:
		TimeSharing(int time)
		{	
			timeSlot = time;
			process_proc_file();
			processes = process_list;
			sort(processes.begin(), processes.end(),process_compare_vector());
			copy(process_list.begin(),process_list.end(),back_inserter(remainingProcesses));
			last_time_slot = processes[processes.size()-1].admission;


		}
		void print()
		{
			cout<<"###############################################################\n";
			queue<process> just_a_copy = FIFO;
			while(!just_a_copy.empty())
			{
				process p = just_a_copy.front();
				cout<<"PROCESS "<<p.p_id<<" "<<p.start_priority<<" "<<p.admission<<endl;
				  for (list<process_phase>::iterator it = p.phases.begin(); it != p.phases.end(); it++)
				  	cout<<it->iterations<<" "<<it->cpu_time<<" "<<it->io_time<<endl;
 				just_a_copy.pop();
			}
			cout<<"###############################################################\n";

		}
		void schedule()
		{
			int time_spent = 0;
			int present_time=0;

			while(true)
			{
				list<process> rmp = remainingProcesses;
				for(list<process>::iterator it = rmp.begin();it!=rmp.end();it++)
				{
					if(it->admission <= present_time)
					{
						FIFO.push(*it);
						remainingProcesses.pop_front();
						break;
					}
					else
						break;
				}
				cout<<"PRESENT TIME::: "<<present_time<<endl;
				print();
				process p1;
				if(FIFO.empty())
				{
					if(remainingProcesses.size()!=0)
					{
						p1 = remainingProcesses.front();
						remainingProcesses.pop_front();
						FIFO.push(p1);
						present_time=p1.admission;
						continue;
					}
					else
						break;
				}
				else
				{
					process p1 = FIFO.front();
					if(present_time == 0)
						present_time = p1.admission;
					FIFO.pop();
					list<process_phase> pp = p1.phases;
					process_phase f = pp.front();
					pp.pop_front();
					if(f.cpu_time > timeSlot)
					{
						if(f.iterations > 1)
						{
							cout<<p1.p_id<<"\n"<<f.iterations<<" "<<f.cpu_time<<" "<<f.io_time<<endl;
							f.iterations--;
							pp.push_front(f);
						}
						process_phase  np;
						np.iterations =1;
						np.cpu_time = f.cpu_time - timeSlot;
						np.io_time = f.io_time;
						cout<<p1.p_id<<"\n"<<np.iterations<<" "<<np.cpu_time<<" "<<np.io_time<<endl;
						pp.push_front(np);
						p1.phases = pp;
						FIFO.push(p1);
						present_time+=timeSlot;
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
							FIFO.push(p1);
					}

				}
			}
		}

};

