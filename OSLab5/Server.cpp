#include <iostream>
#include <windows.h>
#include "Employee.h"

using namespace std;

Employee* emps;
int empsNum;
HANDLE hSemaphore, hMutex;
int readerCount = 0;

DWORD WINAPI InstanceThread(LPVOID param) {
    HANDLE hPipe = (HANDLE)param;
    int empNum, fEmpNum;
    char command[20] = "";
    DWORD dwBytesRead, dwBytesWrote;
    BOOL fSuccess = FALSE;
    bool found = false;

    while (true) {
        fSuccess = ReadFile(hPipe, &command, sizeof(command), &dwBytesRead, NULL);
        if (!fSuccess || dwBytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                cout << "InstanceThread: client is off." << endl;
            else
                cout << "InstanceThread: ReadFile error, GLE: " << GetLastError() << endl;
            break;
        }

        if (strcmp(command, "read") == 0) {
            WaitForSingleObject(hMutex, INFINITE);
            readerCount++;
            if (readerCount == 1)
                WaitForSingleObject(hSemaphore, INFINITE);
            ReleaseMutex(hMutex);

            fSuccess = ReadFile(hPipe, &empNum, sizeof(empNum), &dwBytesRead, NULL);
            if (!fSuccess || dwBytesRead == 0) {
                cout << "InstanceThread: ReadFile error, GLE: " << GetLastError() << endl;
                break;
            }

            found = false;
            for (int i = 0; i < empsNum; i++) {
                if (emps[i].num == empNum) {
                    found = true;
                    fEmpNum = i;
                }
            }

            fSuccess = WriteFile(hPipe, &found, sizeof(bool), &dwBytesWrote, NULL);
            if (!fSuccess || dwBytesWrote == 0) {
                cout << "InstanceThread: WriteFile error, GLE: " << GetLastError() << endl;
                break;
            }

            if (found) {
                fSuccess = WriteFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesWrote, NULL);
                if (!fSuccess || dwBytesWrote == 0) {
                    cout << "InstanceThread: WriteFile error, GLE: " << GetLastError() << endl;
                    break;
                }
            }

            WaitForSingleObject(hMutex, INFINITE);
            readerCount--;
            if (readerCount == 0)
                ReleaseSemaphore(hSemaphore, 1, NULL);
            ReleaseMutex(hMutex);
        }
        else if (strcmp(command, "write") == 0) {
            WaitForSingleObject(hSemaphore, INFINITE);
            fSuccess = ReadFile(hPipe, &empNum, sizeof(empNum), &dwBytesRead, NULL);
            if (!fSuccess || dwBytesRead == 0) {
                cout << "InstanceThread: ReadFile error, GLE: " << GetLastError() << endl;
                break;
            }

            found = false;
            for (int i = 0; i < empsNum; i++) {
                if (emps[i].num == empNum) {
                    found = true;
                    fEmpNum = i;
                }
            }

            fSuccess = WriteFile(hPipe, &found, sizeof(bool), &dwBytesWrote, NULL);
            if (!fSuccess || dwBytesWrote == 0) {
                cout << "InstanceThread: WriteFile error, GLE: " << GetLastError() << endl;
                break;
            }

            if (found) {
                fSuccess = WriteFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesWrote, NULL);
                if (!fSuccess || dwBytesWrote == 0) {
                    cout << "InstanceThread: WriteFile error, GLE: " << GetLastError() << endl;
                    break;
                }

                fSuccess = ReadFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesRead, NULL);
                if (!fSuccess || dwBytesRead == 0) {
                    cout << "InstanceThread: ReadFile error, GLE: " << GetLastError() << endl;
                    break;
                }
            }

            ReleaseSemaphore(hSemaphore, 1, NULL);
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    cout << "Handle finished." << endl;
    return 1;
}

int main()
{
    PROCESS_INFORMATION pi;
    STARTUPINFOW cb;
    wchar_t commandLine[] = L"Client.exe";
    int kolofclients, readerCount = 0;

    cout << "Enter number of students: ";
    cin >> empsNum;
    emps = new Employee[empsNum];

    cout << "Enter student data (number, name, hours):" << endl;
    for (int i = 0; i < empsNum; i++) {
        cout << "Student " << i + 1 << " data:" << endl;
        cout << "number: ";
        cin >> emps[i].num;
        cout << "name: ";
        cin >> emps[i].name;
        cout << "hours: ";
        cin >> emps[i].hours;
    }

    cout << "Enter number of clients: ";
    cin >> kolofclients;

    hSemaphore = CreateSemaphore(NULL, 1, kolofclients, "WriteSempahore");
    hMutex = CreateMutex(NULL, FALSE, "SyncMutex");

    HANDLE hPipe;
    HANDLE* hThreads;
    hThreads = new HANDLE[kolofclients];
    DWORD dwThreadId;
    for (int i = 0; i < kolofclients; i++) {
        hPipe = CreateNamedPipe(
            "\\\\.\\pipe\\pipe_name",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            kolofclients, 0, 0,
            INFINITE,
            NULL);
        if (hPipe == INVALID_HANDLE_VALUE) {
            cout << "Failed to create pipe " << i + 1 << ", GLE: " << GetLastError() << endl;
            return -1;
        }

        ZeroMemory(&cb, sizeof(STARTUPINFOW));
        cb.cb = sizeof(STARTUPINFOW);
        if (!CreateProcessW(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
            NULL, NULL, &cb, &pi)) {
            cout << "Failed to create process " << i + 1 << "." << endl;
            break;
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        if (ConnectNamedPipe(hPipe, NULL)) {
            cout << "Client " << i + 1 << " connected." << endl;
            hThreads[i] = CreateThread(NULL, 0, InstanceThread, (LPVOID)hPipe, 0, &dwThreadId);
            if (hThreads[i] == 0) {
                cout << "Failed to create thread " << i + 1 << "." << endl;
                return -1;
            }
        }
        else {
            CloseHandle(hPipe);
        }
    }

    WaitForMultipleObjects(kolofclients, hThreads, TRUE, INFINITE);
    for (int i = 0; i < kolofclients; i++) {
        CloseHandle(hThreads[i]);
    }
    return 0;
}