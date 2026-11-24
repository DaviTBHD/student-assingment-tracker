#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace fs = filesystem;

struct Assignment {
    wstring course;
    wstring name;
    wstring dueDate;
    bool completed = false;
};

bool isValidDate(const wstring& date) {
    if (date.size() != 10 || date[4] != date[7] ||
        (date[4] != L'-' && date[4] != L' ')) {
        return false;
    }

    for (size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && (date[i] < L'0' || date[i] > L'9')) {
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

string toUtf8(const wstring& value) {
    const int length = static_cast<int>(value.size());
    const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                                          length, nullptr, 0, nullptr, nullptr);
    if (count == 0) return {};
    string result(count, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), length,
                        result.data(), count, nullptr, nullptr);
    return result;
}

wstring fromUtf8(const string& value) {
    const int length = static_cast<int>(value.size());
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                          length, nullptr, 0);
    if (count == 0) return {};
    wstring result(count, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), length,
                        result.data(), count);
    return result;
}

bool loadAssignments(const fs::path& path, vector<Assignment>& assignments) {
    error_code error;
    const bool exists = fs::exists(path, error);
    if (error || !exists) return !error;

    ifstream file(path);
    string line;
    if (!getline(file, line) || line != "TRACKER 1") return false;

    vector<Assignment> loaded;
    while (getline(file, line)) {
        istringstream row(line);
        string course, name, date;
        int completed;
        char extra;
        if (!(row >> quoted(course) >> quoted(name) >> quoted(date) >> completed)
            || (row >> extra) || (completed != 0 && completed != 1)) {
            return false;
        }
        Assignment assignment{fromUtf8(course), fromUtf8(name), fromUtf8(date), completed == 1};
        if (assignment.course.empty() || assignment.name.empty() || !isValidDate(assignment.dueDate)) {
            return false;
        }
        loaded.push_back(move(assignment));
    }
    if (file.bad() || !file.eof()) return false;
    assignments = move(loaded);
    return true;
}

