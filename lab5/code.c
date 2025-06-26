#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>
#include <ctype.h>

#define MAX_LEN 100
#define TABLE_SIZE 13
#define R 11
#define MAX_PAIRS 1024
#define HASH_PRIME 5381

HWND hOutput;
HWND hLogin, hPass;
HWND hWord1, hWord2, hWord3;

CRITICAL_SECTION cs;

struct Pair {
    long long x;
    long long y;
};
char generated_pass[6];

// нахождение длины
int len(const char* str) {
    int length = 0;
    while (str[length] != '\0') length++; // пока строка не закончилась - увеличиваем счетчик
    return length;
}

// проверка введенного слова
int is_valid_word(const char* word) {
    int l = len(word); // находим длину
    if (l < 4) return 0; // если длина меньше 4, то слово не подходит
    for (int i = 0; i < l; i++) { // проверяем, что используется именно латиница
        if (!isalpha(word[i])) return 0;
    }
    return 1;
}

// следующая буква
char next_letter(char c) {
    if (c == 'z') return 'a';
    else return c + 1;
}

//предыдущая буква
char prev_letter(char c) {
    if (c == 'a') return 'z';
    else return c - 1;
}

// позиция в алфавите
char letter_by_pos(int pos) {
    pos = ((pos - 1) % 26);
    return 'a' + pos;
}

// генерация пароля
void generate_password(const char* word1, const char* word2, const char* word3, char* result) {
    char base = next_letter(word1[0]);
    result[0] = base; // первый символ
    result[1] = next_letter(base); // второй символ
    result[2] = prev_letter(word2[2]); // третий символ

    int len3 = len(word3); // находим длину третьего слова
    if (len3 % 2 == 1) { // если длина нечетная
        result[3] = next_letter(word3[3]);
    }
    else { // если четная 
        int mid1 = len3 / 2 - 1; // первый средний символ
        int mid2 = len3 / 2; // второй средний символ
        char mid_char = (word3[mid1] < word3[mid2]) ? word3[mid1] : word3[mid2]; // находим меньший
        result[3] = prev_letter(mid_char);
    }

    int sum = len(word1) + len(word2); // складываем длины
    result[4] = letter_by_pos(sum); // пятый символ
    result[5] = '\0';
}

// преобразуем буквенный пароль в численный вид
long long password_to_key(const char* pw) {
    char buf[256] = ""; // буфер для строки
    for (int i = 0; pw[i]; i++) { // проходимся по слову
        char num[4];
        sprintf(num, "%d", (unsigned char)pw[i]); // преобразуем в ASCII-код
        strcat(buf, num); // добавляем в конец строки
    }
    return _atoi64(buf); // преобразуем в число
}

// первое хеширование
int h1(long long k) {
    return k % TABLE_SIZE;
}

// второе хеширование
int h2(long long k) {
    return 1 + (k % R);
}

// двойное хеширование
int double_hash(long long k, int i) {
    return (h1(k) + i * h2(k)) % TABLE_SIZE;
}

// сохраняем логин и хеш-код
void save_user_to_file(const char* login, const char* pw) {
    long long key = password_to_key(pw); // преобразуем пароль
    int index = -1;
    // проходимся по файлу пока не найдем свободное место
    for (int i = 0; i < TABLE_SIZE; i++) {
        index = double_hash(key, i);

        FILE* chk = fopen("user.txt", "r"); // открываем файл
        int taken = 0;
        char log[MAX_LEN];
        int idx;

        if (chk) { // если файл открылся
            while (fscanf(chk, "%s %d", log, &idx) == 2) { // считываем строки
                if (idx == index) { // сравниваем
                    taken = 1;
                    break;
                }
            }
            fclose(chk);
        }

        if (!taken) break; // если нашли свободную строку 
    }
    // запись в файл
    FILE* f = fopen("user.txt", "a");
    if (f) {
        fprintf(f, "%s %d\n", login, index);
        fclose(f);
    }
}

// ищем пользователя
int find_user_in_file(const char* login, const char* password) {
    long long key = password_to_key(password); // преобразуем пароль

    // перебираем
    for (int i = 0; i < TABLE_SIZE; i++) {
        int expected = double_hash(key, i); // вычисляем хеш

        FILE* file = fopen("user.txt", "r"); // открываем файл
        if (!file) return 0;

        char temp_login[MAX_LEN];
        int temp;
        while (fscanf(file, "%s %d", temp_login, &temp) == 2) { // проходимся по строкам
            if (strcmp(temp_login, login) == 0) { // сверяем логины
                fclose(file);
                return temp == expected; // сверяем хеши
            }
        }

        fclose(file);
    }

    return 0;
}

