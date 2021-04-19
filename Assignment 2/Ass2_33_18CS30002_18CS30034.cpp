/*
Assignment - 2 OS LAB
Implementation of a rudimentary command-line interpreter running on the Linux OS
Group No. : 33
Group Member 1 Name: Aayush Prasad
Group Member 1 Roll No.: 18CS30002
Group Member 2 Name: Rajdeep Das
Group Member 2 Roll No.: 18CS30034
*/
#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

// Trims all spaces to the left of the string
string leftTrim(string s);
// Trims all spaces to the right of the string
string rightTrim(string s);
// Trims all spaces from left and right
string trim(string s);
// Splits the string given delimiter
vector<string> split(string s, char d);
// Splits the command into input and output (for redirection)
vector<string> splitInputOutput(string s);
// Opens files and redirects input with files as arguments
void redirect(string s_in, string s_out);
// Executes the command
void executeCommand(string s);

int main() {
    string s;
    int status = 0;
    while(true) {
        // Flag for runnning in background
        bool background_flag = false;
        // Gets the command as input
        cout <<"command> ";
        getline(cin, s);
        // Trims the command
        s = trim(s);
        // Checks for background run
        if(s.back() == '&') {
            background_flag = true;
            s.back() = ' ';
        }
        // Splits the several commands if piped
        vector<string> s_pipes = split(s, '|');
        // No pipes are present
        if(s_pipes.size() == 1) {
            // Splits the commands and redirects
            vector<string> parsed_s = splitInputOutput(s_pipes[0]);
            // Creates child process
            pid_t pid = fork();
            if(pid == 0) {
                // Redirects input and output
                redirect(parsed_s[1], parsed_s[2]);
                // Executes the command
                executeCommand(parsed_s[0]);
                // Exits the child process
                exit(0);
            }
            // Not running in background
            if(!background_flag) {
                // Waits for status
                wait(&status);
            }
        }
        // Pipes are present
        else {
            // Gets number of piped commands
            int n = s_pipes.size();
            int new_fd[2], old_fd[2];
            for(int i = 0; i < n; i++) {
                // Splits the command and redirects
                vector<string> parsed_s = splitInputOutput(s_pipes[i]);
                // Creates new pipe except for the last command
                if(i != n-1) {
                    pipe(new_fd);
                }
                // Creates child process
                pid_t pid = fork();
                // In the child process
                if(pid == 0) {
                    // Redirects only for the first and the last commands
                    if(!i || i == n-1) {
                        redirect(parsed_s[1], parsed_s[2]);
                    }
                    // Reads from previous command for every command except the first command
                    if(i) {
                        dup2(old_fd[0], 0);
                        close(old_fd[0]);
                        close(old_fd[1]);
                    }
                    // Writes into pipe for every command except the last command
                    if(i != n-1) {
                        close(new_fd[0]);
                        dup2(new_fd[1], 1);
                        close(new_fd[1]);
                    }
                    // Executes the command
                    executeCommand(parsed_s[0]);
                }
                // In the parent process
                // Closes old_fd for every command except the first command
                if(i) {
                    close(old_fd[0]);
                    close(old_fd[1]);
                }
                // Updates old_fd for every command except the last command
                if(i != n-1) {
                    old_fd[0] = new_fd[0];
                    old_fd[1] = new_fd[1];
                }
            }
            // Waits for child processes to return if no background run
            if(!background_flag) {
                while(wait(&status) > 0);
            }
        }
    }
    return 0;
}

// Trims all spaces to the left of the string
string leftTrim(string s) {
    string res;
    for(int i = 0; i < s.size(); i++) {
        if(!isspace(s[i])) {
            // Returns the substring starting from i
            return s.substr(i);
        }
    }
    return res;
}

// Trims all spaces to the right of the string
string rightTrim(string s) {
    string res;
    for(int i = s.size() - 1; i >= 0; i--) {
        if(!isspace(s[i])) {
            // Returns the substring starting from 0, ending at i
            return s.substr(0, i+1);
        }
    }
    return res;
}

// Trims all spaces from left and right
string trim(string s) {
    // First does left trim then right trim
    return rightTrim(leftTrim(s));
}

// Splits the string given delimiter
vector<string> split(string s, char d) {
    vector<string> res;
    // Creates a string stream to use getline later
    stringstream ss(s);
    string temp;
    // Reads from the string stream until delimiter
    while(getline(ss, temp, d)) {
        res.push_back(temp);
    }
    return res;
}

// Splits the command into input and output (for redirection)
vector<string> splitInputOutput(string s) {
    /*
        res[0] = command
        res[1] = input redirection (if any)
        res[2] = output redirection (if any)
    */
    vector<string> res(3);
    vector<string> s_out = split(s, '>');
    // No output redirection
    if(s_out.size() == 1) {
        vector<string> s_in = split(s, '<');
        // No input redirection, no output redirection
        if(s_in.size() == 1) {
            // Trims
            res[0] = trim(s_out[0]);
            return res;
        }
        // Input redirection present, no output redirection
        else {
            // Trims
            res[1] = trim(s_in[1]);
            res[0] = trim(s_in[0]);
            return res;
        }
    }
    vector<string> s_in = split(s, '<');
    // No input redirection, output redirection present
    if(s_in.size() == 1) {
        res[2] = trim(s_out[1]);
        res[0] = trim(s_out[0]);
        return res;
    }
    // Input redirection present, output redirection present

    // Input redirection present as second argument
    // Output redirection present as first argument
    if(split(s_out[0], '<').size() == 1) {
        vector<string> s_out_in = split(s_out[1], '<');
        res[2] = trim(s_out_in[0]);
        res[1] = trim(s_out_in[1]);
        res[0] = trim(s_out[0]);
        return res;
    }
    // Input redirection present as first argument
    // Output redirection present as second argument

    vector<string> s_cmd_in = split(s_out[0], '<');
    res[2] = trim(s_out[1]);
    res[1] = trim(s_cmd_in[1]);
    res[0] = trim(s_cmd_in[0]);
    return res;
}

// Opens files and redirects input with files as arguments
void redirect(string s_in, string s_out) {
    int in_fd, out_fd;
    // Opens input redirecting file
    if(s_in.size()) {
        // Opens in read-only mode
        in_fd = open(s_in.c_str(), O_RDONLY);
        if(in_fd < 0) {
            cout << "Error. Cannot open file: " << s_in << endl;
            exit(EXIT_FAILURE);
        }
        // Redirects input
        if(dup2(in_fd, 0) < 0) {
            cout << "Input redirecting error!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    // Opens output redirecting file
    if(s_out.size()) {
        // Opens in create and truncate mode
        out_fd = open(s_out.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        // Redirects output
        if(dup2(out_fd, 1) < 0) {
            cout << "Output redirecting error!" << endl;
            exit(EXIT_FAILURE);
        }
    }
}

// Executes the command
void executeCommand(string s) {
    // Splits the command and its arguments
    vector<string> arguments;
    for(string temp: split(s, ' ')) {
        if(temp.size()) {
            arguments.push_back(temp);
        }
    }
    // Creates a char* array for the arguments
    char* argv[arguments.size() + 1];
    for(int i = 0; i < arguments.size(); i++) {
        // Converts string to char* for execvp later
        argv[i] = const_cast<char*>(arguments[i].c_str());
    }
    // Terminates with NULL pointer
    argv[arguments.size()] = NULL;
    // Assigns argv to a constant array
    char* const* argv1 = argv;
    // Executes the command
    execvp(arguments[0].c_str(), argv1);
}


