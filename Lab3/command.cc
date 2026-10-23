
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == TOKEN_COMMAND)
        {
            if(tokens[i].value.find("*") || tokens[i].value.find("?"))
            {
                glob_t result;
                glob(tokens[i].value.c_str(), GLOB_NOCHECK | GLOB_TILDE, NULL, &result); // the first argument is the actual string containing the wildcard, the 2nd one is for the flags (NOCHECK is so that if no files match the pattern the function returns the literal pattern itself) (TILDE tells the function to expand ~ to the HOME dir), the 3rd is because we want to ignore read errors
                for (size_t j = 0; j < result.gl_pathc; j++) //gl_path is count of paths matched by the pattern
                {
                    currentSimpleCommand->insertArgument(strdup(result.gl_pathv[j])); // gl_pathv contains a list of matched patterns
                }
                globfree(&result);
            }
            else
            {
                currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str())); //c_str takes c++ string and returns normal C char* strdup is used to allocate memory to an existing string and returns char * which is needed by SimpleCommand
            }
        }
        else if (tokens[i].type == TOKEN_ARGUMENT)
        {
            if(tokens[i].value.find("*") || tokens[i].value.find("?"))
            {
                glob_t result;
                glob(tokens[i].value.c_str(), GLOB_NOCHECK | GLOB_TILDE, NULL, &result);
                for (size_t j = 0; j < result.gl_pathc; j++)
                {
                    currentSimpleCommand->insertArgument(strdup(result.gl_pathv[j]));
                }
                globfree(&result);
            }
            else
            {
                currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
            }
        }
        else if (tokens[i].type == TOKEN_PIPE)
        {
            currentCommand.insertSimpleCommand(currentSimpleCommand); 
            currentSimpleCommand = new SimpleCommand(); 
        }
        else if (tokens[i].type == TOKEN_INPUT)
        {
            currentCommand._inputFile = strdup(tokens[i + 1].value.c_str()); //assing the input file to the  next token since its the input file name
            i++; // increment i because we already handeled the next token above
        }
        else if (tokens[i].type == TOKEN_REDIRECT)
        {
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            i++;
        }
        else if (tokens[i].type == TOKEN_APPEND)
        {
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            currentCommand._append = 1;
            i++;
        }
        else if (tokens[i].type == TOKEN_ERROR)
        {
            currentCommand._errFile = strdup(tokens[i + 1].value.c_str());
            i++;
        }
        else if (tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        {
            currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
            currentCommand._out_error = 1;
            i++;
        }
        else if (tokens[i].type == TOKEN_BACKGROUND)
        {
            currentCommand._background = 1;
        }
        else
        {
            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentCommand.execute();
            break;
        }
    }
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;  // I added these two lines because if i type a wrong command followed by | grep for example, it doesnt show that the command is invalid
    _append = 0;     // this is because without initializing _out_error to 0, it gets a random garbage value which then makes it true 
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    
    if(_numberOfSimpleCommands == 0 || _simpleCommands[0]->_numberOfArguments == 0)
    {
        clear();
        return; // empty input so return
    }
    if(strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0)
    {
        exit(0);
    }
    if(strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0)
    {
        if(_simpleCommands[0]->_arguments[1]) //if there is a dir after cd
        {
            int isFound = chdir(_simpleCommands[0]->_arguments[1]); //change dir to that dir
            if(isFound == -1)
            {
                perror("dir not found!");
            }
            else
            {
                clear();
                return;
            }
        }
        else
        {
            chdir(getenv("HOME")); //if the command is cd only then go to home dir
            clear();
            return;
        }
    }
    int tempIN, tempOUT, tempERR;
    pid_t pid;
    tempIN = dup(0); // saves the stdin so that when I can return to it again
    tempOUT = dup(1); // saves the stdout .....
    tempERR = dup(2); // saves the stderr ......
    int filedescIN;
    if(_inputFile)
    {
       int isOpen = open(_inputFile, O_RDONLY);
       if(isOpen == -1)
       {
        printf("File could not be opened!\n");
        return;
       }
       filedescIN = isOpen;//change the working input file descriptor to the one we just opened
    }
    else
    {
        filedescIN = dup(tempIN); //set the working input file descriptor to the default if there is no _inputFile
    }
    for (size_t i = 0; i < _numberOfSimpleCommands; i++)
    {
        dup2(filedescIN, 0); // redirect default stdin to read from the current working input source
        close(filedescIN);
        int filedescOUT;
        if( i == _numberOfSimpleCommands - 1)
        {
            if(_outFile)
            {
                if(_append == 1)
                {
                    filedescOUT = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666); // if append is true, open the file and assign its value to the working OUT file descriptor
                }
                else
                {
                   filedescOUT = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }
            }
            else
            {
                filedescOUT = dup(tempOUT); // if no output file then return the default value to the working OUT file desc
            }
        }
        else
        {
            int filedescPIPE[2]; // pipe needs an array with 2 values, the first element is for the read part of the pipe and the 2nd element is for the write part
            pipe(filedescPIPE);
            filedescIN = filedescPIPE[0]; // this is the read part of the pipe
            filedescOUT = filedescPIPE[1]; // this is the write part where it gets the input from the read part
        }
        dup2(filedescOUT, 1); // redirect default stdout to read from the current working output source
        close(filedescOUT);
        if(_errFile)
        {
            int filedescERR = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(filedescERR, 2); // redirect default stderr to read from the current working output source
            close(filedescERR);
        }
        else if (_out_error)
        {
            dup2(1, 2); // redirect default stderr to read from default stdout
        }
        
        pid = fork(); //create a child process
        if (pid == 0)
        {
            int dummyerr = execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments); // pass the first argument since it contains the command itself for ex: "ls", and then pass the full arguments array
            if(dummyerr)
            {
                perror("Command not found!");
                exit(1);
            }
        }
                
    }
    dup2(tempIN, 0); //return all stdin,out,err to their default values
    dup2(tempOUT, 1);
    dup2(tempERR, 2);
    close(tempIN);
    close(tempOUT);
    close(tempERR);
    if(_background == 0) // if there is no background process then wait for the child to finish
    {
        waitpid(pid, NULL, 0); // pid waits for that specific child to die, NULL ignores the child exit status, and 0 is a flag that tells the OS to completely freeze the parent till the child is done
    }
    clear();
}

void Command::prompt()
{
    std::string input;
    while (true)
    {
        printf("myshell>"); // I had to edit this and put it inside the loop so that i dont have to call prompt() each time which causes the stack to explode because of the multiple loops created on top of each other and it crashes the shell
        fflush(stdout);
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

void logs(int num)
{
    pid_t pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0) //-1 matches any process, WNOHANG doesnt freeze the parent process and > 0 is to ensure there is a child that finished its process
    {   
        FILE* logFile = fopen("child.log", "a"); //open the file in append mode
        if (logFile)
        {
            fprintf(logFile, "Child process %d terminated\n", pid);
            fclose(logFile);
        }
    }
}

int main()
{
    signal(SIGINT, SIG_IGN); //tells the shell to ignore the Ctrl+C 
    signal(SIGCHLD, logs); // whenever a child is done with a process this tells the OS to go execute the logs function
    Command::_currentCommand.prompt();
    return 0;
}
