
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
            currentCommand._inputFile = strdup(tokens[i + 1].value.c_str());
            i++;
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
            chdir(_simpleCommands[0]->_arguments[1]); //cahnge dir to that dir
            clear();
            return;
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
        printf("File could not be opened!");
        return;
       }
       filedescIN = isOpen;
    }
    else
    {
        filedescIN = dup(tempIN);
    }
    for (size_t i = 0; i < _numberOfSimpleCommands; i++)
    {
        dup2(filedescIN, 0);
        close(filedescIN);
        int filedescOUT;
        if( i == _numberOfSimpleCommands - 1)
        {
            if(_outFile)
            {
                if(_append == 1)
                {
                    filedescOUT = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                }
                else
                {
                   filedescOUT = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }
            }
            else
            {
                filedescOUT = dup(tempOUT);
            }
        }
        else
        {
            int filedescPIPE[2];
            pipe(filedescPIPE);
            filedescIN = filedescPIPE[0]; // this is the read part of the pipe
            filedescOUT = filedescPIPE[1]; // this is the write part where it gets the input from the read part
        }
        dup2(filedescOUT, 1);
        close(filedescOUT);
        if(_errFile)
        {
            int filedescERR = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(filedescERR, 2);
            close(filedescERR);
        }
        else if (_out_error)
        {
            dup2(1, 2);
        }
        
        pid = fork();
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
    dup2(tempIN, 0);
    dup2(tempOUT, 1);
    dup2(tempERR, 2);
    close(tempIN);
    close(tempOUT);
    close(tempERR);
    if(_background == 0) // if there is no background process then wait for the child to finish
    {
        waitpid(pid, NULL, 0);
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
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {   
        FILE* logFile = fopen("child.log", "a");
        if (logFile)
        {
            fprintf(logFile, "Child process %d terminated\n", pid);
            fclose(logFile);
        }
    }
}

int main()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, logs);
    Command::_currentCommand.prompt();
    return 0;
}
