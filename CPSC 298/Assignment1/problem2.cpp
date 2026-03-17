/*
	Name: Dennis Fomichev
	Student ID: 2470131
	Email: fomichev@chapman.edu
	Course: CPSC 298-06
	Excerise: Problem 2

	This program takes input from user and then modifies the first character of the string to make it seem like it's corrupted.
*/

#include <iostream>
#include <string>

using namespace std;

// The main method, contains all of the code for the program where input is taken and modified and outputted.
int main(int argc, char** argv)
{
	string input;

	cout << "Input a sentence:" << endl;

	getline(cin, input); // Get the full string the user inputs with whitespace

	if (input.length() == 0) { // Check if it's empty first, otherwise corrupt it
		cout << "There was no input data to corrupt.";
	} else {
		input[0] = '?';

		cout << "You inputted: \"" << input << "\"" << endl << "Successfully read " << input.length() - 1 <<  " out of " << input.length() << " characters.";
	}

	return 0;
}