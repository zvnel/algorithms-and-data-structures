#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 100
#define ALPHABET_SIZE 256

// Структура дека
typedef struct {
    long long data[MAX_SIZE];
    int last;
} Deque;

// Инициализация дека
void init_deque(Deque* d) {
    d->last = 0;
}

// Проверка на пустоту
int is_empty(Deque* d) {
    return d->last == 0;
}

// Вставка в начало
int add_front(Deque* d, long long value) {
    if (d->last == MAX_SIZE) { // Проверка на место в деке
        wprintf(L"Ошибка: дек переполнен, невозможно добавить %lld\n", value);
        return -1;
    }
    for (int i = d->last; i > 0; i--) { // Сдвиг элементов вправо
        d->data[i] = d->data[i - 1];
    }
    d->data[0] = value; // Вставка
    d->last++; // Увеличиваем счетчик
    return 0;
}

// Вставка в конец
int add_back(Deque* d, long long value) {
    if (d->last == MAX_SIZE) { // Проверка на место в деке
        wprintf(L"Ошибка: дек переполнен, невозможно добавить %lld\n", value);
        return -1;
    }
    d->data[d->last++] = value; // Вставка
    return 0;
}

// Удаление из начала
long long delete_front(Deque* d) {
    if (is_empty(d)) { // Проверка, что дек не пустой
        wprintf(L"Ошибка: дек пуст, извлечение невозможно\n");
        return -1;
    }
    long long value = d->data[0]; // Удаление первого элемента
    for (int i = 0; i < d->last - 1; i++) { // Сдвиг оставшихся элементов влево на одну позицию
        d->data[i] = d->data[i + 1];
    }
    d->last--;
    return value;
}

// Удаление с конца
long long delete_back(Deque* d) {
    if (is_empty(d)) { // Проверка, что дек не пустой
        wprintf(L"Ошибка: дек пуст, извлечение невозможно\n");
        return -1;
    }
    return d->data[--d->last]; // Удаление последнего элемента
}

// Размер дека
int deque_size(Deque* d) {
    return d->last;
}

// Максимальная длина числа
int find_max_len(Deque* d) {
    int max_len = 0; // Счетчик
    int size = deque_size(d);
    Deque temp; // Временный дек
    init_deque(&temp);

    for (int i = 0; i < size; i++) { // Проходимся по деку
        long long num = delete_front(d);
        add_back(&temp, num);
        int len = 0;
        long long abs_num = llabs(num);
        while (abs_num > 0) { // Проходимся по числу
            len++;
            abs_num /= 10;
        }
        if (len > max_len)
            max_len = len;
    }

    for (int i = 0; i < size; i++) { // Восстановление дека
        add_back(d, delete_front(&temp));
    }

    return max_len;
}

// Распределение чисел по кучам в зависимости от разряда
void to_buckets(Deque* d, long long digit, Deque buckets[10]) {
    int size = deque_size(d); // Находим размер дека
    for (int i = 0; i < size; i++) { // Проходимся по всем элементам
        long long num = delete_front(d); // Достаем число
        int bucket_index = (llabs(num) / digit) % 10; // Достаем разряд
        add_back(&buckets[bucket_index], num); // Добавляем в кучу
    }
}

// Сбор чисел из куч обратно в исходный дек
void from_buckets(Deque* d, Deque buckets[10]) {
    for (int i = 0; i < 10; i++) { // Проходимся по всем кучам
        while (!is_empty(&buckets[i])) {
            add_back(d, delete_front(&buckets[i])); // Достаем элемент из кучи и добавляем в исходный дек
        }
    }
}

// Сбор отрицательных чисел из куч обратно в исходный дек
void from_buckets_negative(Deque* d, Deque buckets[10]) {
    for (int i = 9; i >= 0; i--) { // Проходимся по всем кучам в обратном порядке
        while (!is_empty(&buckets[i])) {
            add_back(d, delete_front(&buckets[i])); // Достаем элемент из кучи и добавляем в исходный дек
        }
    }
}

