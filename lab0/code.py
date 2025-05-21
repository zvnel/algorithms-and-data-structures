# получение коэффициентов от пользователя
while True:
    coefficients = input("Введите коэффициенты a, b, c, d через пробел: ") #попросили у пользователя ввести a, b, c, d
    # дальше проверяем, что ввели именно числа, а не другие символы
    try:
        a, b, c, d = map(float, coefficients.split()) #разделяем введенную пользователем строку и преобразуем каждый элемент в число с плавающей точкой
        break
    except ValueError:
        print("Ошибка, введите числовые значения.")

#если введены числа, то выводим полученное кубическое уравнение
print("Ваше уравнение:", a, "* x**3 +", b, "* x**2 +", c, "* x +", d, "= 0")

# функция, которая возвращает значения исходной кубической функции в точке х
def f(x):
    global a, b, c, d
    return a * x ** 3 + b * x ** 2 + c * x + d

# функция для вычисления производной первой степени
def f_first_derivative(x):
    global a, b, c
    return 3 * a * x ** 2 + 2 * b * x + c

# функция для вычисления производной второй степени
def f_second_derivative(x):
    global a, b
    return 6 * a * x + 2 * b

# комбинированный метод хорд и касательных
def combined_method_of_chords_and_tangents(start, end, tolerance=0.0002):

    x_prev = float('inf')  # предыдущее значение (для начала сохраняем просто какое-то большое число)
    x_now = (start + end) / 2   # середина интервала

    # пока разница между соседними х больше допустимого значения, будет работать цикл
    while abs(x_now - x_prev) > tolerance:
        x_prev = x_now

        # определяем, какую формулу использовать
        if f(start) * f_second_derivative(start) < 0:
            # используем метод хорд
             x_now = start - f(start) / (f(end) - f(start)) * (end - start)
        else:
            # используем метод касательных
             x_now = start - f(start) / f_first_derivative(start)

        # обновляем границы интервала
        if f(x_now) * f(start) < 0:
            end = x_now
        else:
            start = x_now

        # проверка на случай, если производная слишком мала
        if abs(f_first_derivative(x_now)) < 1e-6:
            x_now = (start + end) / 2

    return x_now

#если коэффициент a не равен нулю
if a != 0:
    max_point = 1 + max(abs(a), abs(b), abs(c), abs(d)) / abs(a) # определяем максимальную точку поиска корней
    min_point = -max_point # определяем минимальную точку поиска корней
    inflection_point = -b / (3 * a) # находим точку перегиба
    Disc_derivative = (2 * b) ** 2 - 12 * a * c  # находим дискриминант для квадратного уравнения, полученного как производная исходного

    # рассматриваем случай, когда дискриминант отрицательный, то есть будет один корень
    if Disc_derivative <= 0:
        if f(inflection_point) * a > 0:
            x = combined_method_of_chords_and_tangents(min_point, inflection_point)
        elif f(inflection_point) * a < 0:
            x = combined_method_of_chords_and_tangents(inflection_point, max_point)
        else:
            x = inflection_point
        print("Решение равно", x)

    # рассматриваем случай, когда дискриминант отрицательный
    elif Disc_derivative > 0:
        x1_d = (-2 * b - Disc_derivative ** 0.5) / (6 * a)
        x2_d = (-2 * b + Disc_derivative ** 0.5) / (6 * a)

        # случай, когда получаем один корень
        if f(x1_d) * f(x2_d) > 0:
            if f(inflection_point) * a > 0:
                x = combined_method_of_chords_and_tangents(min_point, min(x1_d, x2_d))
            else:
                x = combined_method_of_chords_and_tangents(max(x1_d, x2_d), max_point)
            print("Решение равно", x)

        # случай, когда получаем два корня
        elif f(x1_d) * f(x2_d) == 0:
            if f(inflection_point) * a > 0:
                x2 = max(x1_d, x2_d)
                x1 = combined_method_of_chords_and_tangents(min_point, min(x1_d, x2_d))
            else:
                x2 = min(x1_d, x2_d)
                x1 = combined_method_of_chords_and_tangents(max(x1_d, x2_d), max_point)
            print("Решение равно", x1, x2)

        # случай, когда получаем три корня
        else:
            x1 = combined_method_of_chords_and_tangents(min_point, min(x1_d, x2_d))
            x3 = combined_method_of_chords_and_tangents(max(x1_d, x2_d), max_point)
            if f(inflection_point) * a == 0:
                x2 = inflection_point
            elif f(inflection_point) * a > 0:
                x2 = combined_method_of_chords_and_tangents(inflection_point, max(x1_d, x2_d))
            else:
                x2 = combined_method_of_chords_and_tangents(min(x1_d, x2_d), inflection_point)
            print("Решение равно", x1, x2, x3)

#если коэффициент b не равен нулю
elif b != 0:
    Disc = c ** 2 - 4 * b * d
    if Disc < 0:
        print("Решений нет")
    elif Disc > 0:
        x1 = (-c - Disc ** 0.5) / (2 * b)
        x2 = (-c + Disc ** 0.5) / (2 * b)
        print("Решения:", x1, "и", x2)
    else:
        x = -c / (2 * b)
        print("Решение равно", x)

#если коэффициент c не равен нулю
elif c != 0:
    x = -d / c
    print("Решение равно", x)

#если коэффициент d не равен нулю
elif d != 0:
    print("Решений нет")

#если коэффициент d равен нулю
else:
    print("Решение - любое число")
