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
        cout << "Ошибка открытия" << endl;
        return 1;
    }

    while (true) {

        cout << endl << "Введите команду (read, write или exit): ";
        cin >> command;

        if (strcmp(command, "exit") == 0) {
            cout << "Завершение работы" << endl;
            break;
        }

        if (!WriteFile(hServer, &command, sizeof(command), &dw, NULL)) {
            cerr << "Ошибка записи" << endl;
            return 1;
        }

        if (strcmp(command, "read") == 0) {
            cout << "Введите номер студента для чтения: ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "Ошибка записи" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "Ошибка чтения" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "Такого студента нет" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Ошибка чтения" << endl;
                    CloseHandle(hServer);
                    return 1;
                }
                cout << "Данные студента:\n";
                cout << "Номер: " << emp.num << "\n";
                cout << "Имя: " << emp.name << "\n";
                cout << "Часы: " << emp.hours << "\n";
            }


        }
        else if (strcmp(command, "write") == 0) {
            cout << "Введите номер студента для записи: ";
            cin >> data;

            if (!WriteFile(hServer, &data, sizeof(data), &dw, NULL)) {
                cerr << "Ошибка записи" << endl;
                return 1;
            }

            if (!ReadFile(hServer, &b, sizeof(bool), &dw, NULL)) {
                cerr << "Ошибка чтения" << endl;
                CloseHandle(hServer);
                return 1;
            }

            if (b == false) {
                cout << "Такого студента нет" << endl;
                continue;
            }
            else {
                if (!ReadFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Ошибка чтения" << endl;
                    CloseHandle(hServer);
                    return 1;
                }

                cout << "Данные студента:\n";
                cout << "Номер: " << emp.num << "\n";
                cout << "Имя: " << emp.name << "\n";
                cout << "Часы: " << emp.hours << "\n";

                cout << "Введите новые данные студента c номером: " << emp.num;
                cout << endl;
                cout << "имя: ";
                cin >> emp.name;
                cout << "часы: ";
                cin >> emp.hours;



                if (!WriteFile(hServer, &emp, sizeof(emp), &dw, NULL)) {
                    cerr << "Ошибка записи" << endl;
                    return 1;
                }

                cout << "Данные успешно записаны";
            }


        }
    }

    CloseHandle(hServer);
    cout << "THE END";
    return 0;
}