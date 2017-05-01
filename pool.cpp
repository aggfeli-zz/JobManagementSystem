/* 
 * File:   Pool.cpp
 * Author: angelique
 * 
 * Created on April 13, 2017, 10:07 AM
 */

#include "pool.h"
#define SIZE 1024

Pool::Pool(int temp) 
{
    jobs = temp;
    current_jobs = 0;
    jobs_status = new int[jobs];
    jobs_id = new int[jobs];
    jobs_pid = new int[jobs];
}

Pool::Pool(const Pool& orig) {
}

Pool::~Pool() 
{
    //cout << "Pool destructor" << endl;
    delete []jobs_status;
    delete []jobs_id;
    delete []jobs_pid;
}

void Pool::IncreaseJobs(int counter)
{
    current_jobs++;
    //cout << "Current Jobs " << current_jobs << endl;
    jobs_id[current_jobs] = counter;
}

void Pool::Job_submit(char *temp, int position, int flag, char *path)
{
    char poolin[100], poolout[100], directory[100], str_time[100], str_date[100];
    time_t t;
    struct tm *tm;
    sprintf(poolin, "%spool%din%d", path, position, current_jobs); 
    sprintf(poolout,"%spool%dout%d", path, position, current_jobs);
    
    //int fd; /*file descriptor to the file we will redirect ls's output*/

    pid_t kid = fork();
    
    
    if (kid == -1) {
        fprintf(stderr, "[%d]: fork: %s\n", getpid(), strerror(errno));
    }
    else if (kid == 0) 
    {
      /* child */   
        jobs_pid[current_jobs] = getpid();      
        
        //printf ( "Job: Child’s PID: %d\n", getpid());
        //printf ( "Job: Parent’s PID: %d\n", getppid()); 
        char str[SIZE];
        int mess_from_coord, mess_to_coord;
        mess_from_coord = open(poolin, O_RDONLY);
        mess_to_coord = open(poolout, O_WRONLY);


        //read(mess_from_coord, str, SIZE);
        jobs_status[current_jobs] = 1;

        t = time(NULL);
        tm = localtime(&t);

        strftime(str_time, sizeof(str_time), "%H%M%S", tm);
        strftime(str_date, sizeof(str_date), "%Y%m%d", tm);
       
        sprintf(directory, "%ssdi1300192_%d_%d_%s_%s", path, jobs_id[current_jobs], getpid(), str_date, str_time ); 
        if (mkdir(directory, 0700) == -1)
            cout << "error: mkdir :" << directory << endl;
       
        char ** argv;      
        int counter = 0;
        while ((argv[counter] = strtok(temp, " ")) != NULL)
        {
            //printf("<<%s>>\n", argv[counter]);
            counter++;
            temp = NULL;
        }
        argv[counter+1] = NULL;
        //char file[260];
        //sprintf(file, "%s%S/stdout_%d.txt", path, directory, getpid()); 
        
        int fd = open("stdout_jobid.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(fd == -1){
            write(2, "Error1\n", 7);
        }
        
        int fd2 = open("stderr_jobid.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(fd2 == -1){
            write(2, "Error1\n", 7);
        }
        dup2(fd, 1);   // make stdout go to file
        dup2(fd2, 2);  // make stderr go to file 
                       
        close(fd);     // fd no longer needed 
        close(fd2);    // fd no longer needed
        execvp(argv[0], argv);
        jobs_status[current_jobs] = 0;
        strcpy(str, "Job is done!");                          
        write(mess_to_coord,str,SIZE);


        close(mess_from_coord);
        close(mess_to_coord);

        unlink(poolin);
        unlink(poolout);
            
        //printf("[%d]\n", getpid());
        //printf ( "Child : Hello I am job process\n");
        //printf ( "Child Job: Child’s PID: %d\n", getpid());
        //printf ( "Child Job: Parent’s PID: %d\n", getppid());   
        
        exit(EXIT_SUCCESS);
    }
    else {
      /* parent */
        pid_t pid;
        int status;
        pool_pid = getpid();
        
        pid = waitpid(kid, &status, 0);

        if (pid == -1)
          cout << "error: waitpid:" << getpid()<< endl;
        //printf ( "Parent Coord: Hello I am the parent process\n" ) ;
        //printf ( "Parent Coord: Parent’s PID: %d\n", getpid());
       // printf ( "Parent Coord: Child’s PID: %d\n", kid);
    }
    
    
}

int Pool::get_ID(int j)
{
    return jobs_id[j];
}

int Pool::get_Status(int j)
{
    return jobs_status[j];
}

int Pool::get_Jobs()
{
    return jobs;
}

pid_t Pool::get_Pid()
{
    return pool_pid;
}

pid_t Pool::get_JobPid(int j)
{   
    return jobs_pid[j];  
}

int Pool::get_current_Jobs()
{
    return current_jobs;
}

void Pool:: suspendJob(int j)
{
    if(jobs_status[j] == 1) //If job is active, suspend it
    {
        jobs_status[j] = 2; //Make status 2 which is for suspended
        if (kill(jobs_pid[j], SIGSTOP) == -1)
            cout << "SIGSTOP error" << endl;
    }
    else
        cout << "Job can't be suspended" << endl;
}

void Pool:: resumeJob(int j)
{
    if(jobs_status[j] == 2) //If job has been suspended, activate it again
    {
        jobs_status[j] = 1; //Make status 1 which is for active
        if (kill(jobs_pid[j], SIGCONT) == -1)
            cout << "SIGCONT error" << endl;  
    }
    else
        cout << "Job can't be activated" << endl;
}

void Pool:: Shutdown(int &activejobs)
{
    for(int i = 0; i < current_jobs; i++)
    {
        if(kill(jobs_pid[i], 0) == 0) {             //Check if process exists and then terminate it
            activejobs++;
        if (kill(jobs_pid[i], SIGTERM) == -1)
            cout << "SIGTERM error" << endl;  
        }   cout << kill(jobs_pid[i], 0) << endl;
    }
    if(kill(pool_pid, 0) == 0){
        if (kill(pool_pid, SIGTERM) == -1)
                cout << "SIGTERM error" << endl;  
    }
}