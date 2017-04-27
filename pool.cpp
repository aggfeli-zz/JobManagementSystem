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
    cout << "Pool destructor" << endl;
}

void Pool::IncreaseJobs(int counter)
{
    current_jobs++;
    cout << "Current Jobs " << current_jobs << endl;
    jobs_id[current_jobs] = counter;
}

void Pool::Job_submit(char *temp, int position, int flag, char *path)
{
    //cout << "Current Jobs " << current_jobs << endl;
    char poolin[100], poolout[100], directory[100], str_time[100], str_date[100];
    time_t t;
    struct tm *tm;
    sprintf(poolin, "%spool%din%d", path, position, current_jobs); 
    sprintf(poolout,"%spool%dout%d", path, position, current_jobs);
    /*if (mkfifo(poolin, 0666) == -1)
    {
        printf("fifo in error %s", strerror(errno));
    }

    if (mkfifo(poolout, 0666) == -1)
    {
        printf("fifo out error %s", strerror(errno));
    }
    cout << "Fifo done" << endl;*/
    
    /*cout << "Child Process" << endl;
    printf ( "Child Pool: Child’s PID: %d\n", getpid());
    printf ( "Child Pool: Parent’s PID: %d\n", getppid()); 
    char str[SIZE];
    int mess_from_coord, mess_to_coord;
    mess_from_coord = open(poolin, O_RDONLY);
    mess_to_coord = open(poolout, O_WRONLY);
    //while (pool[position]->get_current_Jobs() < jobs_pool)
   // {
        read(mess_from_coord, str, SIZE);*/
                            
                            
    
   /* printf ( "Child : Hello I am job process\n");
    printf ( "Child Job: Child’s PID: %d\n", getpid());
    printf ( "Child Job: Parent’s PID: %d\n", getppid()); */
        
    

    
    pid_t kid = fork();
    
    
    if (kid == -1) {
        fprintf(stderr, "[%d]: fork: %s\n", getpid(), strerror(errno));
    }
    else if (kid == 0) 
    {
      /* child */
        cout << "Pool is gonna execute the job " << current_jobs << endl;
        jobs_pid[current_jobs] = getpid();
        cout << "Pool is gonna execute the job "  << endl;
        printf ( "Job: Child’s PID: %d\n", getpid());
        printf ( "Job: Parent’s PID: %d\n", getppid()); 
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
        
        //cout << str_time << endl;
        //cout << str_date << endl;
        sprintf(directory, "%ssdi1300192_%d_%d_%s_%s", path, jobs_id[current_jobs], getpid(), str_date, str_time ); 
        if (mkdir(directory, 0700) == -1)
            cout << "error: mkdir :" << directory << endl;
        //exec
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
        /*int mess_from_pool, mess_to_pool;
        char str[SIZE];
        mess_to_pool = open(poolin, O_WRONLY);
        if(mess_to_pool < 0) { perror ("fifo open error mess_to_pool" ); exit (1) ; }

        mess_from_pool = open(poolout, O_RDONLY);
        if(mess_from_pool < 0) { perror ("fifo open error mess_from_pool" ); exit (1) ; }

        if(write(mess_to_pool, temp, sizeof(temp)) < 0){
            perror("Write:");//print error
            exit(-1);
        }
        if(read(mess_from_pool,str,sizeof(str)) < 0){
            perror("Read:"); //error check
            exit(-1);
        }
        printf("\n...received from pool: %s\n\n\n",str);

        close(mess_to_pool);
        close(mess_from_pool);*/
        
        pid = waitpid(kid, &status, 0);
        //current_jobs--;
        if (pid == -1)
          cout << "error: waitpid:" << getpid()<< endl;
        printf ( "Parent Coord: Hello I am the parent process\n" ) ;
        printf ( "Parent Coord: Parent’s PID: %d\n", getpid());
        printf ( "Parent Coord: Child’s PID: %d\n", kid);
    }
    
    /*strcpy(str, "Done!");                          
    write(mess_to_coord,str,SIZE);
    // }

    close(mess_from_coord);
    close(mess_to_coord);

    unlink(poolin);
    unlink(poolout);*/
 
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
        if(kill(jobs_pid[i], 0) == 0) {
            activejobs++;
        if (kill(jobs_pid[i], SIGTERM) == -1)
            cout << "SIGTERM error" << endl;  
        }   cout << kill(jobs_pid[i], 0) << endl;
    }
    //cout <<"after kill 1" << endl;
    if(kill(pool_pid, 0) == 0){
        //cout<<"mphke " << endl;
        if (kill(pool_pid, SIGTERM) == -1)
                cout << "SIGTERM error" << endl;  
        //cout <<"bghke" << endl;
    }
        
    //cout <<"after kill 2" << endl;
}