// Распределяющая (лексикографическая) сортировка
void distributing_sort(Deque* d) {
    Deque positive_numbers, negative_numbers;
    init_deque(&positive_numbers);
    init_deque(&negative_numbers);

    int size = deque_size(d);
    for (int i = 0; i < size; i++) { // Разделяем на положительные и отрицательные числа
        long long num = delete_front(d);
        if (num < 0)
            add_back(&negative_numbers, num); // Добавляем к отрицательным
        else
            add_back(&positive_numbers, num); // Добавляем к положительным
    }

    int max_len = find_max_len(&positive_numbers);
    long long digit = 1;
    for (int i = 0; i < max_len; i++) { // Проходимся по деку с положительными числами
        Deque buckets[10]; // Создаем кучи
        for (int j = 0; j < 10; j++)
            init_deque(&buckets[j]);

        to_buckets(&positive_numbers, digit, buckets); // Распределяем по кучам
        from_buckets(&positive_numbers, buckets); // Обратно объединяем в один дек
        digit *= 10; // Переход к следующему разряду
    }

    max_len = find_max_len(&negative_numbers);
    digit = 1;
    for (int i = 0; i < max_len; i++) { // Проходимся по деку с отрицательными числами
        Deque buckets[10]; // Создаем кучи
        for (int j = 0; j < 10; j++)
            init_deque(&buckets[j]);

        to_buckets(&negative_numbers, digit, buckets); // Распределяем по кучам
        from_buckets_negative(&negative_numbers, buckets); // Обратно объединяем в один дек
        digit *= 10; // Переход к следующему разряду
    }

    while (!is_empty(&negative_numbers)) { // Собираем отрицательные числа в исходный дек
        add_front(d, delete_back(&negative_numbers));
    }
    while (!is_empty(&positive_numbers)) { // Собираем положительные числа в исходный дек
        add_back(d, delete_front(&positive_numbers));
    }
}

// Чтение чисел из файла в дек
void read_to_deque(FILE* file, Deque* d) {
    long long num;
    while (fscanf(file, "%lld", &num) == 1) {
        if (add_back(d, num) == -1) {
            exit(EXIT_FAILURE); // Завершаем программу при ошибке
        }
    }
}

// Вывод чисел из дека
void print_deque(Deque* d) {
    int size = deque_size(d);
    Deque temp; // Временный дек
    init_deque(&temp);
    for (int i = 0; i < size; i++) {
        long long num = delete_front(d);
        wprintf(L"%lld ", num); // Вывод элемента
        add_back(&temp, num); // Сохраняем во временный дек
    }
    for (int i = 0; i < size; i++) { // Возвращаем элементы из временного дека в исходный
        add_back(d, delete_front(&temp));
    }
    wprintf(L"\n");
}

// Преобразуем в строку
void deque_to_string(Deque* d, char* total_str, size_t max_len) {
    char buffer[100];
    Deque temp; // Временный дек
    init_deque(&temp);
    int first = 1;
    size_t len = 0;  // Индекс для текущей позиции в строке

    total_str[0] = '\0';  // Инициализация строки

    while (!is_empty(d)) {
        long long num = delete_front(d);

        // Если это не первое число, добавляем пробел
        if (!first) {
            if (len + 1 < max_len) {
                total_str[len++] = ' ';  // Добавляем пробел
            }
        }

        // Преобразуем число в строку
        int num_len = snprintf(buffer, sizeof(buffer), "%lld", num);

        // Проверяем, есть ли место для добавления числа
        if (len + num_len < max_len) {
            for (int i = 0; i < num_len; i++) {
                total_str[len + i] = buffer[i]; // Записываем число
            }
            len += num_len;
        }

        add_back(&temp, num);  // Сохраняем число, чтобы потом восстановить дек
        first = 0;
    }

    // Восстанавливаем исходный дек
    while (!is_empty(&temp)) {
        add_back(d, delete_front(&temp));
    }

    total_str[len] = '\0';  // Завершаем строку
}


// Предварительное вычисление границ суффиксов
void good_suffix_borders(int* shift, int* bpos, const char* pat, int m) {
    int i = m, j = m + 1; // Границы
    bpos[i] = j;

    while (i > 0) { // Идем до начала строки
        while (j <= m && pat[i - 1] != pat[j - 1]) { // Пока символы не совпадают
            if (shift[j] == 0)
                shift[j] = j - i; // Определяем сдвиг
            j = bpos[j]; // Переход к следующей границе
        }
        i--;
        j--;
        bpos[i] = j;
    }
}

// Хорошие суффиксы
void good_suffix(int* shift, int* bpos, const char* pat, int m) {
    // Инициализация shift и bpos
    for (int i = 0; i <= m; i++) {
        shift[i] = 0;
        bpos[i] = 0;
    }

    // Предварительная обработка
    good_suffix_borders(shift, bpos, pat, m);

    // Дополнительная обработка для префиксов
    int j = bpos[0];
    for (int i = 0; i <= m; i++) {
        if (shift[i] == 0)
            shift[i] = j;
        if (i == j)
            j = bpos[j];
    }
}

