// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 12
#define MAX_HISTORY_LENGTH 15

char* history[MAX_HISTORY_LENGTH]={};
int hisPID[MAX_HISTORY_LENGTH]={};
int historyCount=-1;

void printHistory()
{
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count=0, top;

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=mod+1; i<=top; i++)
    printf("%d: %s\n",count++,history[i]);

  for(i=0; i<=mod; i++)
    printf("%d: %s\n",count++,history[i]);
}

void printPIDs()
{
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count=0, top;

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=mod+1; i<=top; i++)
    printf("%d: %d\n",count++,hisPID[i]);

  for(i=0; i<=mod; i++)
    printf("%d: %d\n",count++,hisPID[i]);
}

void addToHis(char x[MAX_COMMAND_SIZE], int y)
{
  strtok(x, "\n");
  int index = ++historyCount % MAX_HISTORY_LENGTH;
  history[index] = strdup(x);

  //Save PID
  hisPID[index] = y;  
}

//Retrieve the actual index in history from the derived number output to user by printHistory
int getFromHistory(int x)
{
  int size;
  if(historyCount < MAX_HISTORY_LENGTH-1) size = historyCount;
  else size = MAX_HISTORY_LENGTH-1;

  if(x<0 || x>size)
  {
    printf("Command not in history\n");
    return 0;
  }
  
    int mod = historyCount % MAX_HISTORY_LENGTH;
    for(int i=x; i>=0; i--)
    {
      if(mod+1 <= size) mod++;
      else mod = 0;
    }
  return mod;
}

int main()
{
  char* command_string = (char*) malloc(MAX_COMMAND_SIZE);

  while( 1 )
  {
    //Print out the prompt
    printf("msh> ");

    //Wait for command to read
    while(!fgets(command_string, MAX_COMMAND_SIZE, stdin));

    char *token[MAX_NUM_ARGUMENTS];

    for(int i=0; i<MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int token_count = 0;                                 
    char *argument_ptr = NULL;                                               
    char *working_string  = strdup(command_string);
    char *head_ptr = working_string;

    //Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if(strlen(token[token_count]) == 0)
        token[token_count] = NULL;
      
      token_count++;
    }

    ////////Built in command handling

    if(token[0] == NULL)
      continue;

    if(strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0)
      exit(0);

    //Note that command !n not directly added to history,
    //  Will update command_string to the retreived command, and update history with that
    //Check first character of input, 
    if(*command_string == '!')
    {
      //1 offset to get number
      int num = atoi((command_string+1));
      
      //Get position in history array of num
      int temp = getFromHistory(num);

      for(int i=0; i<MAX_NUM_ARGUMENTS; i++)
        token[i] = NULL;

      token_count = 0;                                 
      command_string = strdup(history[temp]);
      working_string = strdup(command_string);
      head_ptr = working_string;

      //Tokenize the input strings with whitespace used as the delimiter
      while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
        if(strlen(token[token_count]) == 0 )
          token[token_count] = NULL;
        
        token_count++;
      }
    }

    if(strcmp(token[0], "history") == 0)
    {
      addToHis(command_string,-1);
      if(token[1] == NULL) {
        printHistory();
        continue;
      }
      else if(strcmp(token[1], "-p")==0)
      {
        printPIDs();
        continue;
      }
    }
    
    if(strcmp(token[0],"cd") == 0 )
    {
      addToHis(command_string,-1);
      chdir(token[1]);
      continue;
    }

    //If command wasn't a built in, attempt to resolve as unix command with exec
    pid_t pid = fork( );

    if(pid == 0)
    {
      int ret = execvp(token[0], &token[0]);
    
      if(ret == -1)
      {
        printf("%s: Command not found.\n",token[0]);
        return 0;
      }
    }
    else
    {
      //savedPID = pid;
      int status;
      wait(&status);
    }
    addToHis(command_string,pid);

    // Cleanup allocated memory
    for(int i=0; i<MAX_NUM_ARGUMENTS; i++)
      if(token[i] != NULL)
        free(token[i]);
    
    free(head_ptr);
  }

  free(command_string);
  return 0;
}
