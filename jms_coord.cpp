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

/*./jms_coord -l /home/angelique/NetBeansProjects/CppApplication_17/ -n 2 -w out -r in &
 * ./jms_console -w in -r out -o temp.txt
 */

int num_of_pools = -1;

int main(int argc, char** argv) 
{    
    int i, jobs_pool = 0, fd, len;
    char buf[SIZE], *token, *path, *jms_in, *jms_out, poolin[100], poolout[100], message[SIZE]; 
    
    
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
        
    
    int counter = 0; //Count how many jobs has been submitted 
    char end[5];
    Pool **pool = new Pool*[100];
    
    
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
            //cout << "token = "<< token << endl;
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
                    int position = 0, flag = 0;       
                    counter++;


                    for (int i = 0; i <= num_of_pools; i++) //Find which pool has an empty space for a new job
                    {                        
                        if (pool[i]->get_current_Jobs() < jobs_pool)
                        {  position = i;break;}                      
                    }
                    if(position == 0) position = num_of_pools + 1;
                    
                    if (pool[position] == NULL) //If pool doesn't exist, create a new one.
                    {
                        pool[position] = new Pool(jobs_pool);
                        flag = 1;
                        num_of_pools++;
                    }

                    pool[position]->IncreaseJobs(counter);
                    sprintf(poolin, "%spool%din%d", path, position, pool[position]->get_current_Jobs()); 
                    sprintf(poolout, "%spool%dout%d", path, position, pool[position]->get_current_Jobs());
                    if (mkfifo(poolin, 0666) == -1)
                    {
                        printf("fifo in error %s", strerror(errno));
                    }

                    if (mkfifo(poolout, 0666) == -1)
                    {
                        printf("fifo out error %s", strerror(errno));
                    }
                    cout << "Fifo done" << endl;
                    
                    
                    pid_t poolprocess = fork();

                    if (poolprocess == -1) {
                        fprintf(stderr, "[%d]: fork: %s\n", getpid(), strerror(errno));
                    }
                    else if (poolprocess == 0) 
                    {
                        /* child */
                        //cout << "Pool process in coord" << endl;
                        
                        char str[SIZE];
                        int mess_from_coord, mess_to_coord;
                        mess_from_coord = open(poolin, O_RDONLY);
                        mess_to_coord = open(poolout, O_WRONLY);

                        read(mess_from_coord, str, SIZE);
                        pool[position]->Job_submit(temp_str, position, flag, path);
                        strcpy(str, "Pool job finished!");                          
                        write(mess_to_coord,str,SIZE);


                        close(mess_from_coord);
                        close(mess_to_coord);

                        unlink(poolin);
                        unlink(poolout);
                       
                        //printf ( "Child Pool: Child’s PID: %d\n", getpid());
                        //printf ( "Child Pool: Parent’s PID: %d\n", getppid());    
                        exit(EXIT_SUCCESS);
                    }
                    else {
                      /* parent */
                        //cout << "Coord Process" << endl;
                        pid_t pid;
                        int status;
                        
                        int mess_from_pool, mess_to_pool;
                        char str[SIZE];
                        mess_to_pool = open(poolin, O_WRONLY);
                        if(mess_to_pool < 0) { perror ("fifo open error mess_to_pool" ); exit (1) ; }

                        mess_from_pool = open(poolout, O_RDONLY);
                        if(mess_from_pool < 0) { perror ("fifo open error mess_from_pool" ); exit (1) ; }

                        if(write(mess_to_pool, temp_str, sizeof(temp_str)) < 0){
                            perror("Write:");//print error
                            exit(-1);
                        }
                        if(read(mess_from_pool,str,sizeof(str)) < 0){
                            perror("Read:"); //error check
                            exit(-1);
                        }
                        printf("\n...received from pool: %s\n\n\n",str);

                        close(mess_to_pool);
                        close(mess_from_pool);
        
                        pid = waitpid(poolprocess, &status, 0);
                        //current_jobs--;
                        if (pid == -1)
                          fprintf(stderr, "%d: waitpid: %s\n", getpid(), strerror(errno));
                        //printf ( "Parent Pool: Hello I am the parent process\n" ) ;
                        //printf ( "Parent Pool: Parent’s PID: %d\n", getpid());
                        //printf ( "Parent Pool: Child’s PID: %d\n", poolprocess);

                    }
                    delete temp_str; 
                    sprintf(message, "JobID: %d,PID: %d", counter, pool[position]->get_JobPid(pool[position]->get_current_Jobs())); 
                break;}
                case 2:                   //Second case: syntax:status <JobID> 
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);                   
                    int id = atoi(temp_str);
                    int flag = 0, i, j;

                    for(i = 0; i <= num_of_pools; i++)
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
                    string status;
                    if(pool[i]->get_Status(j) == 0) sprintf(message, "JobID: %d,Status: Finished", id); 
                    else if (pool[i]->get_Status(j) == 0)sprintf(message, "JobID: %d,Status: Active", id);  
                    else sprintf(message, "JobID: %d,Status: Active", id);
                    //cout << "JobID " << id << "  Status: " << status << endl;
                    delete temp_str; 
                    
                break;}
                case 3:                   //Third case: syntax:status-all [time-duration]
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    
                    for(int i = 0; i <= num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            string status;
                            if(pool[i]->get_Status(j) == 0) status = "Finished";
                            else if (pool[i]->get_Status(j) == 0) status = "Active";
                            else status = "Suspended";
                            cout << "JobID " << pool[i]->get_ID(j) << "  Status: " << status << endl;
                        }
                        
                    }                   
                    delete temp_str; 
                    //sprintf(message, "JobID: %d,Status: %s", id, status);
                break;}
                case 4:                   //Fourth case: syntax:show-active
                {
                    cout << "Active jobs:" << endl;
                    for(int i = 0; i <= num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 1)  //1 is for active
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }  
                    sprintf(message, "JobID %d", pool[num_of_pools]->get_ID(jobs_pool-1));
                break;}
                case 5:                   //Fifth case: syntax:show-pools
                {
                    cout << "Pool & NumOfJobs:" << endl;
                    for(int i = 0; i <= num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 1)  //1 is for active
                            {
                                cout << pool[i]->get_Pid() << "  " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }     
                    strcpy(message, "DONE!");
                break;}
                case 6:                   //Sixth case: syntax:show-finished
                {
                    cout << "Finished jobs:" << endl;
                    for(int i = 0; i <= num_of_pools; i++)
                    {
                        for(int j = 0; j < jobs_pool; j++)
                        {
                            if (pool[i]->get_Status(j) == 0)  //0 is for finished
                            {
                                cout << "JobID " << pool[i]->get_ID(j) << endl;
                            }
                        }  
                    }     
                    strcpy(message, "DONE!");
                break;}
                case 7:                   //Seventh case: syntax:suspend <JobID>
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);                    
                    int id = atoi(temp_str);

                    for(int i = 0; i <= num_of_pools; i++)
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
                    strcpy(message, "DONE!");
                break;}
                case 8:                   //Eighth case: syntax:resume <JobID>
                {
                    token = strtok(NULL, "");
                    //cout << "token = "<< token << endl;
                    char *temp_str = new char[strlen(token) + 1];
                    strcpy(temp_str, token);
                    int id = atoi(temp_str);

                    for(int i = 0; i <= num_of_pools; i++)
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
                    strcpy(message, "DONE!");
                break;}
                case 9:                   //Second case: syntax:shutdown
                {
                    int activejobs = 0, numofjobs = 0;
                    for(int i = 0; i <= num_of_pools; i++)
                    {   
                        //pool[i]->Shutdown(activejobs);
                        numofjobs += pool[i]->get_current_Jobs();
                    }    
                    sprintf(message, "Served %d jobs, %d were still in progress.", numofjobs, activejobs);
                    //cout << "Served " << numofjobs << " jobs, " << activejobs << " were still in progress." << endl;
                break;}
                case 10:                   
                {
                    printf("Server OFF.\n");
                    strcpy(end, "exit");
                break;}
                
            }           
            if (strcmp(buf, "shutdown") == 0) strcpy(buf, "end");
            else strcpy(buf, message);
           
            
            write(coord_to_console,buf,SIZE);
        }

        /* clean buf from any data */
        memset(buf, 0, sizeof(buf));

    }   
    close(console_to_coord);
    close(coord_to_console);

    unlink(myfifo);
    unlink(myfifo2);

    
    for (i = 0; i <= num_of_pools; i++) delete pool[i];
    return 0;
}