// хеширование
unsigned int compute_hash(struct Pair* arr, int count) {
    unsigned int hash = HASH_PRIME;
    for (int i = 0; i < count; i++) { // проходимся по всем элементам массива
        hash = ((hash << 5) + hash) + (unsigned int)arr[i].x; // для x
        hash = ((hash << 5) + hash) + (unsigned int)arr[i].y; // для y
    }
    return hash;
}

// оценка криптостойкости
double evaluate_password_entropy(const char* password) {
    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    int l = len(password);

    for (int i = 0; i < l; i++) { // смотрим, какие символы есть в пароле
        if (islower(password[i])) has_lower = 1;
        else if (isupper(password[i])) has_upper = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }

    int alphabet_size = 0; // размер алфавита
    if (has_lower)   alphabet_size += 26;
    if (has_upper)   alphabet_size += 26;
    if (has_digit)   alphabet_size += 10;
    if (has_special) alphabet_size += 32; 


    return l * log2(alphabet_size);
}

// возвращает цифру в заданном разряде по модулю
int get_digit(long long number, long long digit) {
    return (llabs(number) / digit) % 10;
}

// находит максимальную длину числа в массиве по модулю
int find_max_len(long long* arr, int n) {
    if (n == 0) return 0;
    long long max = llabs(arr[0]); // пусть изначально максимальное значение равно первому числу в массиве
    // проходимся по массиву и ищем наибольшее значение
    for (int i = 1; i < n; i++) {
        if (llabs(arr[i]) > max) {
            max = llabs(arr[i]);
        }
    }

    // находим длину максимального числа
    int len = 0;
    while (max > 0) {
        len++;
        max /= 10;
    }
    return len;
}

// распределяет пары по кучам
void to_buckets_pair(long long* x, long long* y, int count, long long digit,
    long long bx[10][100], long long by[10][100], int* sizes) {
    for (int i = 0; i < count; i++) { // перебираем все пары
        int d = get_digit(x[i], digit); // достаем цифру
        bx[d][sizes[d]] = x[i]; // сохраняем в кучу
        by[d][sizes[d]] = y[i];
        sizes[d]++;
    }
}

// возвращает пары из куч в массив
void from_buckets_pair(long long* x, long long* y,
    long long bx[10][100], long long by[10][100], int* sizes) {
    int idx = 0;
    for (int i = 0; i < 10; i++) { // проходимся по всем кучам
        for (int j = 0; j < sizes[i]; j++) { // по всем элементам
            x[idx] = bx[i][j];
            y[idx] = by[i][j];
            idx++;
        }
        sizes[i] = 0;
    }
}

// распределяющая сортировка пар 
void distributing_sort(struct Pair* arr, int n) {
    long long* x = malloc(n * sizeof(long long)); // массив для x
    long long* y = malloc(n * sizeof(long long)); // массив для y
    for (int i = 0; i < n; i++) { x[i] = arr[i].x; y[i] = arr[i].y; } // разделяем по массивам

    long long pos[1000], pval[1000], neg[1000], nval[1000];
    int pc = 0, nc = 0;
    for (int i = 0; i < n; i++) { // разделяем на положительные и отрицательные
        if (x[i] < 0) { neg[nc] = x[i]; nval[nc++] = y[i]; }
        else { pos[pc] = x[i]; pval[pc++] = y[i]; }
    }

    long long bx[10][100], by[10][100]; int sizes[10] = { 0 };

    int maxlen = find_max_len(pos, pc); // максимальная длина для положительных 
    long long digit = 1;
    for (int i = 0; i < maxlen; i++) { // сортируем
        to_buckets_pair(pos, pval, pc, digit, bx, by, sizes);
        from_buckets_pair(pos, pval, bx, by, sizes);
        digit *= 10;
    }

    maxlen = find_max_len(neg, nc); // максимальная длина для отрицательных 
    digit = 1;
    for (int i = 0; i < maxlen; i++) { // сортируем
        to_buckets_pair(neg, nval, nc, digit, bx, by, sizes);
        from_buckets_pair(neg, nval, bx, by, sizes);
        digit *= 10;
    }

    int k = 0; // объединяем
    for (int i = nc - 1; i >= 0; i--) {
        arr[k].x = neg[i]; arr[k].y = nval[i]; k++;
    }
    for (int i = 0; i < pc; i++) {
        arr[k].x = pos[i]; arr[k].y = pval[i]; k++;
    }

    // сортировка по y при равных x
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n && arr[i].x == arr[j].x; j++) {
            if (arr[i].y > arr[j].y) {
                long long tx = arr[i].x, ty = arr[i].y;
                arr[i].x = arr[j].x; arr[i].y = arr[j].y;
                arr[j].x = tx; arr[j].y = ty;
            }
        }
    }

    free(x); free(y);
}

