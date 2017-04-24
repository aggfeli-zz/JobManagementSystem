/* 
 * File:   Pool.cpp
 * Author: angelique
 *
 * Created on April 19, 2017, 2:34 PM
 */

#include <cstdlib>
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
#define SIZE 1024

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) 
{
    cout << "Pool Process" << endl;
    cout << argv[1] << endl;
    int jobs = atoi(argv[1]);
    int currentjobs = atoi(argv[4]);
    char poolin[10], poolout[11];
    
    cout << "current jobs " << currentjobs << endl;
    while(currentjobs < jobs)
    {
        sprintf(poolin, "pool%sin%d", argv[2], currentjobs); 
        sprintf(poolout, "pool%sout%d", argv[2], currentjobs);

        cout << "Pool names done" << endl;
        if (mkfifo(poolin, 0666) == -1)
        {
            printf("fifo in error %s", strerror(errno));
        }

        if (mkfifo(poolout, 0666) == -1)
        {
            printf("fifo out error %s", strerror(errno));
        }
        cout << "Fifo done" << endl;
        
        pid_t kid = fork();


        if (kid == -1) {
            fprintf(stderr, "[%d]: fork: %s\n", getpid(), strerror(errno));
        }
        else if (kid == 0) 
        {
          /* child */
            cout << "Pool is gonna execute the job "  << endl;
            printf ( "Job: Child’s PID: %d\n", getpid());
            printf ( "Job: Parent’s PID: %d\n", getppid()); 
            char str[SIZE];
            int mess_from_coord, mess_to_coord;
            mess_from_coord = open(poolin, O_RDONLY);
            mess_to_coord = open(poolout, O_WRONLY);
            
            
            read(mess_from_coord, str, SIZE);
            
            strcpy(str, "Done!");                          
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
             int mess_from_pool, mess_to_pool;
                        char str[SIZE];
                        mess_to_pool = open(poolin, O_WRONLY);
                        if(mess_to_pool < 0) { perror ("fifo open error mess_to_pool" ); exit (1) ; }
            
                        mess_from_pool = open(poolout, O_RDONLY);
                        if(mess_from_pool < 0) { perror ("fifo open error mess_from_pool" ); exit (1) ; }

                        if(write(mess_to_pool, argv[3], sizeof(argv[3])) < 0){
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
            pid = waitpid(kid, &status, 0);
            //current_jobs--;
            if (pid == -1)
              fprintf(stderr, "%z: waitpid: %s\n", getpid(), strerror(errno));
            printf ( "Parent Coord: Hello I am the parent process\n" ) ;
            printf ( "Parent Coord: Parent’s PID: %d\n", getpid());
            printf ( "Parent Coord: Child’s PID: %d\n", kid);
        }
    }
    
    return 0;
}

