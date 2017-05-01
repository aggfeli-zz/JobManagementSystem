/* 
 * File:   jms_console.cpp
 * Author: angelique
 *
 * Created on April 11, 2017, 12:22 PM
 */

#include <cstdlib>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
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
    int i, jobs_pool = 0, fd, nwrite;
    char buf[100],  *jms_in, *jms_out, *operations_file; 
    
    for (i = 1; i < 6; i += 2)
    {
        if(strcmp(argv[i], "-o") == 0)
        {
            operations_file = new char[strlen(argv[i+1]) + 1];
            strcpy(operations_file, argv[i+1]);
            //cout << operations_file << endl;
        }
        else if (strcmp(argv[i], "-w") == 0) 
        {
            jms_in = new char[strlen(argv[i+1]) + 1];
            strcpy(jms_in, argv[i+1]);
            //cout << jms_in << endl;
        }
        else  
        {
            jms_out = new char[strlen(argv[i+1]) + 1];
            strcpy(jms_out, argv[i+1]);
            //std::cout << jms_out << endl;
        }
    }
     
    /* Open the FIFOs. We assume server has
    already created them. */
    char *myfifo = new char[strlen(jms_in) + 1];
    strcpy(myfifo, jms_in);
    char *myfifo2 = new char[strlen(jms_out) + 1];
    strcpy(myfifo2, jms_out);
    
    int coord_to_console, console_to_coord;

    char str[SIZE];
    FILE *file;
    int flag = 0, numoflines;
    file = fopen(operations_file,"r");   
    if (file == NULL) cout << "Unable to open  operations file" << endl; 
    else                    //Count how many commands we have
    {
        flag = 1;
        numoflines = 0;
        while ( fgets(buf, sizeof(buf), file) != NULL ) numoflines++;
        rewind (file);
    }
    
    while(strcmp(str, "exit") != 0)
    {
        if (flag == 0) 
        {
            printf("Input message to coord: ");        
            cin.getline(str,sizeof(str)); 
        }
        else
        {
            fgets(str, sizeof(str), file); 
            numoflines--;            
            if (numoflines == 0) {flag = 0; fclose(file);}      
        }

        /* write str to the FIFO */
        console_to_coord = open(myfifo, O_WRONLY);
        if(console_to_coord < 0) { perror ("fifo open error console_to_coord" ); exit (1) ; }
        coord_to_console = open(myfifo2, O_RDONLY);
        if(coord_to_console < 0) { perror ("fifo open error coord_to_console" ); exit (1) ; }

        if(write(console_to_coord, str, sizeof(str)) < 0){
            perror("Write:");//print error
            exit(-1);
        }
        if(read(coord_to_console,str,sizeof(str)) < 0){
            perror("Read:"); //error check
            exit(-1);
        }
        printf("\n...received from the coord: %s\n\n\n",str);

        close(console_to_coord);
        close(coord_to_console);
    }
   /* remove the FIFO */
    
    


    return 0;
}

