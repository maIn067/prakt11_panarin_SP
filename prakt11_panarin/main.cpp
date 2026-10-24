#define MAX_ROBOTS 10
#define MAX_ENERGY 100
#define WORKING 0
#define PAUSED 1
#define FINISHED 2
#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <string>
using namespace std;

struct Robot {
    unsigned int ID = 0;
    unsigned int energy = 60;
    unsigned int score = 0;
    unsigned int state = 0;
    HANDLE hThread = NULL;
};

Robot robots[MAX_ROBOTS];
int robotCount = 0;
int* trial = nullptr;
int trialcount = 0;

void firsttrial(Robot* robot) {
    if (robot->energy < 10) {
        cout << "Робот " << robot->ID << " пропускает препятствие, мало энергии (" << robot->energy << ")" << endl;
        return;
    }
    cout << "Робот " << robot->ID << " начал препятствие" << endl;
    Sleep(3000);
    robot->energy -= 10;
    robot->score += 15;
    cout << "Робот " << robot->ID << " прошел препятствие, энергия " << robot->energy << ", очки " << robot->score << endl;
}

void secondtrial(Robot* robot) {
    if (robot->energy < 20) {
        cout << "Робот " << robot->ID << " пропускает спринт, мало энергии (" << robot->energy << ")" << endl;
        return;
    }
    cout << "Робот " << robot->ID << " начал спринт" << endl;
    Sleep(4000);
    robot->energy -= 20;
    robot->score += 30;
    cout << "Робот " << robot->ID << " прошел спринт, энергия " << robot->energy << ", очки " << robot->score << endl;
}

void thirdtrial(Robot* robot) {
    int seconds = rand() % 5 + 1;
    cout << "Робот " << robot->ID << " начал подзарядку на " << seconds << " сек" << endl;
    Sleep(seconds * 1000);
    robot->energy += seconds * 10;
    if (robot->energy > MAX_ENERGY) {
        robot->energy = MAX_ENERGY;
    }
    cout << "Робот " << robot->ID << " закончил подзарядку, энергия " << robot->energy << endl;
}

void fourthtrial(Robot* robot) {
    if (robot->energy < 60) {
        cout << "Робот " << robot->ID << " пропускает марафон, мало энергии (" << robot->energy << ")" << endl;
        return;
    }
    cout << "Робот " << robot->ID << " начал марафон" << endl;
    Sleep(10000);
    robot->energy -= 60;
    robot->score += 55;
    cout << "Робот " << robot->ID << " прошел марафон, энергия " << robot->energy << ", очки " << robot->score << endl;
}

DWORD WINAPI robotwork(LPVOID param) {
    Robot* robot = (Robot*)param;
    srand(time(NULL) + robot->ID);
    Sleep(robot->ID * 200);
    for (int i = 0; i < trialcount; i++) {
        if (trial[i] == 1) {
            firsttrial(robot);
        }
        else if (trial[i] == 2) {
            secondtrial(robot);
        }
        else if (trial[i] == 3) {
            thirdtrial(robot);
        }
        else if (trial[i] == 4) {
            fourthtrial(robot);
        }
        if (i < trialcount - 1) {
            Sleep((rand() % 3 + 1) * 1000);
        }
    }
    robot->state = FINISHED;
    cout << "Робот " << robot->ID << " завершил испытания" << endl;
    return 0;
}

bool loadarena() {
    ifstream in("arena.dat", ios::binary);
    if (!in) {
        cout << "Файл arena.dat не найден" << endl;
        return false;
    }
    in.seekg(0, ios::end);
    trialcount = in.tellg() / sizeof(int);
    in.seekg(0, ios::beg);
    trial = new int[trialcount];
    in.read(reinterpret_cast<char*>(trial), trialcount * sizeof(int));
    in.close();
    return true;
}

bool allfinished() {
    for (int i = 0; i < robotCount; i++) {
        if (robots[i].state != FINISHED) {
            return false;
        }
    }
    return true;
}

void switchrobot(int index) {
    if (robots[index].state == WORKING) {
        SuspendThread(robots[index].hThread);
        robots[index].state = PAUSED;
        cout << "Робот " << robots[index].ID << " приостановлен" << endl;
    }
    else if (robots[index].state == PAUSED) {
        robots[index].state = WORKING;
        ResumeThread(robots[index].hThread);
        cout << "Робот " << robots[index].ID << " возобновлен" << endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    if (!loadarena()) {
        return 0;
    }
    cout << "Испытания:";
    for (int i = 0; i < trialcount; i++) {
        cout << " " << trial[i];
    }
    cout << endl;
    do {
        cout << "Количество роботов (от 3 до 10): ";
        cin >> robotCount;
    } while (robotCount < 3 || robotCount > MAX_ROBOTS);

    for (int i = 0; i < robotCount; i++) {
        robots[i].ID = i + 1;
    }
    cout << "Нажмите номер робота (1-9, 0 - десятый), чтобы приостановить или возобновить его" << endl;
    cout << endl;

    for (int i = 0; i < robotCount; i++) {
        robots[i].hThread = CreateThread(NULL, 0, robotwork, &robots[i], 0, NULL);
    }
    while (!allfinished()) {
        if (_kbhit()) {
            char key = _getch();
            int index = -1;
            if (key >= '1' && key <= '9') {
                index = key - '1';
            }
            else if (key == '0') {
                index = 9;
            }
            if (index >= 0 && index < robotCount) {
                switchrobot(index);
            }
        }
        Sleep(100);
    }
    for (int i = 0; i < robotCount; i++) {
        WaitForSingleObject(robots[i].hThread, INFINITE);
        CloseHandle(robots[i].hThread);
    }
    for (int i = 0; i < robotCount - 1; i++) {
        for (int j = 0; j < robotCount - 1 - i; j++) {
            if (robots[j].score < robots[j + 1].score) {
                Robot temp = robots[j];
                robots[j] = robots[j + 1];
                robots[j + 1] = temp;
            }
        }
    }
    cout << endl << "Результаты:" << endl;
    for (int i = 0; i < robotCount; i++) {
        cout << i + 1 << ". Робот " << robots[i].ID << ", очки: " << robots[i].score << ", энергия: " << robots[i].energy << endl;
    }
    if (robots[0].score == robots[1].score) {
        cout << "Ничья" << endl;
    }
    else {
        cout << "Победитель: Робот " << robots[0].ID << endl;
    }
    delete[] trial;
    trial = nullptr;
}