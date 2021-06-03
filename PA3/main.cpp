#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

using namespace std;
//global
bool background = false;

char** get_arglist(std::string line){
    vector<string> arglist;
    string token2 = "";
    char* token2_to_char;
    bool in_single_quote = false;
    bool in_double_quote = false;
    int size_arglist;
    
    for(char const c : line){
        if (c == '\'' && in_single_quote == false){
            //token2 = token2 + c;
            in_single_quote = true;
        }
        else if (c == '\'' && in_single_quote == true){
            //token2 = token2 + c;
            in_single_quote = false;
        }  
        else if (c == '\"' && in_double_quote == false){
            //token2 = token2 + c;
            in_double_quote = true;
        }
        else if (c == '\"' && in_double_quote == true){
            //token2 = token2 + c;
            in_double_quote = false;
        }
        else if( (c == ' ' ) && (in_single_quote == false && in_double_quote == false) && token2 != ""){
            arglist.push_back(token2);
            
            token2 = "";
        }
        else if(c == '&'){
            background = true;
        }
        else if(c == ' ' && token2 == ""){
            //do nothing
        }
        else{
            token2 = token2 + c;
        }
        
    }
    if(token2 != ""){
        arglist.push_back(token2);
    }
    
    size_arglist = arglist.size();
    char** args = new char*[8];
    for(int i=0;i<arglist.size();++i){
        token2_to_char = const_cast<char*>(arglist[i].c_str());
        args[i] = token2_to_char;
    }
    args[size_arglist+1] = NULL;
    return args;  
}


