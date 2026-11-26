# 🎓 Student Assignment Tracker

> A C++ desktop application for Windows for organizing assignments, tracking deadlines, and managing academic progress.

![Status](https://img.shields.io/badge/status-working-brightgreen)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 About the Project

**Student Assignment Tracker** is a personal project created to practice C++ programming and software development fundamentals while building a useful tool for students.

The app asks you to choose **Português** or **English** each time it opens. Use the form to add assignments and select a row to mark it complete or delete it.

## What's New in v2

- **Windows interface.** The desktop window is available in Portuguese and English. It includes a form for adding assignments, a table with due dates and statuses, and a confirmation prompt before deletion.
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

## Temp Run

Use these steps to run the current version from source on Windows until a packaged release is available.

1. Install MinGW-w64 with C++17 support if you don't already have it. The build script looks for `g++.exe` on `PATH` and checks the WinLibs installation under WinGet as a fallback.
2. Open PowerShell in the project folder, where `main.cpp` and `build.ps1` are located.
3. Compile the app. Wait for the `Build complete` message before continuing.

   ```powershell
   .\build.ps1
   ```

   If PowerShell blocks the script because of its execution policy, run `powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1`.

4. Run the generated local executable and choose **Português** or **English** in the startup dialog.

   ```powershell
   .\tracker.exe
   ```

   You can also double-click `tracker.exe` in File Explorer. After editing `main.cpp`, close the app and repeat steps 3 and 4.

Assignments are saved after each change in `%LOCALAPPDATA%\StudentAssignmentTracker\assignments.txt` and loaded when the app opens.

## 🔮 Future Improvements

- Assignment sorting and filtering
- Grade calculations
- Deadline notifications

## 👤 Author

**Davi Toledo Buainain**

Computer Science Student · Douglas College
