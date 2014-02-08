#include <bits/stdc++.h>
#include "asgn5.cpp"
using namespace std;

struct Event
{
	int time;
	int processID;
	process p;
	int typeOfEvent;
	// 1 - Admission
	// 2 - CPU Completion
	// 3 - I/O Completion
	
	// Event() {
	// 	time = 0;
	// 	processID = 0;
	// 	typeOfEvent = 0;
	// }
};

bool operator==(const Event& lhs, const Event& rhs) {return ((lhs.time == rhs.time) && 
																(lhs.processID == rhs.processID) &&
																(lhs.typeOfEvent == rhs.typeOfEvent));}

class process_compare_priority {
	  bool reverse;

public:

  bool operator()(process c1, process c2) {
    return c1.start_priority > c2.start_priority;
  }
};

class process_compare_time {
	  bool reverse;

public:

  bool operator()(Event c1, Event c2) {
    return c1.time > c2.time;
  }
};

class MultiProgramming
{
	priority_queue<process,vector<process>,process_compare_priority> processes_waiting;
	priority_queue<Event,vector<Event>,process_compare_time> events;
	public:
		MultiProgramming()
		{
			process_proc_file();
			for(int i=0; i< process_list.size();i++)
			{
				Event e;
				e.time = process_list[i].admission;
				e.processID = process_list[i].p_id;
				e.typeOfEvent = 1;
				events.push(e);
			}
			processes_waiting.push(process_list[0]);
			
		}
		void print()
		{
			
		}
		void schedule()
		{
			process process_current;
			Event Last ;
			// while(!events.empty())
			// {
			// 	Event e1 = events.top();
			// 	cout<<e1.typeOfEvent<<" Time: "<<e1.time<<" ";
			// 	events.pop();
			// 	if(e1 == Last || e1.p.start_priority < process_current.start_priority) {

			// 		if(e1.typeOfEvent == 1) {
			// 			// Push the corresponding into the current process waiting list.
			// 			processes_waiting.push(e1.p);
			// 		}

			// 		else if(e1.typeOfEvent == 2) {
			// 			// Schedule the I/O event for this process.
			// 			Event e;
			// 			e.time = e1.time + e1.p.phases.front().cpu_time;
			// 			e.processID = e1.processID;
			// 			e.typeOfEvent = 3;
			// 			events.push(e);
			// 			// Pre-emption?
			// 		}

			// 		else if(e1.typeOfEvent == 3) {
			// 			// Remove the iterations for the phases etc. and push the process into the waiting processes.
			// 			process p1 = e1.p;
			// 			if(p1.phases.front().iterations > 1) {
			// 				p1.phases.front().iterations--;
			// 				processes_waiting.push(p1);
			// 			}
			// 			else {
			// 				if(p1.phases.size() > 1) {
			// 				p1.phases.pop_front();
			// 				processes_waiting.push(p1);
			// 				}
			// 				else;
			// 			}
			// 		}
			// 	}

			// 	else {
			// 		// This is pre-epmtion.
			// 		process_current.phases.front().cpu_time = process_current.phases.front().cpu_time - (e1.time - process_current.admission);
			// 	}

			// 	process_current = processes_waiting.top();
			// 	//cout<<process_current.p_id;
			// 	//processes_waiting.pop();
			// 	// Schedule the CPU completion event
			// 	Event e;
			// 	e.time = e1.time + process_current.phases.front().cpu_time;
			// 	e.processID = process_current.p_id;
			// 	e.typeOfEvent = 2;
			// 	events.push(e);
			// 	Last = e;

			// }
			while(!events.empty())
			{
				cout<<events.size()<<endl;
				Event e1 = events.top();
				cout<<"Type: "<<e1.typeOfEvent<<" Time: "<<e1.time<<"\n";
				cout<<events.size()<<endl;
				events.pop();
				cout<<events.size()<<endl;
				process_current = processes_waiting.top();
				cout<<events.size()<<endl;
				cout<<process_current.p_id<<endl;
				if(e1.p.start_priority < process_current.start_priority) {

					if(e1.typeOfEvent == 1) {
						// Push the corresponding into the current process waiting list.
						processes_waiting.push(e1.p);
					}

					else if(e1.typeOfEvent == 2) {
						// Schedule the I/O event for this process.
						Event e;
						e.time = e1.time + e1.p.phases.front().cpu_time;
						e.processID = e1.processID;
						e.typeOfEvent = 3;
						events.push(e);
						// Pre-emption?
					}

					else if(e1.typeOfEvent == 3) {
						// Remove the iterations for the phases etc. and push the process into the waiting processes.
						process p1 = e1.p;
						if(p1.phases.front().iterations > 1) {
							p1.phases.front().iterations--;
							processes_waiting.push(p1);
						}
						else {
							if(p1.phases.size() > 1) {
							p1.phases.pop_front();
							processes_waiting.push(p1);
							}
							else;
						}
					}
				}

				else {
					// This is pre-epmtion.
					process_current.phases.front().cpu_time = process_current.phases.front().cpu_time - (e1.time - process_current.admission);
				}

				processes_waiting.pop();
				// Schedule the CPU completion event
				Event e;
				e.time = e1.time + process_current.phases.front().cpu_time;
				e.processID = process_current.p_id;
				e.typeOfEvent = 2;
				events.push(e);
				Last = e;

			}
		}
};

