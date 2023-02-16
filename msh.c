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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>


#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 10
#define MAX_HISTORY_LENGTH 15

char* history[MAX_HISTORY_LENGTH]={};
int hisPID[MAX_HISTORY_LENGTH]={};
int savedPID=0;
int historyCount=-1;

void printHistory()
{
  //printf("Entering printHistory\n");
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count=0, top;
  for(i=mod; i>=0; i--)
    printf("%d: %s\n",count++,history[i]);

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=top; i>mod; i--)
    printf("%d: %s\n",count++,history[i]);
}

void printPIDs()
{
  //printf("entering printPID\n");
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count=0, top;
  for(i=mod; i>=0; i--)
    printf("%d: %d\n",count++,hisPID[i]);

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=top; i>mod; i--)
    printf("%d: %d\n",count++,hisPID[i]); 
}

//For outputting live pids. (Not used currently, also incomplete for that purpose)
/* void printHistoryPID() {
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count=0, top;

  int historyPID[MAX_HISTORY_LENGTH]={};
  char* commands[MAX_HISTORY_LENGTH]={};
  //printf("%d: %s\n",count++,history[i]);

  //Get string from history index, strtok to get command of it,
  // and exec the command name to get PID

  char *temp = NULL;
  for(i=mod; i>=0; i--) {
    temp = strtok(history[i]," ");
    printf("Got %s from history, at %d\n",temp,count);
    commands[count++] = temp;
  }

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=top; i>mod; i--) {
    temp = strtok(history[i]," ");
    printf("Got %s from history, at %d\n",temp,count);
    commands[count++] = temp;
  }

  pid_t pid = fork();

  for(int i=0; i<=count; i++)
  {
    if( pid == 0 )
    {
      temp = commands[i];
      int ret = execvp("pgrep",&temp);
      printf("Ret was %d for %s\n",ret, temp);
      historyPID[i] = ret;
    }
    else 
    {
      int status;
      wait( & status );
    }
  }

  for(i=0; i<=count; i++) {
    printf("%d: %d\n",i,historyPID[i]);
  }

} */

void savePID()
{
  hisPID[historyCount-1 % MAX_HISTORY_LENGTH] = savedPID;
  savedPID=0;
}

//Add new commands entered to history[]
void addToHistory(char x[MAX_COMMAND_SIZE])
{
  strtok(x, "\n");
  history[++historyCount % MAX_HISTORY_LENGTH] = strdup(x);
  //printf("Added %s to history at %d\n", x, historyCount%MAX_HISTORY_LENGTH);
  savePID();
}

//Retrieve the actual index in history from the derived number output to user by printHistory
int getFromHistory(int x)
{
  int size;
  if(historyCount > MAX_HISTORY_LENGTH-1) size = historyCount;
  else size = MAX_HISTORY_LENGTH-1;

  if(x<0 || x>size)
  {
    printf("Command not in history\n");
    return 0;
  }
  else
  {
    int mod = historyCount % MAX_HISTORY_LENGTH;
    for(int i=x; i>0; i--)
    {
      if(mod-1 >= 0) mod--;
      else mod = MAX_HISTORY_LENGTH-1;
    }
    return mod;
  }
}

int main()
{
  char* command_string = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the prompt
    printf ("msh> ");

    //Wait for command to read
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int token_count = 0;                                 
    char *argument_ptr = NULL;                                               
    char *working_string  = strdup( command_string );
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
        token[token_count] = NULL;
      
      token_count++;
    }


    //Built in command handling

    if(token[0] == NULL)
      continue;

    if(strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0)
      exit(0);

    //Check first character of input, then 1 offset for number
    if(*command_string == '!')
    {
      int num = atoi((command_string+1));
      
      //Get position in history array of num
      int temp = getFromHistory(num);
      
      for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
        token[i] = NULL;

      token_count = 0;                                 
      command_string = history[temp];
      working_string = strdup( command_string );
      head_ptr = working_string;

      // Tokenize the input strings with whitespace used as the delimiter
      while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
          token[token_count] = NULL;
        
        token_count++;
      }
    }
    else if(strcmp(token[0], "history") == 0)
    {
      //printf("Command was history\n");
      //printf("token[1] == %s\n",token[1]);
      if(token[1] == NULL) {
        printHistory();
        continue;
      }
      else if(strcmp(token[1], "-p")==0)
      {
        //printf("Command was history -p\n");
        printPIDs();
        continue;
      }
    } //Don't save history call to history[]
    else addToHistory(command_string);

    if(strcmp(token[0],"cd") == 0 )
    {
      chdir(token[1]);
      continue;
    }

    //If command wasn't a built in, attempt to resolve as unix command with exec
    pid_t pid = fork( );

    if( pid == 0 )
    {
      int ret = execvp( token[0], &token[0] );
    
      if( ret == -1 )
      {
        printf("%s: Command not found.\n",token[0]);
        return 0;
      }
    }
    else
    {
      savedPID = pid;
      //printf("Saved PID %d for %s\n",pid, token[0]);
      int status;
      wait( & status );
    }

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
      if( token[i] != NULL )
        free( token[i] );
    
    free( head_ptr );
  }

  free( command_string );

  return 0;
}
