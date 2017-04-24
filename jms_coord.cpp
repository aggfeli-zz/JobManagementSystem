/* 
 * File:   jms_coord.cpp
 * Author: angelique
 *
 * Created on April 11, 2017, 12:22 PM
 */

#include <cstdlib>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <string>
#include <iostream>
#include "pool.h"
#define SIZE 1024

using namespace std;

/*
 * 
 */
//void PoolProcess(char *temp, Pool *pool, int position);
int num_of_pools = 0;

int main(int argc, char** argv) 
{
    //cout << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
    int i, jobs_pool = 0, fd, len, current_jobs = 0, pools = 0;
    char buf[SIZE], *token, *path, *jms_in, *jms_out, poolin[10], poolout[11]; 
    
    
    for (i = 1; i < 8; i += 2)
    {
        if(strcmp(argv[i], "-l") == 0)
        {
            path = new char[strlen(argv[i+1]) + 1];
            strcpy(path, argv[i+1]);
            //cout << path << endl;
        }
        else if (strcmp(argv[i], "-n") == 0) {jobs_pool = atoi(argv[i+1]);}
        else if (strcmp(argv[i], "-w") == 0) 
        {
            jms_out = new char[strlen(argv[i+1]) + 1];
            strcpy(jms_out, argv[i+1]);
            //cout << jms_in << endl;
        }
        else 
        {
            jms_in = new char[strlen(argv[i+1]) + 1];
            strcpy(jms_in, argv[i+1]);            
            //cout << jms_out << endl;
        }
    }
    char *myfifo = new char[strlen(jms_in) + 1];
    strcpy(myfifo, jms_in);
    char *myfifo2 = new char[strlen(jms_out) + 1];
    strcpy(myfifo2, jms_out);
    
    int coord_to_console, console_to_coord;
    

    /* create the FIFO (named pipe) */
    fd = mkfifo(myfifo, 0666);
    if (fd == -1)
    {
        printf("fifo in error %s", strerror(errno)); fd =0;
    }
    fd = mkfifo(myfifo2, 0666);
    if (fd == -1)
    {
        printf("fifo out error %s", strerror(errno));
    }
    printf("Server ON.\n");

    /* open, read, and display the message from the FIFO */
    console_to_coord = open(myfifo, O_RDONLY | O_NONBLOCK);
    coord_to_console = open(myfifo2, O_WRONLY);
        
    Pool **pool = new Pool*[10];
    int counter = 0; //Count how many jobs has been submitted 
    char end[5];
    while (strcmp("exit",end) != 0)
    {
    
        read(console_to_coord, buf, SIZE);

        if (strcmp("exit",buf) == 0)
        {
            printf("Server OFF.\n");
            strcpy(end, "exit");
            break;
        }
        if (strcmp("",buf) != 0)
        {
            printf("Received: %s\n", buf);
            len = strlen(buf);
            if (buf[len - 1] == '\n') buf[len -1] = '\0';   //Get rid of the new line
            token = strtok(buf, "  ");
            cout << "token = "<< token << endl;
            if (strcmp(token,"submit") == 0) i = 1;
            else if (strcmp(token,"status") == 0) i = 2;
            else if (strcmp(token,"status-all") == 0) i = 3;
            else if (strcmp(token,"show-active") == 0) i = 4;
            else if (strcmp(token,"show-pools") == 0) i = 5;
            else if (strcmp(token,"show-finished") == 0) i = 6;
            else if (strcmp(token,"suspend") == 0) i = 7;
            else if (strcmp(token,"resume") == 0) i = 8;
            else if (strcmp(token,"shutdown") == 0) i = 9;
            else if (strcmp(token,"exit") == 0) i = 10;
            
            switch(i){
                case 1:                   //First case: syntax:submit <jobid> 
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    //cout << "job id is  " << temp_str << endl;
                    int position = 0, flag = 0;       
                    counter++;
                    int message_from_pool, message_to_pool;
                    for (int i = 0; i < num_of_pools; i++) //Find which pool has an empty space for a new job
                    {
                        cout << "i " << i << "  " << jobs_pool << endl;
                        if (pool[i]->get_current_Jobs() < jobs_pool)
                            position = i; 
                        else
                            position = num_of_pools + 1;
                    }
                    cout << "pool position is  " << position << endl;
                    
                    if (pool[position] == NULL) //If pool doesn't exist, create a new one.
                    {
                        cout << "mphhhhhka " << endl;
                        pool[position] = new Pool(jobs_pool);
                        flag = 1;
                        num_of_pools++;
                        //cout << "I'm gonna make a new pool process" << endl;

                    }
                    //cout <<"Pool Process" << endl;
                    pool[position]->IncreaseJobs(counter);
                    
                    /*
                     * Exec version
                     * current_jobs++;
                    char flag[5];
                    sprintf(flag, "%d", current_jobs);
                    if(current_jobs == jobs_pool)
                    {
                        current_jobs = 0;
                        pools++;
                        //sprintf(flag, "%d", current_jobs);
                    }*/
                    
                    pid_t poolprocess = fork();

                    if (poolprocess == -1) {
                        fprintf(stderr, "[%d]: fork: %s\n", getpid(), strerror(errno));
                    }
                    else if (poolprocess == 0) 
                    {
                        /* child */
                        
                        
                        /*sprintf(poolin, "pool%din%d", position, pool[position]->get_current_Jobs()); 
                        sprintf(poolout, "pool%dout%d", position, pool[position]->get_current_Jobs());
                            
                        if (mkfifo(poolin, 0666) == -1)
                        {
                            printf("fifo in error %s", strerror(errno));
                        }

                        if (mkfifo(poolout, 0666) == -1)
                        {
                            printf("fifo out error %s", strerror(errno));
                        }
                        cout << "Fifo done" << endl;*/
                        
                        pool[position]->Job_submit(temp_str, position);
                        
                        
                       /* Exec verion here!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        * char jobs[5], pool[5];
                        sprintf(jobs, "%d", jobs_pool); 
                        sprintf(pool, "%d", pools);
                        
                        execlp("./pool", "pool", jobs, pool,temp_str,flag, NULL);
                        perror("exec failure");*/

                        //printf("[%d]\n", getpid());
                        //printf ( "Child Pool: Hello I am the pool process\n");
                        //printf ( "Child Pool: Child’s PID: %d\n", getpid());
                        //printf ( "Child Pool: Parent’s PID: %d\n", getppid());    
                        exit(EXIT_SUCCESS);
                    }
                    else {
                      /* parent */
                        cout << "Coord Process" << endl;
                        pid_t pid;
                        int status;
                        pid = waitpid(poolprocess, &status, 0);
                        //current_jobs--;
                        if (pid == -1)
                          fprintf(stderr, "%d: waitpid: %s\n", getpid(), strerror(errno));
                        printf ( "Parent Pool: Hello I am the parent process\n" ) ;
                        printf ( "Parent Pool: Parent’s PID: %d\n", getpid());
                        printf ( "Parent Pool: Child’s PID: %d\n", poolprocess);

                    }
                    
                    //PoolProcess(temp_str, pool[position], position);   
                    
                        //unlink(poolin);
                        //unlink(poolout);
                    
                    cout << "pools : " << num_of_pools << endl;
                    
                    
                    
                    
                    delete temp_str; 
                    
                break;}
                case 2:                   //Second case: syntax:status <JobID> 
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    cout << "job id is  " << temp_str << endl;
                    int id = atoi(temp_str);
                    cout << "job id is  " << id << endl;
                    int flag = 0, i, j;
                    for(i = 0; i < num_of_pools; i++)
                    {
                        for(j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_ID(j) == id)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if(flag == 1) break; 
                    }
                    cout << "JobID " << id << "  Status: " << pool[i]->get_Status(j) << endl;
                    delete temp_str; 
                    
                break;}
                case 3:                   //Third case: syntax:status-all [time-duration]
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    cout << "job id is  " << temp_str << endl;
                    
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            cout << "JobID " << pool[i]->get_ID(j) << "  Status: " << pool[i]->get_Status(j) << endl;
                        }
                        
                    }                   
                    delete temp_str; 
                    
                break;}
                case 4:                   //Fourth case: syntax:show-active
                {
                    cout << "Active jobs:" << endl;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 1)  //1 is for active
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }     
                break;}
                case 5:                   //Fifth case: syntax:show-pools
                {
                    cout << "Pool & NumOfJobs:" << endl;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 1)  //1 is for active
                            {
                                cout << pool[i]->get_Pid() << "  " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }     
                    
                break;}
                case 6:                   //Sixth case: syntax:show-finished
                {
                    cout << "Finished jobs:" << endl;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 0)  //0 is for finished
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }     
                    
                break;}
                case 7:                   //Seventh case: syntax:suspend <JobID>
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    //cout << "job id is  " << temp_str << endl;
                    int id = atoi(temp_str);
                    cout << "job id is  " << id << endl;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_ID(j) == id)  
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                                pool[i]->suspendJob(j);
                                cout << "Sent suspend signal to JobID " << id << endl;
                            }
                        }  
                    }    
                    delete temp_str; 
                    
                break;}
                case 8:                   //Eighth case: syntax:resume <JobID>
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    //cout << "job id is  " << temp_str << endl;
                    int id = atoi(temp_str);
                    cout << "job id is  " << id << endl;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_ID(j) == id)  
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                                pool[i]->resumeJob(j);
                                cout << "Sent resume signal to JobID " << id << endl;
                            }
                        }  
                    }    
                    delete temp_str; 
                    
                break;}
                case 9:                   //Second case: syntax:shutdown
                {
                    int activejobs = 0, numofjobs = 0;
                    for(int i = 0; i < num_of_pools; i++)
                    {
                        pool[i]->Shutdown(activejobs);
                        numofjobs += pool[i]->get_current_Jobs();
                    }    
                    cout << "Served " << numofjobs << " jobs, " << activejobs << " were still in progress." << endl;
                break;}
                case 10:                   
                {
                    printf("Server OFF.\n");
                    strcpy(end, "exit");
                break;}
                
            }
            //printf("Sending back...\n");
            //printf("Reply to console: ");
           // scanf("%s", buf);
            strcpy(buf, "Hi");
            
            write(coord_to_console,buf,SIZE);
        }

        /* clean buf from any data */
        memset(buf, 0, sizeof(buf));

        //close(client_to_server);
        //close(server_to_client);
    }

    close(console_to_coord);
    close(coord_to_console);

    unlink(myfifo);
    unlink(myfifo2);

    return 0;
}