int main (){
    //based on skeleton code of discussion
    
    int stdin = dup(0);
    int stdout = dup(1); 
    char* directory = nullptr;
    string directory_str;
    vector<int> pids; 
    while (true){ 
        //mostly from stackoverflow and codebind
        char buffer[FILENAME_MAX];
        getcwd(buffer, FILENAME_MAX);
        string working_dir(buffer);
        time_t now = time(0);
        struct tm tstruct;
        tstruct = *localtime(&now);
        strftime(buffer, sizeof(buffer), "%m/%d/%Y %X", &tstruct);
        string datetime(buffer);
        getlogin_r(buffer, FILENAME_MAX);
        string username(buffer);
        dup2(stdout, 1);    
        dup2(stdin, 0);
        //
        string inputline;
        cout << username <<": "<< working_dir <<" "<< datetime << " "<< "$ ";
        getline (cin, inputline);
        //cout << inputline;
        //cout << endl;
        if(inputline == "quit" || inputline == "q"){
            cout << "Bye! End of My Shell" << endl;
            exit(1);
        }
        //parser
        vector<string> parsed_line;
        string token = "";
        bool in_single_quote = false;
        bool in_double_quote = false;
        int index =0;
        for (char const c: inputline){   
            index ++;  
            if (c == '\'' && in_single_quote == false){
                token = token + c;
                in_single_quote = true;
            }
            else if (c == '\'' && in_single_quote == true){
                token = token + c;
                in_single_quote = false;
            }  
            else if (c == '\"' && in_double_quote == false){
                token = token + c;
                in_double_quote = true;
            }
            else if (c == '\"' && in_double_quote == true){
                token = token + c;
                in_double_quote = false;
            }
            else if( (c == '|' || c == '<' || c == '>') && (in_single_quote == false && in_double_quote == false)){
                parsed_line.push_back(token);
                token = c;
                parsed_line.push_back(token);
                token = "";
            }
            //remove the first space of the token
            else if(token.size() == 0 && c == ' '){
                //do nothing
            }
            else{
                token = token + c;
            }
        }
        parsed_line.push_back(token);
        //end of parser
        //testing parsed_line
        /*
        cout << "Size: " << parsed_line.size() << endl;
        for(int i=0;i<parsed_line.size();++i){
            cout << parsed_line[i] << endl;
        }
        //end test
        */
        //executing commands
        
        for(int i=0; i<parsed_line.size(); ++i){
            int fds [2];
            int pid;
            int pid2;
            int status;
            pipe(fds);
            
            char** arglist;
            
            //PIPING
            if(parsed_line[i] == "|" || (i == parsed_line.size()-1 && parsed_line.size()>1 && parsed_line[i-1] == "|")){             
                pid = fork();
                if (pid == 0){ //child
                    if(parsed_line[i] == "|"){
                        dup2(fds[1], 1);
                        close (fds[0]);
                        arglist = get_arglist(parsed_line.at(i-1));
                    }
                    else{
                        arglist = get_arglist(parsed_line.at(i));
                    }
                    execvp(arglist[0], arglist);
                }
                else{
                    waitpid(pid, &status, WUNTRACED);
                    dup2(fds[0], 0);
                    close (fds[1]);
                } 
            }

            //REDIRECT IN
            else if(parsed_line[i] == "<" )  {
                //int fds1[2];
                cout << "<<<";
                if(parsed_line.size() > i+3){
                    cout << "<<<<<<";
                    if(parsed_line[i+2]==">"){
                        if (!fork()){
                            execlp("rm", "rm", "-f", parsed_line[i+3].c_str(), NULL);
                        }
                        else{
                        wait(0);
                        int fd = open(parsed_line[i+3].c_str(), O_CREAT|O_WRONLY|O_TRUNC|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                        dup2(fd, 1);
                        }
                    }
                }
                pid = fork();
                if (pid ==0){
                    int fd = open(parsed_line[i+1].c_str(), O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    if(i < parsed_line.size()-2){
                        pipe(fds);
                        dup2(fds[0],0);
                    }
                    arglist = get_arglist(parsed_line.at(i-1));
                    execvp(arglist[0], arglist);
                }
                else{
                    waitpid(pid, &status, WUNTRACED);
                    dup2(fds[0], 0);
                    close (fds[1]);
                }
            }
            //REDICTION OUT
            else if(parsed_line[i] == ">" ){
                if(i < 3){

                
                if (!fork()){
                    execlp("rm", "rm", "-f", parsed_line[i+1].c_str(), NULL);
                }
                else{
                    wait(0);
                int fd = open(parsed_line[i+1].c_str(), O_CREAT|O_WRONLY|O_TRUNC|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                dup2(fd, 1);
                
                
                pid = fork();
                if (pid == 0){
                    arglist = get_arglist(parsed_line.at(i-1));
                    execvp(arglist[0], arglist);              
                    //waitpid(pid, &status, WUNTRACED);
                    //dup2(fds[0], 0);
                    
                }
                    else{
                        wait(0);
                        close (fd);
                    
                    } 
                }   
                }
            }
            
            //EXECUTING LAST COMMAND
            if (parsed_line.size()==1 && parsed_line[i] != ""){
                
                arglist = get_arglist(parsed_line.at(i));
                string cd = "";
                cd = string(arglist[0]);
                
                string cd2;
                
                if(cd == "cd"){
                    cd2 = string(arglist[1]);
                    if(cd2 == "-"){
                        cout << directory << endl;
                        chdir("--");
                        chdir(directory);
                    }
                    else{
                        char buffer2 [FILENAME_MAX];
                        getcwd(buffer2, FILENAME_MAX);
                        directory = buffer2;
                        chdir(arglist[1]);
                    }
                }
                
                else if(inputline == "jobs"){
                    for(int k = 0; k < pids.size(); ++k){
                        cout << pids[i] << endl;
                    }
                }
                else if (background == false){
                    pid = fork();
                    if (pid == 0){ //child
                        execvp(arglist[0], arglist);
                    }
                    else{
                        wait(0);
                    }
                }
                else if (background == true){
                    pid = fork();
                    if (pid == 0){
                        
                        execvp(arglist[0], arglist);
                    }
                    else{
                        pids.push_back(pid);
                    }
                }
                
                for(int j = 0; j < pids.size(); ++j){
                    int status = 0;
                    status = waitpid(pids.at(i), NULL, WNOHANG);
                        if (status > 0){
                            pids.erase(pids.begin() + i);
                        }
                    
                }
                background = false;
            }           
        }       
    }
    return 0;
};