#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <bits/stdc++.h>
using namespace std;
#define NUMBER 4
#define BUFFER_SIZE 2
pthread_mutex_t lock1;
pthread_cond_t condv[4];
pthread_t threads[NUMBER];
int size_of_buffer = 0;
struct messages_t
{
	int send_id;
	int receive_id;
	string message;
	//messages_t();
};


bool is_waiting[4] = {false};
bool is_waiting_send[4] = {false};
list<messages_t> q_messages_t[4];
bool is_deadlock()
{
	bool ans = true;
	for(int i=0; i<4;i++)
	{
		ans = ans && (is_waiting_send[i] || is_waiting[i]);
	}
	if(ans)
	{
		bool another_ans = true;
		for(int i=0;i<4;i++)
		{
			if(is_waiting[i])
			{
				if(q_messages_t[i].empty())
				{
					another_ans =another_ans && true;
				}
				else
				{
					another_ans =false;
					break;
				}
			}
		}
		bool ans1=false;
		for (int i = 0; i < 4; ++i)
		{
			ans1 = ans1 || is_waiting_send[i];
		}
		if(ans1)
		{
			if(another_ans && size_of_buffer == BUFFER_SIZE)
			{
				printf("Deadlock\n");
				return true;
			}
		}
		else
		{
			if(another_ans)
			{
				printf("Deadlock\n");
				return true;
			}
		}
	}
	return false;
}

void* some_funct(void* args)
{	
	long id;
	id = (long) args;
	string filename= "process"+to_string(id);
	ifstream fs(filename);
	string line;
	if(fs.is_open())
	{
		while(getline(fs,line))
		{
			istringstream iss(line);
			string token;
			vector<string> tokens;
			while(getline(iss,token,':'))
			{	
				tokens.push_back(token);
			}
			if(tokens[0] == "SEND")
			{
				pthread_mutex_lock(&lock1);
				if(size_of_buffer <= BUFFER_SIZE)
				{
					messages_t m;
					m.send_id = id;
					char ck = tokens[1][0];
					char ckl[1] = {ck};
					m.receive_id = atoi(ckl);
					m.message = tokens[2]; 
					q_messages_t[m.receive_id].push_back(m);
					size_of_buffer++;
					//cout<<"is waiting "<<is_waiting[m.receive_id]<<" "<<m.receive_id<<endl;
					if(is_waiting[m.receive_id])
					{
						// is_waiting[m.receive_id] = false;
						//cout<<"sent signal to "<<m.receive_id<<endl;
						pthread_cond_signal(&condv[m.receive_id]);
					}
					printf("Process %ld: Message \"%s\" sent to process %d\n", id, (m.message.c_str()), m.receive_id);
				}

				else
				{
					printf("Process %ld: Waiting...\n", id);
					is_waiting_send[id] = true;
					if(is_deadlock())
					{
						for (int i = 0; i < 4; ++i)
						{
							pthread_kill(threads[i],SIGKILL);
						}
						exit(0);
					}
					pthread_cond_wait(&condv[id],&lock1);
					is_waiting_send[id] = false;
					messages_t m;
					m.send_id = id;
					char ck = tokens[1][0];
					char ckl[1] = {ck};
					m.receive_id = atoi(ckl);
					m.message = tokens[2]; 
					q_messages_t[m.receive_id].push_back(m);
					size_of_buffer++;
					//cout<<"is waiting "<<is_waiting[m.receive_id]<<" "<<m.receive_id<<endl;
					if(is_waiting[m.receive_id])
					{
						// is_waiting[m.receive_id] = false;
						//cout<<"sent signal to "<<m.receive_id<<endl;
						pthread_cond_signal(&condv[m.receive_id]);
					}
					printf("Process %ld: Message \"%s\" sent to process %d\n", id, (m.message.c_str()), m.receive_id);
				}
				// cout<<"Message " << m.message<<" sent by "<<id<<endl;
				pthread_mutex_unlock(&lock1);
			}
			else if(tokens[0] == "RECEIVE")
			{
					pthread_mutex_lock(&lock1);
					if(q_messages_t[id].empty()) {
						messages_t s;
						is_waiting[id] = true;
						//cout<<"awesom"<<endl;
						if(is_deadlock())
						{
							cout<<"Deadlock";
							for (int i = 0; i < 4; ++i)
							{
								pthread_kill(threads[i],SIGKILL);
							}

							exit(0);
						}
						cout<<is_deadlock()<<endl;
						pthread_cond_wait(&condv[id],&lock1);
						is_waiting[id] = false;
						s = q_messages_t[id].front();
						q_messages_t[id].pop_front();
						if(size_of_buffer == BUFFER_SIZE) {
							for(int count = 0; count < 4; count++)
							{
								if(is_waiting_send[count])
								{
									pthread_cond_signal(&condv[count]);
									break;
								}
							}
						}
						size_of_buffer--;
						printf("Process %d: Message \"%s\" received from process %d\n", s.receive_id, (s.message.c_str()), s.send_id);
					}

					else {
						messages_t s;
						s = q_messages_t[id].front();
						q_messages_t[id].pop_front();
						if(size_of_buffer == BUFFER_SIZE) {
							for(int count = 0; count < 4; count++)
							{
								if(is_waiting_send[count])
								{
									pthread_cond_signal(&condv[count]);
									break;
								}
							}
						}
						size_of_buffer--;
						printf("Process %d: Message \"%s\" received from process %d\n", s.receive_id, (s.message.c_str()), s.send_id);
					}
					pthread_mutex_unlock(&lock1);
			}
		}
		fs.close();
	}
	is_waiting_send[id] = true;
	is_waiting[id] = true;
	pthread_exit(NULL);
}
int main()
{

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&lock1, NULL);
   	void *status;
	//create four threads
	int i0=0,i1=1,i2=2,i3=3;
	int rc;
	for (int i = 0; i < 4; ++i)
		pthread_cond_init(&condv[i],NULL);
	if(rc=pthread_create(&threads[i0],&attr,some_funct,(void * )i0))
	{
		fprintf(stderr, "error:pthread_create,rc: %d\n",rc);
	}
	if(rc=pthread_create(&threads[i1],&attr,some_funct,(void * )i1))
	{
		fprintf(stderr, "error:pthread_create,rc: %d\n",rc);
	}
	if(rc=pthread_create(&threads[i2],&attr,some_funct,(void * )i2))
	{
		fprintf(stderr, "error:pthread_create,rc: %d\n",rc);
	}
	if(rc=pthread_create(&threads[i3],&attr,some_funct,(void * )i3))
	{
		fprintf(stderr, "error:pthread_create,rc: %d\n",rc);
	}
	for(int i=0; i<4;i++)
	{
		rc = pthread_join(threads[i], &status);
	      if (rc) 
	      {
	         printf("ERROR; return code from pthread_join() is %d\n", rc);
	         exit(-1);
	       }
	}
	return 0;
}