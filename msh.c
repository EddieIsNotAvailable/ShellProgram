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


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 10
#define MAX_HISTORY_LENGTH 5

char* history[MAX_HISTORY_LENGTH]={};
//int sp = -1;
int historyCount=-1;

void printHistoryWrapped() {
  int mod = historyCount % MAX_HISTORY_LENGTH;
  int i=0, count =0, top;

  for(i=mod; i>=0; i--)
    printf("%d: %s\n",count++,history[i]);

  if(historyCount<MAX_HISTORY_LENGTH-1) top = historyCount;
  else top = MAX_HISTORY_LENGTH-1;

  for(i=top; i>mod; i--)
    printf("%d: %s\n",count++,history[i]);
}

void printHistory() {
  /* printf("History count is %d upon entering\n",historyCount);
  int height=0;
  if(MAX_HISTORY_LENGTH > historyCount)
  {
    printf("(History count) [%d < %d] (MAX hist len)\n",historyCount,MAX_HISTORY_LENGTH);
    height = historyCount;
    printf("So height is %d\n",height);

    for(int i=0; i <= height; i++) {
      printf("%d: %s\n",i,history[i]);
    }
  }
  else {
    //printf("History ct is full (%d), max len is %d\n",historyCount,MAX_HISTORY_LENGTH);
    //printf("printing history wrapped\n\n");
    printHistoryWrapped();
  } */
      printHistoryWrapped();

}

/* void updateHistory(char x[MAX_COMMAND_SIZE]) {
  for(int i=MAX_HISTORY_LENGTH-1; i>=1; i--) {
    history[i] = history[i-1];
  }
  history[0] = x;
}

void push(char x[MAX_COMMAND_SIZE]) {
  if(sp == MAX_HISTORY_LENGTH-1) {
    updateHistory(x);
  } else {
    sp++;
    for(int i = sp; i>=0; i--) {
      history[i+1] = history[i];
    }
    history[0] = x;
  }
} */

void addToHistory(char x[MAX_COMMAND_SIZE])
{
  strtok(x, "\n");
  history[++historyCount % MAX_HISTORY_LENGTH] = strdup(x);
  printf("Added %s to history[%d]\n",x, historyCount % MAX_HISTORY_LENGTH);
}

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
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input

    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

///////////////////////

    //Built in command handling

    if(token[0] == NULL)
    {
      continue;
    }

    if(strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0)
    {
      exit(0);
    }

    //Check first character of input, then 1 offset for number
    if(*command_string == '!')
    {
      int num = atoi((command_string+1));
      printf("num was: %d\n",num);
      //TODO, should run the command in history at the number, or give error for "Command not in history" (at n)
      
      int temp = getFromHistory(num);
      printf("Got %d from history\n",temp);
      
      for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
      {
        token[i] = NULL;
      }

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
        {
          token[token_count] = NULL;
        }
          token_count++;
      }
    }
    else addToHistory(command_string);

    if(strcmp(token[0], "history") == 0)
    {
      printHistoryWrapped();
      continue;
    }

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
        //perror("Command not found."); //Use perror for more descriptive error message
        printf("%s: Command not found.\n",token[0]);
        return 0;
      }
    }
    else 
    {
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
