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
                cout << "InstanceThread: ������ ��������." << endl;
            else
                cout << "InstanceThread: ������ ReadFile, GLE: " << GetLastError() << endl;
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
                cout << "InstanceThread: ������ ReadFile, GLE: " << GetLastError() << endl;
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
                cout << "InstanceThread: ������ WriteFile, GLE: " << GetLastError() << endl;
                break;
            }

            if (found) {
                fSuccess = WriteFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesWrote, NULL);
                if (!fSuccess || dwBytesWrote == 0) {
                    cout << "InstanceThread: ������ WriteFile, GLE: " << GetLastError() << endl;
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
                cout << "InstanceThread: ������ ReadFile, GLE: " << GetLastError() << endl;
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
                cout << "InstanceThread: ������ WriteFile, GLE: " << GetLastError() << endl;
                break;
            }

            if (found) {
                fSuccess = WriteFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesWrote, NULL);
                if (!fSuccess || dwBytesWrote == 0) {
                    cout << "InstanceThread: ������ WriteFile, GLE: " << GetLastError() << endl;
                    break;
                }

                fSuccess = ReadFile(hPipe, &emps[fEmpNum], sizeof(Employee), &dwBytesRead, NULL);
                if (!fSuccess || dwBytesRead == 0) {
                    cout << "InstanceThread: ������ ReadFile, GLE: " << GetLastError() << endl;
                    break;
                }
            }

            ReleaseSemaphore(hSemaphore, 1, NULL);
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    cout << "����� �������� ���� ������." << endl;
    return 1;
}

int main()
{
    setlocale(LC_ALL, "rus");
    PROCESS_INFORMATION pi;
    STARTUPINFOW cb;
    wchar_t commandLine[] = L"Client.exe";
    int kolofclients, readerCount = 0;

    cout << "������� ���������� ���������: ";
    cin >> empsNum;
    emps = new Employee[empsNum];

    cout << "������� ������ � �������� (�����, ���, ����):" << endl;
    for (int i = 0; i < empsNum; i++) {
        cout << "������ " << i + 1 << "-�� ��������:" << endl;
        cout << "�����: ";
        cin >> emps[i].num;
        cout << "���: ";
        cin >> emps[i].name;
        cout << "����: ";
        cin >> emps[i].hours;
    }

    cout << "������� ���������� ��������: ";
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
            cout << "�� ������� ������� ����� " << i + 1 << ", GLE: " << GetLastError() << endl;
            return -1;
        }

        ZeroMemory(&cb, sizeof(STARTUPINFOW));
        cb.cb = sizeof(STARTUPINFOW);
        if (!CreateProcessW(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
            NULL, NULL, &cb, &pi)) {
            cout << "�� ������� ������� ������� " << i + 1 << "." << endl;
            break;
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        if (ConnectNamedPipe(hPipe, NULL)) {
            cout << "������ " << i + 1 << " ���������." << endl;
            hThreads[i] = CreateThread(NULL, 0, InstanceThread, (LPVOID)hPipe, 0, &dwThreadId);
            if (hThreads[i] == 0) {
                cout << "�� ������� ������� ����� " << i + 1 << "." << endl;
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