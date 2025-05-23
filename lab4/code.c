#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>

// Структура узла для связного списка
typedef struct Node {
    long long data;
    struct Node* next;
} Node;

// Структура кольцевой очереди
typedef struct {
    Node* head; 
    Node* tail; 
    int size;
} Queue;

// Инициализация кольцевой очереди
void init_queue(Queue* Q) {
    Q->head = NULL;
    Q->tail = NULL;
    Q->size = 0;
}

// Проверка очереди на пустоту
int is_empty(Queue* Q) {
    return Q->size == 0;
}

// Добавление элемента
void add(Queue* Q, long long value) {
    Node* new_node = (Node*)malloc(sizeof(Node)); // Указатель на узел (выделение памяти)
    new_node->data = value; // Записываем значение

    if (is_empty(Q)) { // Проверяем очередь на пустоту
        new_node->next = new_node; // Кольцо из одного элемента
        Q->head = new_node;
        Q->tail = new_node;
    }
    else {
        new_node->next = Q->head; // Добавляем новый узел
        Q->tail->next = new_node;
        Q->tail = new_node;
    }
    Q->size++;
}

// Удаление элемента
long long delete(Queue* Q) {
    if (is_empty(Q)) { // Проверяем, что очередб не пустая
        printf("\nОчередь пуста\n");
        exit(1);
    }

    Node* to_delete = Q->head; // Сохраняем первый элемент
    long long value = to_delete->data;

    if (Q->head == Q->tail) { // В очереди один элемент
        Q->head = NULL;
        Q->tail = NULL;
    }
    else { // Сдвигаем
        Q->head = Q->head->next;
        Q->tail->next = Q->head;
    }

    free(to_delete);
    Q->size--;

    return value;
}

// Размер очереди
int queue_size(Queue* Q) {
    return Q->size;
}

// Получение разряда числа
int get_digit(long long number, long long digit) {
    return (llabs(number) / digit) % 10;
}

// Максимальнаядлина
int find_max_len(Queue* q) {
    int max_len = 0, size = queue_size(q);
    Queue temp; // Временная очередь
    init_queue(&temp);
    for (int i = 0; i < size; i++) { // Проходимся по очереди
        long long num = delete(q); // Достаем элемент
        add(&temp, num);
        int len = 0;
        long long abs_num = llabs(num);
        while (abs_num > 0) {
            len++; 
            abs_num /= 10; // Переходим к следующему разряду 
        }
        if (len > max_len) max_len = len;
    }
    for (int i = 0; i < size; i++) add(q, delete(&temp)); // Восстанавливаем очередь
    return max_len;
}

// Распределение чисел по кучам в зависимости от разряда
void to_buckets(Queue* q, long long digit, Queue buckets[10]) {
    int size = queue_size(q);
    for (int i = 0; i < size; i++) { // Проходимся по очереди
        long long num = delete(q); // Достаем элемент
        int bucket_index = get_digit(num, digit);
        add(&buckets[bucket_index], num); // Добавляем в кучу
    }
}

// Сбор чисел из куч обратно в исходную очередь
void from_buckets(Queue* q, Queue buckets[10]) {
    for (int i = 0; i < 10; i++) // Проходимся по всем кучам
        while (!is_empty(&buckets[i]))
            add(q, delete(&buckets[i])); // Достаем элемент из кучи и добавляем в исходную очередь
}

