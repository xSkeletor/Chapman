/*
	Name: Dennis Fomichev
	Student ID: 2470131
	Email: fomichev@chapman.edu
	Course: CPSC 298-06
	Excerise: Problem 1

	This program takes input from user for converting a double from ounces into metric tons.
*/

#include <iostream>

using namespace std;

// The main method, contains all of the code for the program where input is taken and modified and outputted.
int main(int argc, char** argv)
{
	double input;

	cout << "Input the weight of a package of breakfast cereal in ounces:" << endl;

	cin >> input;

	cout << "Your " << input << " oz cereal box is " << input / 35273.92 << " metric tons.";

	return 0;
}