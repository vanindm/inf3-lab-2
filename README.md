# Лабораторная работа 1

## Зависимости

Компилятор, совместимый со стандартом C++11; CMake

```bash
pacman -Syu gcc cmake
```

## Компиляция

```bash
mkdir -p build
cd build
cmake ..
make
```

## Тестирование

```bash
cd build
ctest
```

## Результаты проверки производительности

Производительность

![Тест](./contrib/perf_test.png)

Использование памяти

![Тест](./contrib/mem_test.png)

![МИФИ](./contrib/mephi.png)
