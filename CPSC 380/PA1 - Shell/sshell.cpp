/*
	Name: Dennis Fomichev
	Student ID: 2470131
	Email: fomichev@chapman.edu
	Course: CPSC 380-01
	Project: Programming Assignment 1 - Simple Shell Interface

	This C++ file asks for the user to enter commands (with/without arguments) and then 
    executes each command in a separate process, and can also run in background if user
    requests it via an ampersand.
*/

#include <unistd.h> // Needed for fork
#include <sys/wait.h> // Needed for wait
#include <stdlib.h> // Needed for exit
#include <cstring> // Needed for strtok
#include <iostream>
#include <string>

#define MAX_LINE 80

int main(void) {
    // Variables provided by Dr. Springer in the assignment desc
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;

    pid_t pid;
    bool ampersandEntered = false; // if entered, do not wait for child process to terminate (do not run wait())

    while (should_run) {
        std::cout << "osh>";

        std::string userInput;

        getline(std::cin, userInput);

        if (userInput == "exit") { // Don't run if the user wants to exit
            should_run = 0;
        }

        if (userInput[userInput.length() - 1] == '&') { // Check if the last character entered is an &
            ampersandEntered = true;

            userInput.erase(userInput.length() - 1); // Remove the & so that it doesn't screw up token/arg code below
        }

        // Using strok to parse the user input and seperate it into arguments for the character array
        // Run the loop while there is a valid argument and we're below the max possible args
        int num = 0;
        char *token = strtok(userInput.data(), " "); // Need to run .data to convert it to char*, since I'm using C++ string.h
        while (token != nullptr && num < MAX_LINE / 2) {
            args[num] = token;
            num++;
            token = strtok(NULL, " ");
        }

        args[num] = nullptr; // Last argument always needs to be NULL for execvp to work

        pid = fork();

        if (pid < 0) { // Something went wrong
            perror("There was an error with the fork");
        } else if (pid == 0) { // Child process
            int status = execvp(args[0], args);

            if (status == -1) {
                perror("There was an error with execvp");
            }

            should_run = 0; // Exit child process after running command
        } else { // Parent process
            if (!ampersandEntered) { // No ampersand, we wait for process. Otherwise, let run in background 
                wait(nullptr);
            }
        }
    }

    return 0;
}