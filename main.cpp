#include <iostream>
using namespace std;


struct Assignment {
    string course;
    string name;
    string dueDate;
    bool completed;
};


// Assignment storage
Assignment assignments[100];
int assignmentCount = 0;


//parameters
void displayMenu();
void addAssignment();
void viewAssignments();
void markCompleted();
void deleteAssignment();
//parameters

int main() {

     int choice;

    do {
        displayMenu();
        cin >> choice;

        
        switch (choice) {
            case 1:
            addAssignment();
            break;
            
            case 2:
            viewAssignments();
            break;
            
            case 3:
            markCompleted();
            break;
            
            case 4:
            deleteAssignment();
            break;
            
            case 5:
            cout << "Goodbye!" << endl;
            break;
            
            default:
            cout << "Invalid Output, try again." << endl;
            
        }
    } while (choice != 5);
        
        
        return 0;
}



void addAssignment(Assignment assignments[], int& assignmentCount) {

    if (assignmentCount >= 100) {
        cout << "Assignment storage is full!" << endl;
        return;
    }

    cin.ignore(1000, '\n');

    cout << "Enter course name: ";
    getline(cin, assignments[assignmentCount].course);

    cout << "Enter assignment name: ";
    getline(cin, assignments[assignmentCount].name);

    cout << "Enter due date (YYYY-MM-DD): ";
    getline(cin, assignments[assignmentCount].dueDate);

    assignments[assignmentCount].completed = false;
    assignmentCount++;

}