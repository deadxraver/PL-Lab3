# Assignment: Image rotation
---
Лабораторная работа: Поворот картинки

# Подготовка

- Прочитайте главу 12 (стр. 221, 231&ndash;239) и 13 (целиком) "Low-level programming: C, assembly and program execution". 

На защите мы можем обсуждать любые вопросы из учебника из глав 8&ndash;13 включительно.

# Структура BMP файла

BMP файл состоит из заголовка и растрового массива.
Заголовок задаётся следующей структурой (обратите внимание на атрибут `packed`):

```c
// Описание для gcc и clang
#include  <stdint.h>
struct bmp_header __attribute__((packed))
{
        uint16_t bfType;
        uint32_t  bfileSize;
        uint32_t bfReserved;
        uint32_t bOffBits;
        uint32_t biSize;
        uint32_t biWidth;
        uint32_t  biHeight;
        uint16_t  biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        uint32_t biXPelsPerMeter;
        uint32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t  biClrImportant;
};
```

Сразу после него (всегда ли?) идёт растровый массив, в котором последовательно хранятся пиксели по строчкам.
Каждый пиксель задаётся структурой размером 3 байта:

```c
   struct pixel { uint8_t b, g, r; };
```

## Padding

Если ширина изображения в байтах кратна четырём, то строчки идут одна за другой без пропусков.
Если ширина не кратна четырём, то она дополняется мусорными байтами до ближайшего числа, кратного четырём.
Эти байты называются *padding*.

Пример:

1. Изображение имеет ширину 12 пикселей = 12 * 3 байт = 36 байт. Ширина кратна четырём, каждая следующая строчка начинается сразу после предыдущей.
2. Изображение имеет ширину 5 пикселей. 5 * 3 = 15 байт, ближайшее число кратное четырём (округление вверх) это 16. После каждой строчки будет отступ в 16-15 = 1 байт перед началом следующей.

Обратите внимание: отступы в *байтах*, не в пикселях.



# Пользователям компилятора от Microsoft 

Вам придётся задать структуру по-другому, без атрибута `packed`:

```c
#include  <stdint.h>
#pragma pack(push, 1)
struct bmp_header 
{
        uint16_t bfType;
        uint32_t  bfileSize;
        uint32_t bfReserved;
        uint32_t bfOffBits;
        uint32_t biSize;
        uint32_t biWidth;
        uint32_t  biHeight;
        uint16_t  biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        uint32_t biXPelsPerMeter;
        uint32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t  biClrImportant;
};
#pragma pack(pop)
```

Объяснение этого прочтите находится на страницах 235&ndash;239 учебника. 


# Об архитектуре

Программа разделена на модули; каждый модуль это `.c` файл, который становится файлом с расширением `.o`.

Продуманная архитектура приложения в каждом конкретном модуле минимизирует
знания о других модулях по следующим причинам:

- Когда программист работает над одним модулем (разрабатывает, модифицирует,
  ищет ошибки), ему проще не держать в голове знания про всю остальную
  программу. Скрытие информации про другие модули позволяет ему не брать в 
  голову детали их внутреннего устройства.
- Пусть модуль A не использует определения из модуля B, но имеет к ним доступ.
  Разумеется, можно как угодно менять B и это не скажется на A. 
  Однако есть шанс, что автор программы или кто-то из будущих соавторов может
  использовать в модуле A определения из B &mdash; ведь к ним есть доступ.  Это
  установит жёсткую связь между A и B, и будет нельзя больше свободно менять B не
  влияя на A. Программы являются сложными системами, и мы хотим иметь минимум
  связей между их элементами, иначе модификация (и исправление ошибок) будут
  требовать постоянной модификации не одной, а многих частей программы.

В нашем случае в программе разумно выделить несколько частей.

## Часть 1: Внутренний формат

Описание внутреннего представления картинки `struct image`, очищенное от
деталей формата, и функции для работы с ним: создание, деинициализация и т.д.

   ```c
   struct image {
     uint64_t width, height;
     struct pixel* data;
   };
   ```
  
  Эта часть программы не должна знать ни про входные форматы, ни про трансформации.