bool saveAssignments(const fs::path& path, const vector<Assignment>& assignments) {
    fs::path temporary = path;
    temporary += L".tmp";
    ofstream file(temporary, ios::trunc);
    file << "TRACKER 1\n";
    for (const auto& assignment : assignments) {
        const auto course = toUtf8(assignment.course);
        const auto name = toUtf8(assignment.name);
        const auto date = toUtf8(assignment.dueDate);
        if (course.empty() || name.empty() || date.empty()) return false;
        file << quoted(course) << ' ' << quoted(name) << ' '
             << quoted(date) << ' ' << assignment.completed << '\n';
    }
    file.close();
    return file && MoveFileExW(temporary.c_str(), path.c_str(),
                               MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

namespace {
constexpr wchar_t AppTitle[] = L"Agenda de trabalhos";
enum ControlId { Add = 100, Complete, Remove, List, Calendar };

struct TrackerWindow {
    fs::path dataFile;
    vector<Assignment> assignments;
    vector<MONTHDAYSTATE> calendarDays;
    HWND window{}, course{}, name{}, date{}, list{}, summary{}, status{}, complete{}, remove{};
    HWND calendar{}, daySummary{};
    HFONT font{}, headingFont{};

    ~TrackerWindow() {
        if (font) DeleteObject(font);
        if (headingFont) DeleteObject(headingFont);
    }

    HWND control(const wchar_t* type, const wchar_t* label, DWORD style,
                 int x, int y, int width, int height, int id = 0, DWORD exStyle = 0) const {
        HWND handle = CreateWindowExW(exStyle, type, label, WS_CHILD | WS_VISIBLE | style,
                                      x, y, width, height, window,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                      GetModuleHandleW(nullptr), nullptr);
        SendMessageW(handle, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        return handle;
    }

    void createControls() {
        font = CreateFontW(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        headingFont = CreateFontW(-32, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        HWND title = control(L"STATIC", AppTitle, 0, 28, 22, 680, 43);
        SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(headingFont), TRUE);
        summary = control(L"STATIC", L"", 0, 30, 69, 680, 25);

        control(L"STATIC", L"Disciplina", 0, 30, 113, 165, 23);
        control(L"STATIC", L"Nome do trabalho", 0, 209, 113, 255, 23);
        control(L"STATIC", L"Prazo (AAAA-MM-DD)", 0, 479, 113, 220, 23);
        course = control(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL,
                         30, 140, 165, 30, 0, WS_EX_CLIENTEDGE);
        name = control(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL,
                       209, 140, 255, 30, 0, WS_EX_CLIENTEDGE);
        date = control(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL,
                       479, 140, 220, 30, 0, WS_EX_CLIENTEDGE);
        SendMessageW(course, EM_SETLIMITTEXT, 200, 0);
        SendMessageW(name, EM_SETLIMITTEXT, 200, 0);
        SendMessageW(date, EM_SETLIMITTEXT, 10, 0);
        control(L"BUTTON", L"Adicionar trabalho", WS_TABSTOP | BS_PUSHBUTTON,
                30, 185, 176, 36, Add);

        list = control(WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                       30, 244, 669, 240, List, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
        const wchar_t* headers[] = {L"Disciplina", L"Trabalho", L"Prazo", L"Status"};
        const int widths[] = {145, 259, 119, 117};
        for (int i = 0; i < 4; ++i) {
            LVCOLUMNW column{};
            column.mask = LVCF_TEXT | LVCF_WIDTH;
            column.pszText = const_cast<wchar_t*>(headers[i]);
            column.cx = widths[i];
            ListView_InsertColumn(list, i, &column);
        }

        complete = control(L"BUTTON", L"Marcar como concluído", WS_TABSTOP,
                           30, 502, 205, 36, Complete);
        remove = control(L"BUTTON", L"Excluir trabalho", WS_TABSTOP, 247, 502, 158, 36, Remove);
        status = control(L"STATIC", L"", 0, 30, 552, 669, 30);

        control(L"STATIC", L"Calendário de prazos", 0, 724, 113, 245, 23);
        calendar = control(MONTHCAL_CLASSW, L"", WS_TABSTOP | MCS_DAYSTATE,
                           724, 140, 245, 215, Calendar);
        RECT calendarBounds{};
        if (MonthCal_GetMinReqRect(calendar, &calendarBounds)) {
            SetWindowPos(calendar, nullptr, 0, 0, calendarBounds.right, calendarBounds.bottom,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        control(L"STATIC", L"Negrito indica prazo pendente.", 0, 724, 371, 245, 25);
        daySummary = control(L"STATIC", L"", 0, 724, 407, 245, 65);

        refreshList();
        if (assignments.empty()) SetWindowTextW(status, L"Preencha os campos acima para adicionar um trabalho.");
    }

    vector<MONTHDAYSTATE> pendingDays(SYSTEMTIME start, int months) const {
        vector<MONTHDAYSTATE> days(months);
        const int firstMonth = start.wYear * 12 + start.wMonth;
        for (const auto& assignment : assignments) {
            if (assignment.completed) continue;
            const auto& due = assignment.dueDate;
            const int month = stoi(due.substr(0, 4)) * 12 + stoi(due.substr(5, 2));
            const int offset = month - firstMonth;
            if (offset >= 0 && offset < months) {
                const int day = stoi(due.substr(8, 2));
                days[offset] |= MONTHDAYSTATE{1} << (day - 1);
            }
        }
        return days;
    }

    void showDay(const SYSTEMTIME& day) const {
        wostringstream dateKey;
        dateKey << setfill(L'0') << setw(4) << day.wYear << L'-'
                << setw(2) << day.wMonth << L'-' << setw(2) << day.wDay;
        const auto selectedDate = dateKey.str();

        int pending = 0;
        int completed = 0;
        for (const auto& assignment : assignments) {
            if (assignment.dueDate != selectedDate) continue;
            if (assignment.completed) ++completed;
            else ++pending;
        }

        const wstring message = selectedDate + L"\r\n" +
            (pending == 0 && completed == 0
                ? L"Nenhum trabalho nesta data."
                : to_wstring(pending) + L" pendente(s) / " +
                  to_wstring(completed) + L" concluído(s)");
        SetWindowTextW(daySummary, message.c_str());
    }

    void refreshCalendar() const {
        SYSTEMTIME range[2]{};
        const int months = MonthCal_GetMonthRange(calendar, GMR_DAYSTATE, range);
        if (months > 0) {
            const auto days = pendingDays(range[0], months);
            MonthCal_SetDayState(calendar, months, days.data());
        }

        SYSTEMTIME selected{};
        if (MonthCal_GetCurSel(calendar, &selected)) showDay(selected);
    }

    void updateActions() const {
        const int selection = ListView_GetNextItem(list, -1, LVNI_SELECTED);
        EnableWindow(remove, selection >= 0);
        EnableWindow(complete, selection >= 0 && !assignments[selection].completed);
    }

    void refreshList(int selection = -1) {
        ListView_DeleteAllItems(list);
        for (size_t i = 0; i < assignments.size(); ++i) {
            const auto& assignment = assignments[i];
            LVITEMW row{};
            row.mask = LVIF_TEXT;
            row.iItem = static_cast<int>(i);
            row.pszText = const_cast<wchar_t*>(assignment.course.c_str());
            const int index = ListView_InsertItem(list, &row);
            if (index < 0) break;
            ListView_SetItemText(list, index, 1, const_cast<wchar_t*>(assignment.name.c_str()));
            ListView_SetItemText(list, index, 2, const_cast<wchar_t*>(assignment.dueDate.c_str()));
            ListView_SetItemText(list, index, 3,
                                 const_cast<wchar_t*>(assignment.completed ? L"Concluído" : L"Pendente"));
        }
        const auto completed = count_if(assignments.begin(), assignments.end(),
                                             [](const auto& item) { return item.completed; });
        const wstring counts = to_wstring(assignments.size() - completed) + L" pendentes   /   " +
                                    to_wstring(completed) + L" concluídos";
        SetWindowTextW(summary, counts.c_str());
        if (selection >= 0) {
            ListView_SetItemState(list, selection, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(list, selection, FALSE);
        }
        updateActions();
        refreshCalendar();
    }

    static wstring textOf(HWND input) {
        wstring value(GetWindowTextLengthW(input) + 1, L'\0');
        value.resize(GetWindowTextW(input, value.data(), static_cast<int>(value.size())));
        const auto first = value.find_first_not_of(L" \t");
        if (first == wstring::npos) return {};
        return value.substr(first, value.find_last_not_of(L" \t") - first + 1);
    }

    bool commit(vector<Assignment> next, const wchar_t* message, int selection) {
        if (!saveAssignments(dataFile, next)) {
            MessageBoxW(window, L"Não foi possível salvar. Tente novamente.", AppTitle, MB_OK | MB_ICONERROR);
            return false;
        }
        assignments = move(next);
        refreshList(selection);
        SetWindowTextW(status, message);
        return true;
    }

    void add() {
        Assignment assignment{textOf(course), textOf(name), textOf(date)};
        if (assignment.course.empty() || assignment.name.empty()) {
            SetWindowTextW(status, L"Preencha a disciplina e o nome do trabalho.");
            SetFocus(assignment.course.empty() ? course : name);
            return;
        }
        if (!isValidDate(assignment.dueDate)) {
            SetWindowTextW(status, L"Informe uma data válida no formato AAAA-MM-DD.");
            SetFocus(date);
            return;
        }
        assignment.dueDate[4] = assignment.dueDate[7] = L'-';
        auto next = assignments;
        next.push_back(move(assignment));
        if (commit(move(next), L"Trabalho adicionado e salvo.", static_cast<int>(assignments.size()))) {
            SetWindowTextW(course, L"");
            SetWindowTextW(name, L"");
            SetWindowTextW(date, L"");
            SetFocus(course);
        }
    }

    void changeSelected(int action) {
        const int selection = ListView_GetNextItem(list, -1, LVNI_SELECTED);
        if (selection < 0) return;

        auto next = assignments;
        if (action == Complete) {
            if (next[selection].completed) return;
            next[selection].completed = true;
            commit(move(next), L"Trabalho marcado como concluído.", selection);
        } else if (MessageBoxW(window, L"Excluir o trabalho selecionado?", AppTitle,
                               MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
            next.erase(next.begin() + selection);
            commit(move(next), L"Trabalho excluído.", -1);
        }
    }
};

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* creation = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* tracker = static_cast<TrackerWindow*>(creation->lpCreateParams);
        tracker->window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tracker));
    }

    auto* tracker = reinterpret_cast<TrackerWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    switch (message) {
        case WM_CREATE:
            tracker->createControls();
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == Add) tracker->add();
            if (LOWORD(wParam) == Complete || LOWORD(wParam) == Remove) tracker->changeSelected(LOWORD(wParam));
            return 0;
        case WM_NOTIFY: {
            const auto* notice = reinterpret_cast<NMHDR*>(lParam);
            if (wParam == List && notice->code == LVN_ITEMCHANGED) {
                tracker->updateActions();
            }
            if (wParam == Calendar && notice->code == MCN_GETDAYSTATE) {
                auto* state = reinterpret_cast<NMDAYSTATE*>(lParam);
                tracker->calendarDays = tracker->pendingDays(state->stStart, state->cDayState);
                state->prgDayState = tracker->calendarDays.data();
            }
            if (wParam == Calendar && notice->code == MCN_SELCHANGE && tracker->daySummary) {
                tracker->showDay(reinterpret_cast<const NMSELCHANGE*>(lParam)->stSelStart);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int showTracker(HINSTANCE instance, int show, const fs::path& dataFile) {
    TrackerWindow tracker;
    tracker.dataFile = dataFile;
    if (!loadAssignments(dataFile, tracker.assignments)) {
        MessageBoxW(nullptr, L"Não foi possível carregar o arquivo de trabalhos.", AppTitle, MB_OK | MB_ICONERROR);
        return 1;
    }

    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES};
    InitCommonControlsEx(&controls);
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = L"StudentAssignmentTracker";
    if (!RegisterClassW(&windowClass)) return 1;

    constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT bounds{0, 0, 995, 595};
    AdjustWindowRect(&bounds, style, FALSE);
    HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, windowClass.lpszClassName, AppTitle, style,
                                   CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left,
                                   bounds.bottom - bounds.top, nullptr, nullptr, instance, &tracker);
    if (!window) return 1;

    ShowWindow(window, show);
    SetFocus(tracker.course);
    MSG message{};
    BOOL result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (message.message == WM_KEYDOWN && message.wParam == VK_RETURN &&
            (message.hwnd == tracker.course || message.hwnd == tracker.name || message.hwnd == tracker.date)) {
            tracker.add();
        } else if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    PWSTR localAppData = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppData))) {
        MessageBoxW(nullptr, L"A pasta de dados do usuário está indisponível.", AppTitle, MB_OK | MB_ICONERROR);
        return 1;
    }
    const fs::path directory = fs::path(localAppData) / L"StudentAssignmentTracker";
    CoTaskMemFree(localAppData);

    error_code error;
    fs::create_directories(directory, error);
    if (error) {
        MessageBoxW(nullptr, L"Não foi possível preparar a pasta de dados.", AppTitle, MB_OK | MB_ICONERROR);
        return 1;
    }
    HANDLE lock = CreateFileW((directory / L"assignments.lock").c_str(), GENERIC_WRITE, 0, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (lock == INVALID_HANDLE_VALUE) {
        const wchar_t* message = GetLastError() == ERROR_SHARING_VIOLATION
            ? L"A agenda já está aberta."
            : L"Não foi possível acessar a pasta de dados.";
        MessageBoxW(nullptr, message, AppTitle, MB_OK | MB_ICONERROR);
        return 1;
    }
    const int result = showTracker(instance, show, directory / L"assignments.txt");
    CloseHandle(lock);
    return result;
}
