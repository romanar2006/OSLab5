#include <iostream>
#include <windows.h>
#include "Employee.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");


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
        cout << "������ ��������" << endl;
        return 1;
    }

    while (true) {

        cout << endl << "������� ������� (read, write ��� exit): ";
        cin >> command;

        if (strcmp(command, "exit") == 0) {
            cout << "���������� ������" << endl;
            break;
        }

        if (!WriteFile(hServer, &command, sizeof(command), &dw, NULL)) {
            cerr << "������ ������" << endl;
            return 1;
        }

        if (strcmp(command, "read") == 0) {
            cout << "������� ����� �������� ��� ������: ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "������ ������" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "������ ������" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "������ �������� ���" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "������ ������" << endl;
                    CloseHandle(hServer);
                    return 1;
                }
                cout << "������ ��������:\n";
                cout << "�����: " << emp.num << "\n";
                cout << "���: " << emp.name << "\n";
                cout << "����: " << emp.hours << "\n";
            }


        }
        else if (strcmp(command, "write") == 0) {
            cout << "������� ����� �������� ��� ������: ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "������ ������" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "������ ������" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "������ �������� ���" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "������ ������" << endl;
                    CloseHandle(hServer);
                    return 1;
                }

                cout << "������ ��������:\n";
                cout << "�����: " << emp.num << "\n";
                cout << "���: " << emp.name << "\n";
                cout << "����: " << emp.hours << "\n";

                cout << "������� ����� ������ �������� c �������: " << emp.num;
                cout << endl;
                cout << "���: ";
                cin >> emp.name;
                cout << "����: ";
                cin >> emp.hours;



                if (!WriteFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "������ ������" << endl;
                    return 1;
                }

                cout << "������ ������� ��������";
            }


        }
    }

    CloseHandle(hServer);
    cout << "THE END";
    return 0;
}