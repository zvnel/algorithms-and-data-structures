#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>

// функция, которая возвращает цифру в заданном разряде (по модулю)
int get_digit(long long number, long long digit) {
    return (llabs(number) / digit) % 10;
}

// функция, которая находит максимальную длину числа в массиве (по модулю)
int find_max_len(long long* arr, int n) {
    if (n == 0) return 0;
    long long max = llabs(arr[0]); // пусть изначально максимальное значение равно первому числу в массиве
    // проходимся по массиву и ищем наибольшее значение
    for (int i = 1; i < n; i++)
        if (llabs(arr[i]) > max)
            max = llabs(arr[i]);

    // находим длину максимального числа (сколько в нем цифр)
    int len = 0;
    while (max > 0) {
        len++;
        max /= 10;
    }
    return len;
}

// функция, которая распределяет элементы по кучкам
void to_buckets(long long* arr, int start, int finish, long long digit, long long buckets[10][100], int* sizes) {
    for (int i = start; i <= finish; i++) {
        // проходимся по массиву и распределяем числа по кучкам (от 0 до 9) в зависимости от цифры в разряде
        int num = get_digit(arr[i], digit);
        buckets[num][sizes[num]] = arr[i];
        sizes[num]++;
    }
}

// функция для объединения кучек в один массив
void from_buckets(long long* arr, long long buckets[10][100], int* sizes) {
    int index = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < sizes[i]; j++) {
            arr[index] = buckets[i][j];
            index++;
        }
        sizes[i] = 0; // очистка кучки для следующей итерации
    }
}

// распределяющая (лексикографическая) сортировка 
void distributing_sort(long long* arr, int n) {
    long long negatives[1000], positives[1000];
    int neg_count = 0, pos_count = 0;

    // разделение на отрицательные и положительные
    for (int i = 0; i < n; i++) {
        if (arr[i] < 0)
            negatives[neg_count++] = arr[i];
        else
            positives[pos_count++] = arr[i];
    }

    long long buckets[10][100]; // массив для 10 кучек
    int sizes[10] = { 0 }; // массив для отслеживания размеров кучек
    int max_len, i;
    long long digit;

    // сортировка положительных чисел
    max_len = find_max_len(positives, pos_count);
    digit = 1; // начинаем с младшего разряда
    for (i = 0; i < max_len; i++) {
        to_buckets(positives, 0, pos_count - 1, digit, buckets, sizes); // распределяем элементы по кучкам
        from_buckets(positives, buckets, sizes); // сбор всех элементов из кучек в один массив
        digit *= 10; // переход к следующему разряду
    }

    // сортировка отрицательных чисел по модулю
    max_len = find_max_len(negatives, neg_count);
    digit = 1; // начинаем с младшего разряда
    for (i = 0; i < max_len; i++) {
        to_buckets(negatives, 0, neg_count - 1, digit, buckets, sizes); // распределяем элементы по кучкам
        from_buckets(negatives, buckets, sizes); // сбор всех элементов из кучек в один массив
        digit *= 10; // переход к следующему разряду
    }

    // объединение отрицательных и положительных
    int index = 0;
    for (i = neg_count - 1; i >= 0; i--)
        arr[index++] = negatives[i];
    for (i = 0; i < pos_count; i++)
        arr[index++] = positives[i];
}

// функция для вывода массива
void print_array(long long* arr, int n) {
    for (int i = 0; i < n; i++) {
        wprintf(L"%lld ", arr[i]);
    }
    wprintf(L"\n");
}

int main() {
    setlocale(LC_ALL, ""); // для поддержки русских символов

    wchar_t filename[100]; // переменная, в которой хранится имя файла
    FILE* file = NULL; // указатель на файл

    // просим пользователя ввести имя файла
    do {
        wprintf(L"Введите имя файла, в котором хранятся числа: ");
        wscanf(L"%ls", filename);
        file = _wfopen(filename, L"r");

        if (file == NULL) {
            wprintf(L"Не удалось открыть файл. Введите имя файла повторно.\n\n");
        }
    } while (file == NULL);

    long long* arr = NULL; // указатель на массив с числами
    int n = 0; // количество чисел
    int type;

    // просим пользователя выбрать тип массива
    do {
        wprintf(L"Выберите тип массива (1 — статический массив; 2 — динамический массив):\n");
        wscanf(L"%d", &type);

        if (type != 1 && type != 2)
            wprintf(L"Некорректный выбор. Попробуйте снова.\n");
    } while (type != 1 && type != 2);

    // если пользователь выбрал статический массив
    if (type == 1) {
        long long static_arr[1000];
        arr = static_arr;
        while (fscanf(file, "%lld", &arr[n]) == 1 && n < 1000)
            n++;
    }

    // если пользователь выбрал динамический массив
    else if (type == 2) {
        int size = 2;
        arr = malloc(size * sizeof(long long));
        
        while (fscanf(file, "%lld", &arr[n]) == 1) {
            n++;
            if (n >= size) {
                size += 1;
                long long* new_arr = realloc(arr, size * sizeof(long long));
                
                arr = new_arr;
            }
        }
    }

    fclose(file);

    // вывод исходного массива
    wprintf(L"Исходный массив:\n");
    print_array(arr, n);

    // сортировка
    distributing_sort(arr, n);

    // вывод отсортированного массива
    wprintf(L"\nОтсортированный массив:\n");
    print_array(arr, n);

    // освобождаем память
    if (type == 2)
        free(arr);

    return 0;
}
