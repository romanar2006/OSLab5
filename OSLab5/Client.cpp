#include <iostream>
#include <windows.h>
#include "Employee.h"

using namespace std;

int main()
{
    char command[20];
    int data;
    Employee emp;
    DWORD dw;
    bool b;

    HANDLE hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "WriteSemaphore");
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "SyncMutex");
    HANDLE hServer = CreateFileA(
        "\\\\.\\pipe\\pipe_name",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        0,
        (HANDLE)NULL
    );

    if (hServer == INVALID_HANDLE_VALUE) {
        cout << "Open error" << endl;
        return 1;
    }

    while (true) {

        cout << endl << "Enter sonething (read, write или exit): ";
        cin >> command;

        if (strcmp(command, "exit") == 0) {
            cout << "The end." << endl;
            break;
        }

        if (!WriteFile(hServer, &command, sizeof(command), &dw, NULL)) {
            cerr << "Failed to write" << endl;
            return 1;
        }

        if (strcmp(command, "read") == 0) {
            cout << "Enter student number to read: ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "Failed to write" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "Failed to read" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "No student" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Failed to read" << endl;
                    CloseHandle(hServer);
                    return 1;
                }
                cout << "Student data:\n";
                cout << "Number: " << emp.num << "\n";
                cout << "Name: " << emp.name << "\n";
                cout << "Hours: " << emp.hours << "\n";
            }


        }
        else if (strcmp(command, "write") == 0) {
            cout << "Enter student number to write ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "Failed to write" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "Failed to read" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "No student" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Failed to read" << endl;
                    CloseHandle(hServer);
                    return 1;
                }

                cout << "Student data:\n";
                cout << "Number: " << emp.num << "\n";
                cout << "Name: " << emp.name << "\n";
                cout << "Hours: " << emp.hours << "\n";

                cout << "Enter new data for student: " << emp.num;
                cout << endl;
                cout << "name: ";
                cin >> emp.name;
                cout << "hours: ";
                cin >> emp.hours;



                if (!WriteFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Failed to write" << endl;
                    return 1;
                }

                cout << "Data updated";
            }


        }
    }

    CloseHandle(hServer);
    cout << "THE END";
    return 0;
}