## Часть 2: Входные форматы

Каждый входной формат описывается в отдельном модуле; они предоставляют функции
для считывания файлов разных форматов в `struct image` и для записи на диск в
тех же форматах.

Эти модули знают про модуль, описывающий `struct image`, но ничего не знают про
трансформации. Поэтому можно будет добавлять новые трансформации не переписывая
код для входных форматов.

  Как только мы считали изображение во внутренний формат, мы должны забыть, из
какого формата оно было считано!  Именно поэтому в `struct image` оставлен
только самый минимум деталей изображения (размеры), и никаких частей
bmp-заголовка.  Для BMP начать можно с:

```c
/*  deserializer   */
enum read_status  {
  READ_OK = 0,
  READ_INVALID_SIGNATURE,
  READ_INVALID_BITS,
  READ_INVALID_HEADER
  /* коды других ошибок  */
  };

enum read_status from_bmp( FILE* in, struct image* img );

/*  serializer   */
enum  write_status  {
  WRITE_OK = 0,
  WRITE_ERROR
  /* коды других ошибок  */
};

enum write_status to_bmp( FILE* out, struct image const* img );

```

Функции `from_bmp` и `to_bmp` принимают уже открытый файл, что позволяет
им работать с заранее открытыми файлами `stdin`, `stdout`, `stderr`.

Функции `from_bmp` и `to_bmp` не должны ни открывать, ни закрывать файлы.
Для ошибок открытия/закрытия, возможно, вам захочется ввести отдельные типы
перечислений.

Как только мы считали изображение во внутренний формат, мы должны забыть, из
какого формата оно было считано! Именно поэтому в `struct image` оставлен
только самый минимум деталей изображения (размеры), и никаких частей
bmp-заголовка.

Вам также потребуются функции, аналогичные `from_bmp` и `to_bmp`, которые
будут принимать имена файлов и заниматься корректным открытием (`fopen`) и
закрытием (`fclose`) файлов; на открытых файлах они могут запускать `from_bmp`
и `to_bmp`.

Имеет смысл разделять открытие/закрытие файлов и работу с ними. Уже
открытие и закрытие могут сопровождаться ошибками (см. `man fopen` и 
`man fclose`) и хочется отделить обработку ошибок открытия/закрытия и
обработку ошибок чтения/записи.



## Часть 3: Трансформации

Каждая трансформация описывается в отдельном модуле. Эти модули знают про
модуль, описывающий `struct image`, но ничего не знают про входные форматы.
Поэтому можно будет добавлять новые входные форматы не переписывая код для
трансформаций. Без дополнительных усилий мы получим возможность, описав входной
формат, сразу же поддержать все трансформации над ним.

Вам потребуется функция для поворота картинки в её внутреннем представлении:

   ```c
   /* создаёт копию изображения, которая повёрнута на 90 градусов */
   struct image rotate( struct image const source );
   ```
   