// Плохие символы
void bad_char(const char* pat, int m, int badchar[ALPHABET_SIZE]) {
    for (int i = 0; i < ALPHABET_SIZE; i++) // Обнуляем таблицу
        badchar[i] = -1;

    for (int i = 0; i < m; i++) // Обходим шаблон и заполняем таблицу
        badchar[pat[i]] = i;
}

// Поиск Байера-Мура
int boyer_moore_search(const char* text, const char* pattern) { 
    int m = 0;
    while (pattern[m] != '\0') { // Длина шаблона
        m++;
    }

    int n = 0;
    while (text[n] != '\0') { // Длина текста
        n++;
    }

    // Если шаблон пустой или его длина больше текста
    if (m == 0 || n < m)
        return -1;

    int badchar[ALPHABET_SIZE];
    bad_char(pattern, m, badchar);  // Вычисляем таблицу плохих символов

    // Динамически выделяем память для таблиц сдвигов
    int* bpos = (int*)malloc((m + 1) * sizeof(int));
    int* shift = (int*)malloc((m + 1) * sizeof(int));

    // Инициализация таблицы сдвигов
    for (int i = 0; i <= m; i++)
        shift[i] = 0;

    good_suffix(shift, bpos, pattern, m); // Вычисление таблицы сдвигов по хорошему суффиксу

    int s = 0;  // Текущая позиция в тексте
    while (s <= n - m) {  // Пока не выйдем за пределы текста
        int j = m - 1;  // Начинаем с конца шаблона

        // Сравниваем шаблон с соответствующими символами текста справа налево
        while (j >= 0 && pattern[j] == text[s + j])
            j--;  // Если символы совпадают, продолжаем сравнивать дальше

        if (j < 0) {  // Если j < 0, то мы нашли совпадение
            free(bpos);  // Освобождаем память
            free(shift);
            return s;  
        }
        else {
            // Если символы не совпали, вычисляем сдвиги
            int bad_shift = j - badchar[text[s + j]];  // Сдвиг по плохому символу
            int good_shift = shift[j + 1];  // Сдвиг по хорошему суффиксу

            // Сдвигаем шаблон на большее из двух значений сдвига
            if (bad_shift > good_shift) {
                s += bad_shift;
            }
            else {
                s += good_shift;
            }
        }
    }

    free(bpos);  // Освобождаем память
    free(shift);
    return -1;  // Если совпадение не найдено, возвращаем -1
}


void highlight_match(const char* text, int pos, const char* pattern) {
    // Выводим текст до найденной подстроки
    for (int i = 0; i < pos; i++)
        printf("%c", text[i]);

    printf("\x1b[31m"); // Меняем на красный цвет

    // Вычисляем длину шаблона
    int pattern_len = 0;
    while (pattern[pattern_len] != '\0') {
        pattern_len++;
    }

    // Выводим подсвеченную подстроку
    for (int i = 0; i < pattern_len; i++) {
        printf("%c", text[pos + i]);
    }

    printf("\x1b[0m"); // Сбрасываем цвет

    // Выводим оставшуюся часть текста после подстроки
    printf("%s\n", text + pos + pattern_len);
}


int main() {
    setlocale(LC_ALL, "");

    wchar_t filename[100];
    FILE* file = NULL;

    do { // Просим файл
        wprintf(L"Введите имя файла, в котором хранятся числа: ");
        wscanf(L"%ls", filename);
        file = _wfopen(filename, L"r");
        if (!file) wprintf(L"Не удалось открыть файл. Попробуйте снова.\n");
    } while (!file);

    Deque d;
    init_deque(&d);
    read_to_deque(file, &d);
    fclose(file);

    // Выводим последовательности
    wprintf(L"\nИсходная последовательность:\n");
    print_deque(&d);
    distributing_sort(&d);
    wprintf(L"\nОтсортированная последовательность:\n");
    print_deque(&d);

    char total_str[16384];
    deque_to_string(&d, total_str, sizeof(total_str));

    //Просим подстроку для поиска
    char pattern[100]; // Подстрока
    wprintf(L"\nВведите подстроку для поиска в последовательности: ");
    fgetc(stdin); // Удаляем лишний символ 
    fgets(pattern, 100, stdin); // Считываем строку 
    int i = 0;
    while (pattern[i] != '\0') {
        if (pattern[i] == '\n') {
            pattern[i] = '\0';
            break;
        }
        i++;
    }

    int pos = boyer_moore_search(total_str, pattern); // Поиск

    if (pos != -1) {
        wprintf(L"Подстрока найдена в общей строке на позиции %d.\n", pos + 1);
        printf("Последовательность с выделенной подстрокой: ");
        highlight_match(total_str, pos, pattern);
    }
    else {
        wprintf(L"Подстрока не найдена в последовательности.\n");
    }

    return 0;
}