// Распределяющая (лексикографическая) сортировка
void distributing_sort(Queue* q) {
    Queue positive_numbers, negative_numbers;
    init_queue(&positive_numbers);
    init_queue(&negative_numbers);
    int size = queue_size(q);
    for (int i = 0; i < size; i++) { // Разделяем на положительные и отрицательные числа
        long long num = delete(q);
        if (num < 0) add(&negative_numbers, num);// Добавляем к отрицательным
        else add(&positive_numbers, num); // Добавляем к положительным
    }

    int max_len = find_max_len(&positive_numbers);
    long long digit = 1;
    for (int i = 0; i < max_len; i++) { // Проходимся по очереди с положительными числами
        Queue buckets[10]; // Создаем кучи
        for (int j = 0; j < 10; j++) init_queue(&buckets[j]);
        to_buckets(&positive_numbers, digit, buckets); // Распределяем по кучам
        from_buckets(&positive_numbers, buckets); // Обратно объединяем в одну очередь
        digit *= 10; // Переход к следующему разряду
    }

    max_len = find_max_len(&negative_numbers);
    digit = 1;
    for (int i = 0; i < max_len; i++) { // Проходимся по очереди с отрицательными числами
        Queue buckets[10]; // Создаем кучи
        for (int j = 0; j < 10; j++) init_queue(&buckets[j]);
        to_buckets(&negative_numbers, digit, buckets); // Распределяем по кучам
        from_buckets(&negative_numbers, buckets); // Обратно объединяем в одну очередь
        digit *= 10; // Переход к следующему разряду
    }

    long long temp[100];
    int neg_size = queue_size(&negative_numbers); 
    for (int i = 0; i < neg_size; i++) temp[i] = delete(&negative_numbers); // Собираем отрицательные числа в исходную очередь
    for (int i = neg_size - 1; i >= 0; i--) add(q, temp[i]);

    int pos_size = queue_size(&positive_numbers);
    for (int i = 0; i < pos_size; i++) add(q, delete(&positive_numbers)); // Собираем положительные числа в исходную очередь
}

// Чтение чисел из файла
void read_to_queue(FILE* file, Queue* q) {
    long long num;
    while (fscanf(file, "%lld", &num) == 1) add(q, num);
}

// Вывод чисел из очереди
void print_queue(Queue* q) {
    int size = queue_size(q);
    for (int i = 0; i < size; i++) {
        long long num = delete(q); // Достаем элемент
        wprintf(L"%lld ", num); // Вывод элемента
        add(q, num); // Убираем обратно
    }
    wprintf(L"\n");
}

// Красно-чёрное дерево
typedef enum { RED, BLACK } Color;

// Структура узла дерева
typedef struct TreeNode {
    long long data; // Значение, хранимое в узле
    Color color; // Цвет узла
    struct TreeNode* left, * right, * parent; // Потомки и родитель
} TreeNode;

// Структура дерева
typedef struct {
    TreeNode* root; // Указатель на корень дерева
    TreeNode* nil; // Указатель на специальный nil-узел
} RBTree;

// Инициализация дерева
void init_rbtree(RBTree* tree) {
    tree->nil = (TreeNode*)malloc(sizeof(TreeNode)); // Выделение память под nil-узел
    tree->nil->color = BLACK; // Цвет делаем черным
    tree->nil->left = tree->nil->right = tree->nil->parent = NULL; // Обнуляем родителей и потомков
    tree->root = tree->nil; // Корень
}

// Левое вращение
void left_rotate(RBTree* tree, TreeNode* x) {
    TreeNode* y = x->right; // Делаем правым ребенком
    x->right = y->left; // У узла x правым ребёнком теперь будет то, что было левым ребёнком у y
    if (y->left != tree->nil) y->left->parent = x; // Если левый потомок y не nil, то его родителем теперь становится x
    y->parent = x->parent; 
    if (x->parent == tree->nil) tree->root = y; // Если x был корнем, то корнем дерева становится y
    else if (x == x->parent->left) x->parent->left = y; // Если x был левым ребёнком, y становится левым ребёнком
    else x->parent->right = y; // Если x был правым ребёнком, y становится правым ребёнком
    y->left = x; // x становится левым потомком y, то есть смещается вниз и влево
    x->parent = y; // y родитель x
}

// Правое вращение
void right_rotate(RBTree* tree, TreeNode* y) {
    TreeNode* x = y->left; // x - левый ребёнок y
    y->left = x->right; // У узла y левым ребёнком теперь будет то, что было правым ребёнком у x
    if (x->right != tree->nil) x->right->parent = y; // Если правый потомок x не nil, то его родителем теперь становится y
    x->parent = y->parent;
    if (y->parent == tree->nil) tree->root = x; // Если y был корнем, то теперь корень дерева — x
    else if (y == y->parent->right) y->parent->right = x; // Если y был правым ребёнком, то правым ребёнком становится x
    else y->parent->left = x; // Если y был левым ребёнком, x становится левым ребёнком
    x->right = y; // y становится правым ребёнком x
    y->parent = x; // x становится родителем y
}