Вам может пригодиться [программа для отображения заголовков BMP файлов](https://gitlab.se.ifmo.ru/low-level-programming/bmp-header-viewer) 
**ВНИМАНИЕ** это программа для просмотра заголовков BMP файлов. Это не заготовка для решения! Можете скомпилировать её с помощью `make` и проверять заголовки на битность; в решении вам нужно поддерживать только 24-битные BMP файлы.

## Часть 4: всё остальное

Остальная часть программы может быть организована любым осмысленным способом. Возможно, вам захочется написать небольшую библиотеку для ввода-вывода, работы со строками и т.д.

Приветствуется разумное создание новых модулей и введение дополнительных функций для удобства, где это необходимо.

Дополнительные функции, которые вы ввели для удобства, но которые не относятся по смыслу ни к одному из этих модулей, можно выделить
в отдельный модуль. Часто его называют `util.c` или как-то похоже.

# Задание


- Необходимо реализовать поворот изображения в формате BMP на 90 градусов против 
  часовой стрелки. Формат использования такой:

```
./image-transformer <source-image> <transformed-image>
```

- Архитектура приложения описана в предыдущем разделе. 
- Код размещается в директории `solution/src`, заголовочные файлы ищутся в `solution/include`.

## Система сборки и тестирования

Для сборки кода вам предоставлена система сборки на языке CMake, самим писать систему сборки не требуется.

- В зависимости от платформы и компилятора, система сборки поддерживает несколько конфигураций с динамическими
  анализаторами (санитайзерами). Санитайзеры могут дать подробную информацию о возможных и реальных ошибках в
  программе вместо классического сообщения о segmentation fault. Выбрать подходящую конфигурацию вы можете с
  помощью переменной `CMAKE_BUILD_TYPE`:

  - `ASan` &mdash; [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html),
    набор проверок на некорректное использование адресов памяти. Примеры:
    use-after-free, double-free, выход за пределы стека, кучи или статического блока.

  - `LSan` &mdash; [LeakSanitizer](https://clang.llvm.org/docs/LeakSanitizer.html),
    проверки на утечки памяти.

  - `MSan` &mdash; [MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html),
    проверяет, что любая используемая ячейка памяти проинициализирована на момент чтения из нее.

  - `UBSan` &mdash; [UndefinedBehaviourSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html),
    набор базовых проверок на неопределенное поведение. Примеры: переполнение численного типа,
    null-pointer dereference.

- Если в вашей системе имеется статический анализатор `clang-tidy`, он будет запущен во время компиляции программы.
  Список проверок описан в файле `clang-tidy-checks.txt`. Вы можете добавить свои проверки в конец этого файла.

- Директория `tester` содержит код и изображения для тестирования вашей программы. Для запуска тестов используется CTest.

- Поддержана интеграция системы сборки со средами разработки CLion, Visual Studio и Visual Studio Code.

Чтобы система сборки работала на вашей системе, вам необходимо:

### Linux и MacOS

- Компилятор (`gcc`/`clang`) и `cmake` (проверьте, что `cmake` версии 3.12 или выше)
- Если вы хотите использовать санитайзеры с GCC, установите `libasan`, `liblsan` и `libubsan` с помощью пакетного менеджера (названия могут отличаться).
- Если вы хотите использовать санитайзеры с Clang, на некоторых системах вам может понадобиться пакет `compiler-rt`.
- Если вы хотите пользоваться `clang-tidy`, установите `clang-tools-extra`.

### Windows

- Какая-либо среда разработки (CLion, Visual Studio, Visual Studio Code)
- Если вы хотите пользоваться `clang-tidy`, скачайте LLVM: https://github.com/llvm/llvm-project/releases (найдите установщик win64 под одной из версий)
- Для VS Code требуется отдельно поставить Visual Studio (с сайта Microsoft) и CMake: https://cmake.org/download/

### Инструкции по сборке и тестированию

- [Работа с терминалом](docs/Terminal.md)
- [CLion](docs/CLion.md)
- [Visual Studio](docs/Visual%20Studio.md)
- [Visual Studio Code](docs/VSCode.md)

# Для самопроверки

- Прочитайте [правила хорошего стиля](https://gitlab.se.ifmo.ru/c-language/main/-/wikis/%D0%9F%D1%80%D0%B0%D0%B2%D0%B8%D0%BB%D0%B0-%D1%81%D1%82%D0%B8%D0%BB%D1%8F-%D0%BD%D0%B0%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D1%8F-%D0%BF%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC-%D0%BD%D0%B0-C). Ваше решение должно им соответствовать.
- Архитектура: думайте о том, как бы вы хотели организовать код, чтобы легко добавлять входные форматы (не только BMP) и трансформации (не только поворот на 90 градусов).
- Пожалуйста, присылайте решение в виде pull-request. [Инструкция](https://gitlab.se.ifmo.ru/cse/main/-/wikis/%D0%9A%D0%B0%D0%BA-%D0%BF%D0%BE%D1%81%D0%BB%D0%B0%D1%82%D1%8C-%D0%B7%D0%B0%D0%B4%D0%B0%D0%BD%D0%B8%D0%B5-%D0%BD%D0%B0-%D0%BF%D1%80%D0%BE%D0%B2%D0%B5%D1%80%D0%BA%D1%83).