// дочерний процесс
void run_child(const char* pid_str) {
    char pipe_name[256]; // массив для имени канала
    sprintf(pipe_name, "\\\\.\\pipe\\data_pipe_%s", pid_str); // записываем имя

    // подключение к именованному каналу
    for (int i = 0; i < 10; i++) {
        HANDLE pipe = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); // подключение
        if (pipe != INVALID_HANDLE_VALUE) { // если получилось
            int count;
            struct Pair pairs[MAX_PAIRS];
            DWORD read;
            unsigned int received_hash;

            // чтение количества пар 
            BOOL readCountResult = ReadFile(pipe, &count, sizeof(int), &read, NULL);
            if (!readCountResult || read != sizeof(int)) { // проверка
                CloseHandle(pipe);
                return;
            }

            // чтение хеша 
            BOOL readHashResult = ReadFile(pipe, &received_hash, sizeof(unsigned int), &read, NULL);
            if (!readHashResult || read != sizeof(unsigned int)) { //проверка
                CloseHandle(pipe); // проверка
                return;
            }

            // чтение пар координат 
            BOOL readPairsResult = ReadFile(pipe, pairs, sizeof(struct Pair) * count, &read, NULL);
            if (!readPairsResult || read != sizeof(struct Pair) * count) {
                CloseHandle(pipe); // проверка
                return;
            }

            unsigned int actual_hash = compute_hash(pairs, count); // вычисляем хеш
            int ack = (actual_hash == received_hash); // сравниваем
            DWORD written;

            // запись подтверждения 
            BOOL writeAckResult = WriteFile(pipe, &ack, sizeof(int), &written, NULL);
            if (!writeAckResult || written != sizeof(int)) { // проверка
                CloseHandle(pipe);
                return;
            }

            distributing_sort(pairs, count); // сортируем
            unsigned int result_hash = compute_hash(pairs, count); // хеш

            // запись количества пар
            BOOL writeCountResult = WriteFile(pipe, &count, sizeof(int), &written, NULL);
            if (!writeCountResult || written != sizeof(int)) { // проверка
                CloseHandle(pipe);
                return;
            }

            // запись хеша результатов 
            BOOL writeHashResult = WriteFile(pipe, &result_hash, sizeof(unsigned int), &written, NULL);
            if (!writeHashResult || written != sizeof(unsigned int)) { // проверка
                CloseHandle(pipe);
                return;
            }

            // запись пар координат
            BOOL writePairsResult = WriteFile(pipe, pairs, sizeof(struct Pair) * count, &written, NULL);
            if (!writePairsResult || written != sizeof(struct Pair) * count) { // проверка
                CloseHandle(pipe);
                return;
            }

            CloseHandle(pipe);
            ExitProcess(0);
            return;
        }
        Sleep(100); // если не получилось подключится, ждем и пробуем еще раз
    }
    MessageBox(NULL, "Не удалось подключиться к каналу", "Child Error", MB_OK | MB_ICONERROR);
}

