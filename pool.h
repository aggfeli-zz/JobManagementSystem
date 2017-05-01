/* 
 * File:   Pool.h
 * Author: angelique
 *
 * Created on April 13, 2017, 10:07 AM
 */

#ifndef POOL_H
#define	POOL_H
#include <cstdlib>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>

using namespace std;

struct Jobs{
    string status;
};

class Pool {
public:
    Pool(int temp);
    Pool(const Pool& orig);
    virtual ~Pool();
    int get_Jobs();
    int get_current_Jobs();
    void Job_submit(char *temp, int position, int flag, char *path);
    void IncreaseJobs(int counter);
    int get_ID(int j);
    int get_Status(int j);
    pid_t get_Pid();
    pid_t get_JobPid(int j);
    void suspendJob(int j);
    void resumeJob(int j);
    void Shutdown(int &activejobs);
private:
    int jobs;   //Max number of jobs inside Pool
    int current_jobs; //Current number of jobs inside Pool
    int *jobs_status; // 0 is for finished, 1 is for active, 2 is for suspended
    int *jobs_id;     //inique number 1,2,3,...  
    pid_t *jobs_pid;
    pid_t pool_pid;
};

#endif	/* POOL_H */

