# 🎓 Student Assignment Tracker

> A C++ desktop application for Windows for organizing assignments, tracking deadlines, and managing academic progress.

![Status](https://img.shields.io/badge/status-working-brightgreen)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 About the Project

**Student Assignment Tracker** is a personal project created to practice C++ programming and software development fundamentals while building a useful tool for students.

The app opens a Windows desktop window in Portuguese. Use the form to add assignments and select a row to mark it complete or delete it.

## What's New in v2

- **Windows interface.** The original terminal menu was replaced with a Portuguese desktop window. It includes a form for adding assignments, a table with due dates and statuses, and a confirmation prompt before deletion.
- **Automatic saving.** Adding, completing, or deleting an assignment saves the updated list. The tracker restores assignments and their statuses the next time it opens.
- **Deadline calendar.** Dates with pending assignments appear in bold. Select a date to see how many assignments are pending or completed that day.
- **Windows executable.** Once built, `tracker.exe` opens with a double-click in File Explorer.
- **Build and source updates.** The project includes a VS Code build task and a PowerShell build script. In `main.cpp`, `using namespace std;` lets the code use names such as `string`, `wstring`, and `vector` directly.

## ✨ Features

- [x] Add new assignments
- [x] View all assignments
- [x] Mark assignments as completed
- [x] Delete assignments
- [x] Track assignment due dates
- [x] See pending deadlines in the calendar and select a date to see its status
- [x] Save and load assignments automatically
- [ ] Track grades and academic progress

## 🛠️ Technologies

| Category | Technology |
| --- | --- |
| Language | C++ |
| Editor | Visual Studio Code |
| Version Control | Git and GitHub |

## 🚧 Build and Run

Open the project folder in VS Code and press **Ctrl+Shift+B** to build with MinGW-w64. Then run `.\tracker.exe` in the integrated terminal.

From PowerShell in the project folder, run the build script. It also checks the WinGet installation of MinGW-w64 when `g++` is missing from PATH.

```powershell
.\build.ps1
.\tracker.exe
```

To invoke the compiler directly:

```powershell
g++ -std=c++17 -O2 -municode -mwindows -static -o tracker.exe main.cpp -lcomctl32 -lshell32 -lole32 -luuid -lgdi32
.\tracker.exe
```

Assignments are saved after each change in `%LOCALAPPDATA%\StudentAssignmentTracker\assignments.txt` and loaded when the app opens.

## 🔮 Future Improvements

- Assignment sorting and filtering
- Grade calculations
- Deadline notifications

## 👤 Author

**Davi Toledo Buainain**

Computer Science Student · Douglas College
