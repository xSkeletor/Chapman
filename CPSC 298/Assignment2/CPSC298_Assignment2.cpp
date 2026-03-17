/*
	Name: Dennis Fomichev
	Student ID: 2470131
	Email: fomichev@chapman.edu
	Course: CPSC 298-06
	Excerise: Programming Assignment 2

	This program is a virtual phone book, where the user is able to navigate a menu
	to add contacts, search for a specific contact, display a list of all stored contacts,
	or quit the application.
*/

#include <iostream>
#include <string>

using namespace std;

// ONLY THESE 2 global variables are accepted!
// Adding more global variables will make your code messy, and you only need these two variables for all your functions, which is why it’s global
string* names = new string[100]; // Global Variable!
unsigned long* phoneNumbers = new unsigned long[100];

// Adds a contact to the names and phone numbers array based on user provided input or modifies an already existing contact's phone number.
bool addContact(string name, unsigned long phoneNumber) {
	// Loop through all existing name entries to see if someone with same name exists, if so, replace their phone number.
	for (int i = 0; i < 100; i++) {
		if (names[i] == name) {
			phoneNumbers[i] = phoneNumber;

			return false;
		}
	}
	
	// We checked to see that this person does not exist already, find the next empty element in array and add their name and phone number.
	for (int i = 0; i < 100; i++) {
		if (names[i] == "") {
			names[i] = name;
			phoneNumbers[i] = phoneNumber;

			break;
		}
	}

	return true;
}

// Loops through the names array to see if the contact exists and if so return their phone number.
unsigned long searchContact(string name) {
	for (int i = 0; i < 100; i++) {
		if (names[i] == name) {
			return phoneNumbers[i];
		}
	}

	return 0;
}

// Display all contacts except any empty entires in the arrays along with their names and phone numbers. 
void displayContacts() {
	// Used to see if any contacts were displayed.
	int numDisplayed = 0;

	for (int i = 0; i < 100; i++) {
		if (names[i] != "") {
			cout << names[i] << "\t" << phoneNumbers[i] << endl;

			numDisplayed++;
		}
	}

	// If none displayed, show a not found message.
	if (numDisplayed == 0) {
		cout << "No contacts found." << endl;
	}
}

// Main menu for the program, asks what the user wants to do and also handles user input to call the appropriate functions implemented above.
int main(int argc, char** argv) {
	const string menu = "------------------------------\n"
		"Please select the number from the following options:\n"
		" 1) Add contact\n 2) Search contact\n"
		" 3) Display all contacts\n 4) Quit\n"
		"------------------------------\n"; // Menu options

	int inputOption;
	do {
		cout << menu << endl; // Printing the menu
		cin >> inputOption; // Getting the user input number
		cin.ignore(); // Ignore trailing input

		if (inputOption == 1) {
			string name;
			unsigned long number;

			cout << "What is the new contact's name?" << endl;

			getline(cin, name);

			cout << "What is their phone number?" << endl;

			cin >> number;

			bool result = addContact(name, number);

			// If true, a new contact was added, if false then an already existing contact's phone number was modified.
			if (result) {
				cout << "Successfully added new contact!" << endl;
			} else {
				cout << "Replaced " <<  name << "'s" << " phone number with " << number << "." << endl;
			}
		} else if (inputOption == 2) {
			string search;

			cout << "What is the name of the contact you're searching for?" << endl;

			getline(cin, search);

			long result = searchContact(search);

			// Check if there is a valid phone number associated with the input the user gave, if there is print it out otherwise show an error message.
			if (result == 0) {
				cout << "No contact found for \"" << search << "\"" << endl;
			} else {
				cout << search << ": " << result << endl;
			}
		} else if (inputOption == 3) {
			displayContacts();
		} else if (inputOption == 4) {
			cout << "Exiting program..." << endl;

			break; // exits the while loop
		} else {
			cout << "Not a valid option, please try again." << endl;
		}
	} while (inputOption != 4);

	// Free up memory from the dynamically stored arrays
	delete [] names;
	delete [] phoneNumbers;

	return 0;
}