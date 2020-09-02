#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFERSIZE 1024

int main(int argc, char const *argv[]) {
  //since first argument is filename itself
  int status; //used for waiting child processes
  char buffer[BUFFERSIZE]; //buffer for stdin reads
  int i;
  int j;
  int k;
  int current_index;
  int numberOfChildren;
  int * pidArray;
  int ** fdArray;
  int ** mrArray;
  int ** reducerPipeArray;
  char idArray1[20];
  char idArray2[20];
  char idArray3[20];


  if(argc>=2)
  {
    numberOfChildren=(int) strtol(argv[1], (char **)NULL, 10);
  }
  pid_t pid; // to ensure that child processes cannot call fork
  pid_t wpid; //waiting pids of child processes
  pid_t npid;
  if(argc == 3)
  {
    fdArray = malloc(sizeof(int*)*numberOfChildren);
    for(i=0;i<numberOfChildren;i++)
    {
      fdArray[i] = malloc(sizeof(int)*2);
    }
    for(i=0;i<numberOfChildren;i++)
    {
      //creates a pipe between child and parent
      if(pipe(fdArray[i])<0)
      {
        //printf("Pipe error!\n");
        exit(-1);
      }

      if(pid != 0 && (pid=fork())<0) // to ensure that child processes cannot call fork
      {
        //printf("Fork error!\n");
      }
      else if (pid>0) //parent process
      {
        close(fdArray[i][0]); // close reading end of parent
      }
      else //child processes
      {
        //redirect child pipe to stdin
        close(fdArray[i][1]); //close writing end of child
        dup2(fdArray[i][0] ,0); //redirect
        //direkt exec i cag??r burada

        sprintf(idArray1, "%d", i);
        execl(argv[2],argv[2],idArray1,(char*) NULL);
        break;
      }
    }
    //when this loop ends parent and child processes are created
    if(pid!=0)
    {
      //read line one by one
      i=0;
      while(fgets(buffer, BUFFERSIZE , stdin)!=NULL)
      {
        // printf("Read on buffer: %s\n",buffer);
        write(fdArray[i][1],buffer,strlen(buffer));
        i++;
        i = i % numberOfChildren;
        // printf("Sending to child %d:\n",i);
      }
      for(i=0;i<numberOfChildren;i++)
      {
        // printf("Closes pipe to child: %d\n",i);
        close(fdArray[i][1]);
      }
      while ((wpid = wait(&status)) > 0);
      //push it to children in a round robin way
      //close when EOF is called
      //wait for them
    }
  }
  else if(argc == 4)
  {
    /*INITIALIZATIONS*/
    fdArray = malloc(sizeof(int*)*numberOfChildren);
    for(i=0;i<numberOfChildren;i++)
    {
      fdArray[i] = malloc(sizeof(int)*2); //fd1
    }
    mrArray = malloc(sizeof(int*)*numberOfChildren);
    for(i=0;i<numberOfChildren;i++)
    {
      mrArray[i] = malloc(sizeof(int)*2); //fd1
    }
    reducerPipeArray = malloc(sizeof(int*)*(numberOfChildren-1));
    for(i=0;i<numberOfChildren-1;i++)
    {
      reducerPipeArray[i] = malloc(sizeof(int)*2); //reducer pipes
    }
    pidArray = (int*) malloc(sizeof(int)*numberOfChildren*2);
        /*END INITIALIZATIONS*/
        /*CREATE ALL PIPES*/
    for(i=0;i<numberOfChildren;i++)
    {
      if(pipe(fdArray[i])>=0)
      {
        //printf("fd pipe is created %d \n",i );
      }
      if(pipe(mrArray[i])>=0) //new pipe between M-R is created
      {
        //printf("mr pipe is created %d \n",i );
      }
      if(i<numberOfChildren-1)
      {
        if(pipe(reducerPipeArray[i])>=0) //new pipe between R-R is created
        {
          //printf("rr pipe is created %d \n",i );
        }
      }
    }
    //printf("Pipe creation ends\n\n");
    /*END CREATING ALL PIPES*/

    for(j=0;j<numberOfChildren*2;j++)
    {
      pid=fork();
      if(pid == 0) //child process
      {
        //printf("Child no %d \n", j);
        if(j<numberOfChildren) //mapper
        {
          //printf("Mapper no : %d \n",j );
          //close all others
          for(k=0;k<numberOfChildren;k++)
          {
            if(j==k) //if it has correct id, close wrong ones
            {
              dup2(fdArray[j][0],0); //redirect input from pipe to stdin
              dup2(mrArray[j][1],1); // close reading end of middle child to MR pipe
            }
              close(fdArray[k][0]); //wont use pipes with other id's
              close(fdArray[k][1]);
              close(mrArray[k][0]);
              close(mrArray[k][1]);

            if(k<numberOfChildren-1)
            {
              close(reducerPipeArray[k][0]);
              close(reducerPipeArray[k][1]);
            }
          }
          sprintf(idArray2, "%d", j);
          execl(argv[2],argv[2],idArray2,(char*) NULL); //TODO change this
        }
        else //reducer part
        {
          current_index = j-numberOfChildren;
          //printf("Reducer no : %d \n",current_index );

          dup2(mrArray[current_index][0],0); //receive input from pipe to stdin

          if(current_index == 0 )
          {
            dup2(reducerPipeArray[current_index][1],1); //push output to pipe below to stdout
          }
          else if (current_index == numberOfChildren-1) //last reducer
          {
            dup2(reducerPipeArray[current_index-1][0],2); //get pipe from previous level stderr
          }
          else // reducers in the middle
          {
            dup2(reducerPipeArray[current_index-1][0],2);
            dup2(reducerPipeArray[current_index][1],1);
          }
          for(k=0;k<numberOfChildren;k++) // since all the dups have been done
          {
            close(fdArray[k][0]);
            close(fdArray[k][1]);
            close(mrArray[k][0]); //MOD 3
            close(mrArray[k][1]); //close writing end of reducer child to MR pipe
            if(k<numberOfChildren-1)
            {
              close(reducerPipeArray[k][0]);
              close(reducerPipeArray[k][1]);
            }
          }
          sprintf(idArray3, "%d", current_index);
          execl(argv[3],argv[3],idArray3,(char*) NULL); // reducer exec
        }
      }
    }
       //parent part pipes are complete

        //printf("Fork is created no %d \n",j );
        for(k=0;k<numberOfChildren;k++)
        {
          close(fdArray[k][0]);  //parent never reads from pipe to mapper
          close(mrArray[k][0]);//parent never uses MR pipe for reading
          close(mrArray[k][1]);//parent never uses MR pipe for writing
          if(k<numberOfChildren-1)
          {
            close(reducerPipeArray[k][0]); //parent never uses pipe between reducers for reading
            close(reducerPipeArray[k][1]); //parent never uses pipe between reducers for writing
          }
        }



    //when this loop ends parent and child processes are created
    //GIVE INPUTS TO CHILDREN AND WAIT FOR THEM
    //printf("Reading lines \n");
    //read line one by one
    i=0;
    while(fgets(buffer, BUFFERSIZE , stdin)!=NULL)
    {
      //printf("Read on buffer: %s %d\n",buffer,i);
      write(fdArray[i][1],buffer,strlen(buffer));
      //printf("Sending to child %d:\n",i);
      i++;
      i = i % numberOfChildren;
    }

    for(i=0;i<numberOfChildren;i++)
    {
      //printf("Closes pipe to child: %d\n",i);
      close(fdArray[i][0]);
      close(fdArray[i][1]);
    }
    while ((wpid = wait(&status)) > 0);

  }
  else
  {
    //printf("Must be 3 or 4 arguments\n" );
    exit(-1);
  }
  return 0;
}
