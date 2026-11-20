#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;


struct Assignment {
    string course;
    string name;
    string dueDate;
    bool completed = false;
};

bool parseInteger(const string& line, int& number) {
    istringstream input(line);
    char extra;
    return (input >> number) && !(input >> extra);
}

bool containsLetter(const string& text) {
    for (unsigned char character : text) {
        if (isalpha(character)) {
            return true;
        }
    }
    return false;
}

bool isValidDate(const string& date) {
    if (date.size() != 10 || date[4] != date[7] ||
        (date[4] != '-' && date[4] != ' ')) {
        return false;
    }

    for (size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) {
            return false;
        }
    }

    const int year = stoi(date.substr(0, 4));
    const int month = stoi(date.substr(5, 2));
    const int day = stoi(date.substr(8, 2));
    if (year == 0 || month < 1 || month > 12) {
        return false;
    }

    const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leapYear = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    const int maxDay = month == 2 && leapYear ? 29 : daysInMonth[month - 1];
    return day >= 1 && day <= maxDay;
}

bool readField(const char* prompt, const char* error, string& value,
               bool (*isValid)(const string&)) {
    for (;;) {
        cout << prompt;
        if (!getline(cin, value)) {
            return false;
        }
        if (isValid(value)) {
            return true;
        }
        cout << error << '\n';
    }
}

void addAssignment(vector<Assignment>& assignments) {
    Assignment assignment;
    if (!readField("Enter course name: ", "Invalid course name! Try again.",
                   assignment.course, containsLetter) ||
        !readField("Enter assignment name: ", "Invalid assignment name! Try again.",
                   assignment.name, containsLetter) ||
        !readField("Enter due date (YYYY-MM-DD or YYYY MM DD): ", "Invalid date! Try again.",
                   assignment.dueDate, isValidDate)) {
        return;
    }

    assignment.dueDate[4] = '-';
    assignment.dueDate[7] = '-';
    assignments.push_back(assignment);
    cout << "Assignment added successfully!\n";
}

void viewAssignments(const vector<Assignment>& assignments) {
    if (assignments.empty()) {
        cout << "No assignments found!\n";
        return;
    }

    cout << "\n===== YOUR ASSIGNMENTS =====\n";
    for (size_t i = 0; i < assignments.size(); ++i) {
        const Assignment& assignment = assignments[i];
        cout << i + 1 << ". " << assignment.course << " - " << assignment.name << '\n'
             << "Due: " << assignment.dueDate << '\n'
             << "Status: " << (assignment.completed ? "Completed" : "Pending") << "\n\n";
    }
}

int selectAssignment(const vector<Assignment>& assignments, const char* prompt) {
    viewAssignments(assignments);
    if (assignments.empty()) {
        return 0;
    }

    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        return 0;
    }

    int number;
    if (!parseInteger(line, number) || number < 1 ||
        static_cast<size_t>(number) > assignments.size()) {
        cout << "Invalid assignment number!\n";
        return 0;
    }
    return number;
}

void markCompleted(vector<Assignment>& assignments) {
    const int number = selectAssignment(assignments, "Enter assignment number to complete: ");
    if (number != 0) {
        assignments[static_cast<size_t>(number - 1)].completed = true;
        cout << "Assignment marked as completed!\n";
    }
}

void deleteAssignment(vector<Assignment>& assignments) {
    const int number = selectAssignment(assignments, "Enter assignment number to delete: ");
    if (number != 0) {
        assignments.erase(assignments.begin() + (number - 1));
        cout << "Assignment deleted successfully!\n";
    }
}

void displayMenu() {
    cout << "\n===== STUDENT ASSIGNMENT TRACKER =====\n"
         << "1. Add Assignment\n"
         << "2. View Assignments\n"
         << "3. Mark as Completed\n"
         << "4. Delete Assignment\n"
         << "5. Exit\n"
         << "Enter your choice: ";
}

int main() {
    vector<Assignment> assignments;

    while (cin) {
        displayMenu();
        string line;
        if (!getline(cin, line)) {
            break;
        }

        int choice;
        if (!parseInteger(line, choice)) {
            cout << "Invalid option, try again.\n";
            continue;
        }

        switch (choice) {
            case 1:
                addAssignment(assignments);
                break;
            case 2:
                viewAssignments(assignments);
                break;
            case 3:
                markCompleted(assignments);
                break;
            case 4:
                deleteAssignment(assignments);
                break;
            case 5:
                cout << "Goodbye!\n";
                return 0;
            default:
                cout << "Invalid option, try again.\n";
        }
    }
}
