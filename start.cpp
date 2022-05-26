#include <stdio.h>
#include<iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include <time.h>
using namespace std;


string trim(string input) { // to elim spaces before and after a string
    string output;
    for(size_t i = 0; i < input.size(); i++) {
        if(input[i] != ' ') {
            output.push_back(input[i]);
            if(input[i+1] == ' '){
                output.push_back(' ');
            }
        }
    }
    if(output[output.size() - 1] == ' ') {
        output.erase(output.size() - 1);
    }
    return output;
    /*input.erase(remove(input.begin(), input.end(), ' '), input.end());
    return input;*/
}

char** vec_to_char_array(vector<string>& x) {
    char** result = new char*[x.size() + 1];
    for(size_t i = 0; i < x.size(); i++) {
        result[i] = (char*) x [i].c_str();
    }
    result[x.size()] = NULL;
    return result;
}

vector<string> split(string s, char delimiter) {

    string line = s;
    vector<string> vec;
    int h = 0;

    string space = "";

    for(size_t i = 0; i < line.length(); i++) {

        if((line[i] == delimiter) || (i == line.size() - 1)) { // at the end
            string k;
            if(i == line.size() - 1) {
                for(size_t b = h; b < i+1; b++){
                    k.push_back(line[b]);
                }
            }
            else {
                for(size_t b = h; b < i; b++) {
                    k.push_back(line[b]);
                }
            }
            vec.push_back(k);
            h = i+1;
        }
        
    }
    return vec;
} 


bool contains(string input, char delimiter) {
    bool contains = false;
    for(size_t i = 0; i < input.size(); i++) {
        if(input[i] == delimiter) {
            contains = true;
        }
    }
    return contains;
}

int main() {

    vector<int> bgs;
    int input_init = dup(0);
    int output_init = dup(1);

    char cd_buf[1024];
    string curr_dir = getcwd(cd_buf, sizeof(cd_buf));
    chdir("..");
    string prev_dir = getcwd(cd_buf, sizeof(cd_buf));
    //chdir(curr_dir.c_str());

    while (true){ // infinite loop

        dup2(input_init, 0);
        dup2(output_init, 1);

        for(size_t i = 0; i < bgs.size(); i++) { // to reap bg processes
            if(waitpid(bgs[i], 0, WNOHANG) == bgs[i]){
                ;
            }
            else {
                bgs.erase(bgs.begin() + i);
                i--;
            } 
        }

        time_t mytime = time(NULL);
        cout << "My Shell: Mahirah " << ctime(&mytime) << "$ ";
        string inputline;
        getline (cin, inputline);   // get a line from standard input
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }

        // background processes
        bool bg = false;
        if(inputline[inputline.size() - 1] == '&') {
            //cout << "background process found" << endl;
            bg = true;
            inputline = trim(inputline.substr(0, inputline.size() -1)); // remove & symbol
        }

        // piping
        vector<string> pipe_vector = split(inputline, '|');
        //cout << "pipe vector size: " << pipe_vector.size() << endl;

        for(size_t p = 0; p < pipe_vector.size(); p++) {
            int fd[2];
            pipe(fd);
            inputline = trim(pipe_vector[p]);

            int pid = fork ();

            if (pid == 0){ //child process
                int io_index;

                if(contains(inputline, '>')) {
                        // only >
                        // file redirection (output)
                    //int output_red = inputline.find('>');
                    io_index = trim(inputline).find('>');
                    //cout << "here " << endl;
                    //cout << "This text came from a file" << endl;
                    string i_red_command = trim(inputline.substr(0, io_index));
                    string i_red_filename = trim(inputline.substr(io_index+1));
                    inputline = i_red_command;
                    //inputvec[]
                    int i_fd = open(i_red_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC,S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
                    dup2(i_fd, 1); // output redirection
                    close(i_fd);
                }

                if(contains(inputline, '<')) {
                    // only <
                    //int input_red = inputline.find('<');
                    io_index = inputline.find('<');
                    cout << "This text will go to a file" << endl;
                    string o_red_command = trim(inputline.substr(0, io_index));
                    string o_red_filename = trim(inputline.substr(io_index+1));
                    inputline = o_red_command;
                    int o_fd = open(o_red_filename.c_str(), O_RDONLY, S_IWUSR | S_IRUSR);
                    dup2(o_fd, 0); // input redirection
                    //dup2(0, i_fd);
                    close(o_fd);
                }

                if(p < pipe_vector.size() - 1) {
                    dup2(fd[1], 1);
                    close(fd[1]);
                }

                // cd
                if(trim(inputline).find("cd") == 0) {
                    curr_dir = getcwd(cd_buf, sizeof(cd_buf));
                    if(trim(inputline).find("-") == 2){
                        chdir(prev_dir.c_str());
                        prev_dir = curr_dir;
                    }
                    else if(trim(inputline).find("/") > 0) {
                        string new_name = trim(split(inputline, ' ')[1]);
                        chdir(new_name.c_str());
                        prev_dir = curr_dir;
                    }
                }


                vector<string> x = split(inputline, ' ');
                char ** args = vec_to_char_array(x);
                execvp (args[0], args);
            }
            else{
                // only wait for foreground process
                if(!bg) {
                    if(p == pipe_vector.size() - 1) {
                        waitpid (pid, 0, 0);
                    }
                }
                else {
                    bgs.push_back(pid); // to keep track of background processes
                }
                // wait for the child process 
                // we will discuss why waitpid() is preferred over wait()
                dup2(fd[0], 0);
                close(fd[1]);
            }
        }
    }
    return 0;
}