// Восстановление свойств дерева
void insert_fixup(RBTree* tree, TreeNode* z) {
    while (z->parent->color == RED) { // Пока родитель z является красным
        if (z->parent == z->parent->parent->left) { // Если родитель z — левый ребёнок от деда
            TreeNode* y = z->parent->parent->right; // y — правый ребёнок деда (это дядя справа узла z)
            if (y->color == RED) { // Если красный 
                z->parent->color = BLACK; // Родителя z перекрашиваем в черный
                y->color = BLACK; // Дядю тоже перекрашиваем в черный
                z->parent->parent->color = RED; // А деда в красный
                z = z->parent->parent; // Поднимаемся выше по дереву (к деду)
            }
            else { // Если дядя черный или нулевой
                if (z == z->parent->right) { // z — правый ребёнок 
                    z = z->parent; // Двигаем указатель
                    left_rotate(tree, z); // Вращаем влево
                }
                z->parent->color = BLACK; // Родителя z перекрашиваем в черный
                z->parent->parent->color = RED; // А деда в красный
                right_rotate(tree, z->parent->parent); // Вращаем вправо
            }
        }
        else { // Если родитель z — правый ребёнок от деда
            TreeNode* y = z->parent->parent->left; // y — левый ребёнок деда (это дядя слева узла z)
            if (y->color == RED) { // Если красный
                z->parent->color = BLACK; // Родителя z перекрашиваем в черный
                y->color = BLACK; // Дядю тоже перекрашиваем в черный
                z->parent->parent->color = RED; // А деда в красный
                z = z->parent->parent; // Поднимаемся выше по дереву (к деду)
            }
            else { // Если дядя черный или нулевой
                if (z == z->parent->left) { // z — левый ребёнок
                    z = z->parent; // Двигаем указатель
                    right_rotate(tree, z); // Вращаем вправо
                }
                z->parent->color = BLACK; // Родителя z перекрашиваем в черный
                z->parent->parent->color = RED; // А деда в красный
                left_rotate(tree, z->parent->parent); // Вращаем влево
            }
        }
    }
    tree->root->color = BLACK; // Гарантируем, что корень всегда чёрный
}
// Добавление в дерево
void insert_rbtree(RBTree* tree, long long value) {
    TreeNode* z = (TreeNode*)malloc(sizeof(TreeNode)); // Выделение памяти под новый узел
    z->data = value; // Записываем значение
    z->color = RED; // Красим в красный
    z->left = z->right = z->parent = tree->nil; // Изначально нет детей, родителей

    TreeNode* y = tree->nil;
    TreeNode* x = tree->root;

    while (x != tree->nil) { // Перемещаемся от корня
        y = x;
        if (z->data < x->data) // Если значение меньше
            x = x->left; // Идем влево 
        else // Иначе вправо
            x = x->right;
    }

    z->parent = y; // Родитель нового узла
    if (y == tree->nil) // Если дерево было пустым, то z - корень
        tree->root = z;
    else if (z->data < y->data) // Если значение меньше, то вставляем как левого ребенка
        y->left = z;
    else // Иначе как правого
        y->right = z;

    insert_fixup(tree, z); // Восстанавливаем свойства дерева
}

// Вывод содержимого
void inorder_print(TreeNode* node, RBTree* tree) {
    if (node != tree->nil) { // Проверяем, что узел не пустой
        inorder_print(node->left, tree); // Идем по левому поддереву, печатаем все узлы, которые меньше текущего
        wprintf(L"%lld ", node->data); // Печатаем текущий узел
        inorder_print(node->right, tree); // Идем по правому поддереву, печатаем все узлы, которые больше текущего
    }
}

int main() {
    setlocale(LC_ALL, "");
    wchar_t filename[100];
    FILE* file = NULL;
    do { // Просим ввести имя файла 
        wprintf(L"Введите имя файла, в котором хранятся числа: ");
        wscanf(L"%ls", filename);
        file = _wfopen(filename, L"r");
        if (!file)
            wprintf(L"Не удалось открыть файл. Введите имя файла повторно.\n\n");
    } while (!file);

    Queue q;
    init_queue(&q);
    read_to_queue(file, &q); // Читаем в очередь
    fclose(file);

    wprintf(L"Исходная последовательность:\n");
    print_queue(&q);

    distributing_sort(&q); // Сортировка

    RBTree tree;
    init_rbtree(&tree); // Создаем дерево
    while (!is_empty(&q)) insert_rbtree(&tree, delete(&q)); // Заполняем дерево

    wprintf(L"\nОтсортированная последовательность:\n");
    inorder_print(tree.root, &tree); // Выводим последовательность
    wprintf(L"\n");

    return 0;
}
