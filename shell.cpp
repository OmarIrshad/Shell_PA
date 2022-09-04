#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

//My added includes:
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;
std::vector<pid_t> PIDS;
//THis function can be used to convert our vector of arugmentsi into a charecter array
char** vec_to_char_array(vector<string>& parts)
{
    char** result = new char* [parts.size()];
    for (long unsigned int i = 0; i < parts.size(); i++)
    {
        result[i] = (char*) parts [i].c_str();
    }
    result[parts.size()] = NULL;
    return result;
}

void check_background_processes()
{
    int status;
    for (size_t i = 0; i < PIDS.size(); i++)
    {
        if ((status = waitpid(PIDS[i], 0, WNOHANG)) > 0 )
        {
            cout << "PID stopped: " << *(PIDS.begin() + i) << endl;
            PIDS.erase(PIDS.begin() + i);
            i--;
        }
    }
}


int main () {
    //TODO: create copies of stdin and stidout using dup
   char prev_directory[256];
    for (;;) {
        //TODO iteration over vector of bakground pid (vector also declared outside of loop)
        //waitpid() - using flag for non-blocking
        //TODO data/time with TODO
        //TODO username with getlogin()
        //TODO curdir with getcwd()

        //cout << getcwd() << endl;
        // char* getcwd(char* _buf, size_t size);
        check_background_processes();
        time_t timer;
        time(&timer);
        string ting = ctime(&timer);
        ting.pop_back();
        char buf[256];
        getcwd(buf,256);


        cout << GREEN << ting << getenv("USER") << buf <<":" << YELLOW << NC << " ";
        

        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        //TODO CHDIR ()
        //if dir(cd <dir>) is "-", then go to previous directory 
        //variable storing previous wokring directory (it needs to be declared outside of loop) 
       

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        
        if (tknr.commands.at(0)->args[0] == "cd") // Change directories // IF "cd" was passed in
        {
            if (tknr.commands.at(0)->args[1] == "-") //IF "cd -" was passedi n
            {
               //cout <<"Previous: " <<prev_directory << endl;
               chdir(prev_directory);

            }
            getcwd(prev_directory,256); //stores the current directory as previous
            chdir(tknr.commands.at(0)->args[1].c_str()); //Change the directory to the previos
           
            continue;
        }
        
        // if(tknr.commands.size() < 2)
        // {
        //     process_command(tknr); //process a single command (with IO redirection), background process, and change directory
        // }
        // else
        // {
        //     process_pipe(tknr); //process piped commands with support for IO redirection
        // }


        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }


            //TODO: for piping 
            //for cmd : commands 
            //      call pipe() to make pipe
            //      fork() - in child, redirect stdout, in par, redirect stdin
            //      ^ is basically already written, needs to be used properly
            // add checks for first/last command
        int fds[2];
        int term_in = dup(0);
        int term_out = dup(1);

        for(size_t i = 0; i < tknr.commands.size(); i++) /////
        {
            
            pipe(fds);
            // fork to create child
            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            //TODO: add check for background process- add pid to vector background and don't waitpid() in parent

            //Command *cmd = tknr.commands.back();
            char **args = new char *[tknr.commands.at(i)->args.size() + 1];
            for (__uint64_t j = 0; j < tknr.commands.at(i)->args.size(); j++)
            {
                args[j] = (char *)tknr.commands.at(i)->args.at(j).c_str();
            }
            args[tknr.commands.at(i)->args.size()] = nullptr;

            
            if (pid == 0) {  // if child, exec to run command
                // I/O Redirection 
                if(i < tknr.commands.size() - 1)
                {
                    dup2(fds[1], 1); //fixes ls and grep ONLY do it at the end
                }      
                
                //Input 
                if (tknr.commands.at(i)->hasOutput())
                {
                    //open the output file
                    int fd = open(tknr.commands.at(i)->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if ( fd == -1)
                    {
                        perror("open");
                        exit(1);
                    }
                    //Redirect output
                    if (dup2(fd, STDOUT_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }
                if (tknr.commands.at(i)->hasInput()) 
                {
                    int fd = open(tknr.commands.at(i)->in_file.c_str(), O_RDONLY);
                    if (fd == -1)
                    {
                        perror("open");
                        exit(1);
                    }
                    //Redirect Output
                    if (dup2(fd,STDIN_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }     

                     
                    // run single commands with no arguments


                //TODO: implemetn multiple argument-itereage over args of current command to make char* array 

                //char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};


                //TODO if current command is redirected, open file and dup2 std(in/out) that's being redirected 
                //implement it safely for both at same time

                if (execvp(args[0], args) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
               
                

                close(fds[0]); 
                delete[] args;


            }
            else {  // if parent, wait for child to finish
                dup2(fds[0], 0);
                close(fds[1]);
                int status = 0;
                if(i == tknr.commands.size() - 1 && !(tknr.commands.at(i)->isBackground()))
                {
                    waitpid(pid, nullptr, 0);
                }
                
                
                if (status > 1) {  // exit if child didn't exec properly
                    exit(status);
                }
                if(tknr.commands.at(i)->isBackground())
                {
                    PIDS.push_back(pid);
                }
                //Pipe work

            }
        }
            dup2(term_in, 0);
            dup2(term_out,1);
            ////////////// Work
            
        // TODO: restore our stdin/stdout (variables would be outside the loop)
    }
}