// родительский процесс
void run_parent(const char* filename, HWND output) {
    struct Pair pairs[MAX_PAIRS]; // массив для хранения пар чисел
    int count = 0;

    FILE* f = fopen(filename, "r"); // открываем файл
    if (!f) { // обработка ошибки
        MessageBox(NULL, "Файл не найден", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // считываем пары чисел
    while (fscanf(f, "%lld %lld", &pairs[count].x, &pairs[count].y) == 2) {
        if (++count >= MAX_PAIRS) break;
    }
    fclose(f);

    // проверяем входные данные
    for (int i = 0; i < count; i++) {
        if (pairs[i].x < -10000 || pairs[i].x > 10000 || pairs[i].y < -10000 || pairs[i].y > 10000) {
            MessageBox(NULL, "Данные вне допустимого диапазона", "Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
    }

    // создание уникального имени канала
    char pipe_name[256]; // массив для хранения имени
    DWORD pid = GetCurrentProcessId(); // текущее имя
    sprintf(pipe_name, "\\\\.\\pipe\\data_pipe_%lu", pid); // записываем

    // создание именного канала
    HANDLE pipe = CreateNamedPipeA(
        pipe_name, // имя канала
        PIPE_ACCESS_DUPLEX, // работает в обоих направлениях
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // передача и чтение байтов, ожидание
        1, // количество клиентов
        sizeof(struct Pair) * MAX_PAIRS + 256, // буфер для чтения
        sizeof(struct Pair) * MAX_PAIRS + 256, // для записи
        5000, // время ожидания
        NULL
    );
    // проверка
    if (pipe == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, "Ошибка создания канала", "Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    STARTUPINFOA si = { sizeof(si) }; // информация о настроках нового процесса
    PROCESS_INFORMATION pi; // информация о запущенном процессе
    char exe[MAX_PATH], cmd[MAX_PATH + 64]; // путь к файлу
    GetModuleFileNameA(NULL, exe, MAX_PATH); // получаем путь к исполняемому файлу
    sprintf(cmd, "\"%s\" child %lu", exe, pid); // записываем

    BOOL created = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); // дочерний процесс

    if (!created) { // проверка
        MessageBox(NULL, "Ошибка запуска сортировки", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    BOOL connected = ConnectNamedPipe(pipe, NULL); // подключение клиента

    if (!connected) { // проверка
        DWORD err = GetLastError(); // код последней ошибки
        if (err != ERROR_PIPE_CONNECTED) {
            MessageBox(NULL, "Ошибка подключения к каналу", "Ошибка", MB_OK | MB_ICONERROR);
            CloseHandle(pipe);
            return;
        }
    }

    DWORD written; // записанные байты
    unsigned int hash = compute_hash(pairs, count); // хеш

    BOOL writeCount = WriteFile(pipe, &count, sizeof(int), &written, NULL); // записываем количество элементов
    if (!writeCount || written != sizeof(int)) { // проверка
        MessageBox(NULL, "Ошибка записи количества", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    BOOL writeHash = WriteFile(pipe, &hash, sizeof(unsigned int), &written, NULL); // записываем хеш
    if (!writeHash || written != sizeof(unsigned int)) { // проверка
        MessageBox(NULL, "Ошибка записи хеша", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    BOOL writePairs = WriteFile(pipe, pairs, sizeof(struct Pair) * count, &written, NULL); // записываем пары
    if (!writePairs || written != sizeof(struct Pair) * count) { // проверка
        MessageBox(NULL, "Ошибка записи данных", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    int ack = 0; // для ответа
    DWORD read;
    BOOL readAck = ReadFile(pipe, &ack, sizeof(int), &read, NULL); // чтение
    if (!readAck || read != sizeof(int)) { // проверка
        MessageBox(NULL, "Ошибка чтения подтверждения", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    if (!ack) { // если хеш не совпал
        MessageBox(NULL, "Ошибка: Хеш не совпадает", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    BOOL readCount = ReadFile(pipe, &count, sizeof(int), &read, NULL); // считываем количество
    if (!readCount || read != sizeof(int)) { // проверка
        MessageBox(NULL, "Ошибка чтения количества после сортировки", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    unsigned int result_hash;
    BOOL readHash = ReadFile(pipe, &result_hash, sizeof(unsigned int), &read, NULL); // читаем хеш
    if (!readHash || read != sizeof(unsigned int)) { // проверка
        MessageBox(NULL, "Ошибка чтения хеша результата", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    BOOL readPairs = ReadFile(pipe, pairs, sizeof(struct Pair) * count, &read, NULL); // читаем пары
    if (!readPairs || read != sizeof(struct Pair) * count) { // проверка
        MessageBox(NULL, "Ошибка чтения данных после сортировки", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    unsigned int actual_result_hash = compute_hash(pairs, count); // вычисляем хеш
    if (actual_result_hash != result_hash) { // проверка
        MessageBox(NULL, "Ошибка: Хеш результата не совпадает", "Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(pipe);
        return;
    }

    EnterCriticalSection(&cs); // критическая секция
    char buf[256];
    SendMessage(output, WM_SETTEXT, 0, (LPARAM)""); // очищаем
    for (int i = 0; i < count; i++) { // выводим пары
        sprintf(buf, "%lld %lld\n", pairs[i].x, pairs[i].y);
        SendMessage(output, EM_SETSEL, -1, -1); // перемещаем курсор в конец
        SendMessage(output, EM_REPLACESEL, 0, (LPARAM)buf); // 
    }
    LeaveCriticalSection(&cs); // выходим
    // закрываем
    CloseHandle(pipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

DWORD WINAPI sort_thread(LPVOID param) {
    run_parent((const char*)param, hOutput); // запускаем родительский процесс
    free(param);
    return 0;
}

// очистка окна
void clear_window(HWND hwnd) {
    HWND child = GetWindow(hwnd, GW_CHILD); // находим первый элемент в окне
    while (child) { // перебираем все элементы
        HWND next = GetWindow(child, GW_HWNDNEXT); // берем следующий элемент
        DestroyWindow(child); // очищаем
        child = next;
    }
}

// начальное окно
void show_start_menu(HWND hwnd) {
    clear_window(hwnd); // очищаем окно
    if (hOutput) { // поле вывода
        DestroyWindow(hOutput);
        hOutput = NULL;
    }
    // создаем надписи и кнопки
    CreateWindow("STATIC", "Что вы хотите сделать?", WS_CHILD | WS_VISIBLE,
        100, 20, 300, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("BUTTON", "Регистрация", WS_CHILD | WS_VISIBLE,
        100, 60, 120, 30, hwnd, (HMENU)1, NULL, NULL);
    CreateWindow("BUTTON", "Вход", WS_CHILD | WS_VISIBLE,
        240, 60, 120, 30, hwnd, (HMENU)2, NULL, NULL);
}

// регистрация
void show_registration(HWND hwnd) {
    clear_window(hwnd); // очищаем
    // создаем надписи, кнопки, поля ввода
    CreateWindow("STATIC", "Введите 3 слова для генерации пароля:", WS_CHILD | WS_VISIBLE,
        20, 0, 300, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "Слово 1:", WS_CHILD | WS_VISIBLE, 20, 30, 80, 20, hwnd, NULL, NULL, NULL);
    hWord1 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 30, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "Слово 2:", WS_CHILD | WS_VISIBLE, 20, 60, 80, 20, hwnd, NULL, NULL, NULL);
    hWord2 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 60, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "Слово 3:", WS_CHILD | WS_VISIBLE, 20, 90, 80, 20, hwnd, NULL, NULL, NULL);
    hWord3 = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 90, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "Логин:", WS_CHILD | WS_VISIBLE, 20, 120, 80, 20, hwnd, NULL, NULL, NULL);
    hLogin = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 120, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("BUTTON", "Сохранить", WS_CHILD | WS_VISIBLE, 100, 160, 100, 25, hwnd, (HMENU)3, NULL, NULL);
}

// вход
void show_login(HWND hwnd) {
    clear_window(hwnd); // очищаем
    // создаем надписи, кнопки, поля ввода
    CreateWindow("STATIC", "Логин:", WS_CHILD | WS_VISIBLE, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
    hLogin = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 20, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC", "Пароль:", WS_CHILD | WS_VISIBLE, 20, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hPass = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
        100, 50, 200, 20, hwnd, NULL, NULL, NULL);
    SendMessage(hPass, EM_SETPASSWORDCHAR, '*', 0);
    CreateWindow("BUTTON", "Проверить", WS_CHILD | WS_VISIBLE, 100, 90, 100, 25, hwnd, (HMENU)4, NULL, NULL);
}

// сортировка
void show_sort(HWND hwnd) {
    clear_window(hwnd); // очищаем
    // создаем надписи, кнопки, поля ввода
    CreateWindow("BUTTON", "Выбрать файл", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 25, hwnd, (HMENU)5, NULL, NULL);
    hOutput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER |
        ES_MULTILINE | WS_VSCROLL | ES_READONLY,
        20, 60, 540, 380, hwnd, NULL, NULL, NULL);
}

// обработка регистрации
void handle_registration(HWND hwnd) {
    char w1[MAX_LEN], w2[MAX_LEN], w3[MAX_LEN], login[MAX_LEN];
    int accept = 0; // флаг

    while (!accept) {
        // считываем
        GetWindowText(hWord1, w1, MAX_LEN);
        GetWindowText(hWord2, w2, MAX_LEN);
        GetWindowText(hWord3, w3, MAX_LEN);
        GetWindowText(hLogin, login, MAX_LEN);
        // проверяем слова
        if (!is_valid_word(w1) || !is_valid_word(w2) || !is_valid_word(w3)) {
            MessageBox(hwnd, "Некорректные слова", "Ошибка", MB_OK);
            return;
        }

        generate_password(w1, w2, w3, generated_pass); // генерация
        double strength = evaluate_password_entropy(generated_pass); // стойкость

        char msg[128]; // сообщение результата
        sprintf(msg, "Сгенерированный пароль: %s\nЭнтропия: %.2f бит\nСохранить его?", generated_pass, strength);
        int res = MessageBox(hwnd, msg, "Подтверждение", MB_YESNO | MB_ICONQUESTION);
        // обрабатываем ответ пользователя
        if (res == IDYES) accept = 1; // если согласился
        else { // если не согласился
            SetWindowText(hWord1, ""); SetWindowText(hWord2, ""); SetWindowText(hWord3, ""); SetWindowText(hLogin, "");
            return;
        }
    }

    save_user_to_file(login, generated_pass); // сохраняем логин и хеш в файл
    char msgbuf[128];
    sprintf(msgbuf, "Ваш пароль: %s", generated_pass); // вывод результата
    MessageBox(hwnd, msgbuf, "Регистрация", MB_OK);
    show_start_menu(hwnd); // возвращаемся в начальное окно
}

// обработка входа
void handle_login(HWND hwnd) {
    char login[MAX_LEN] = { 0 }, pw[MAX_LEN] = { 0 };
    // считываем текст
    GetWindowText(hLogin, login, MAX_LEN);
    GetWindowText(hPass, pw, MAX_LEN);

    for (int i = 0; i < MAX_LEN; i++) { // очистка логина от мусора
        if (!isascii(login[i]) || login[i] == '\r' || login[i] == '\n' || login[i] == ' ') {
            login[i] = '\0';
            break;
        }
    }

    for (int i = 0; i < MAX_LEN; i++) { // очищаем пароль от мусора
        if (!isascii(pw[i]) || pw[i] == '\r' || pw[i] == '\n' || pw[i] == ' ') {
            pw[i] = '\0';
            break;
        }
    }

    if (find_user_in_file(login, pw)) { // если нашли в файле с пользователями
        MessageBox(hwnd, "Успешный вход", "ОК", MB_OK);
        show_sort(hwnd); // переходим к сортировке
    }
    else { // если не нашли 
        MessageBox(hwnd, "Неверный логин или пароль", "Ошибка", MB_OK);
        show_start_menu(hwnd); // возвращаемся в начальное окно
    }
}

void handle_file_selection(HWND hwnd) {
    OPENFILENAME ofn = { 0 }; // для окна выбора файла
    char fname[MAX_PATH] = "";

    ofn.lStructSize = sizeof(ofn); // размер 
    ofn.hwndOwner = hwnd; // владелец
    ofn.lpstrFile = fname;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Text Files\0*.txt\0"; // какие файлы показывать
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; // файл и путь должны существовать

    if (GetOpenFileName(&ofn)) { // если выбрал файл
        CreateThread(NULL, 0, sort_thread, _strdup(fname), 0, NULL); // вызываем сортировку
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        show_start_menu(hwnd); // показываем окно
        break;
    // при нажатии кнопки
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case 1: show_registration(hwnd); break; // регистрация
        case 2: show_login(hwnd); break; // вход 
        case 3: handle_registration(hwnd); break; // сохранить
        case 4: handle_login(hwnd); break; // проверить
        case 5: handle_file_selection(hwnd); break; // выбрать файл
        }
        break;

    case WM_DESTROY: // при закрытии окна
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp); 
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShowCmd) {
    if (strstr(lpCmdLine, "child") != NULL) { // проверка на дочерний процесс
        char* pid_str = strstr(lpCmdLine, "child");
        pid_str += 5;
        while (*pid_str == ' ') pid_str++;
        run_child(pid_str);
        return 0;
    }
    setlocale(LC_ALL, ""); // русский язык
    InitializeCriticalSection(&cs); // инициализируем критическую секцию
    WNDCLASS wc = { 0 }; // создаем класс окна
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "SortApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("SortApp", "GUI Auth + Sort", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 520, NULL, NULL, hInst, NULL); // создаем окно
    ShowWindow(hwnd, nShowCmd); // показываем окно
    UpdateWindow(hwnd); // обновляем окно
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { // получаем сообщения
        TranslateMessage(&msg); // обработка ввода с клавиатуры
        DispatchMessage(&msg); // отправляем сообщение 
    }
    DeleteCriticalSection(&cs); // удаляем критическую секцию
    return (int)msg.wParam;
}
