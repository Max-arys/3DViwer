[3dviewer.net онлайн просмотр 3д моделей](https://3dviewer.net/index.html)

Фигуры:  
  - [aple_476](https://raw.githubusercontent.com/polings/3DViewer/refs/heads/main/src/objFiles/apple.obj)
  - [teapot_3644](https://raw.githubusercontent.com/polings/3DViewer/refs/heads/main/src/objFiles/teapot.obj)
  - [bluewhale_49826](https://raw.githubusercontent.com/polings/3DViewer/refs/heads/main/src/objFiles/bluewhale.obj)
  - [bulldog_189602](https://raw.githubusercontent.com/AlexeyVarv/CPP4-3DViewer2.0/refs/heads/main/data-samples/French%20Bulldog.obj)

Книги и сайты:
- [Head First. Паттерны проектирования.](https://djvu.online/file/fst3hXd58kYyC)
- [Банда четырех. Паттерны объектно-ориентированного проектирования](https://djvu.online/file/gZWzWB2QKQckM?ysclid=mnhhl1n9s949967733)
- [refactoringu](https://refactoringu.ru/ru/design-patterns/catalog.html)


### Зависимости
```bash
sudo apt install \
make \
cmake \
qt6-base-dev \
qt6-tools-dev \
libassimp-dev \
libgif-dev \
-y
```

###  Подключение заголовочных файлов от корня src/
Описание
В проекте реализован механизм подключения заголовочных файлов через пути относительно корневой директории исходного кода (src/).
Это заменяет использование относительных путей (../model/..., ../../view/...) на однозначные и устойчивые к перемещениям
конструкции вида #include "model/model.h". `target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)`

### Паттерны и структура проекта

#### Паттерн MVC  
- Model отвечает за бизнес процессы приложения:  
Загрузка модели фигуры из файла .obj. Для парсинга данных используется библиотека `assimp`.  
Далее загруженная модель центруется, что бы пересечение мировых координат было примерно в центре фигуры. Далее сама модель больше не изменяется. Для заданных параметров вычисляется положение фигуры в пространстве.
Реализован поворот фигуры вокруг всех трех осей относительно мировых координат.
- Controller  
Controller получает данные управления из View и вызывает сответствующие методы Model. Если в Model есть изменения то Controller перенаправит эти изменения в View  
- View  
Отвечает за графическую составляющую и пользовательский ввод(только принимает его, а обработкой занимается Controller)


#### Паттерн Наблюдатель (Observer)  
- Model выступает в роли наблюдаемого объекта и генерирует сигналы при изменении своего состояния.
- View выступает в роли наблюдаемого объекта.
- Controller наблюдает за Model подключаясь к ее сигналам, а так же он наблюдает за View(получает сигналы от панели управления)


### Рефакторинг 

Планируемая структура проекта:

3DViewer/                         # Корень проекта (исправлено название)
├── Makefile
├── .clang-format                 # Правила форматирования кода
├── .gitignore                    
├── CMakeLists.txt                # Главный CMake
├── README.md
├── scripts/                      # Вспомогательные скрипты
│   ├── build_deb.sh
│   ├── check_format.sh
│   └── generate_docs.sh
├── cmake/                        # CMake модули (Find*, конфиги)
│   └── CompilerOptions.cmake
├── resources/                    # Ресурсы времени выполнения
│   ├── shaders/                  # GLSL шейдеры
│   │   ├── vertex.glsl
│   │   └── fragment.glsl
│   ├── icons/                    # .png / .svg
│   └── models/                   # Тестовые .obj файлы
├── src/                          # Исходный код (Source)
│   ├── app/                      # Точка входа приложения и основное окно
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   ├── MainWindow.cpp
│   │   ├── MainWindow.h
│   │   └── ApplicationSettings.cpp (бывший display_settings)
│   ├── core/                     # Бизнес-логика и доменная модель (бывший model)
│   │   ├── CMakeLists.txt
│   │   ├── Scene.h
│   │   ├── Scene.cpp
│   │   ├── Model3D.h             # Переименовано из model.h (избегаем путаницы)
│   │   ├── Model3D.cpp
│   │   ├── Transform3D.h
│   │   └── Transform3D.cpp
│   ├── rendering/                # Слой View (OpenGL специфика)
│   │   ├── CMakeLists.txt
│   │   ├── GLWidget.h
│   │   ├── GLWidget.cpp
│   │   ├── Camera.h
│   │   ├── Camera.cpp
│   │   ├── ShaderProgram.h       # Обертка над OpenGL шейдерами
│   │   └── ShaderProgram.cpp
│   ├── io/                       # Вспомогательный ввод/вывод
│   │   ├── CMakeLists.txt
│   │   ├── ObjReader.h           # Парсер .obj (должен быть отделен от Model3D)
│   │   ├── ObjReader.cpp
│   │   ├── GifRecorder.h
│   │   ├── GifRecorder.cpp
│   │   ├── ImageSaver.h
│   │   └── ImageSaver.cpp
│   └── controller/               # Связующий слой (Controller)
│       ├── CMakeLists.txt
│       ├── SceneController.h     # Управляет связью MainWindow <-> Scene <-> GLWidget
│       └── SceneController.cpp
└── tests/                        # Модульные тесты (рядом с src, а не внутри)
├── CMakeLists.txt
├── mocks/                    # Моки для OpenGL
└── unit/
├── TestTransform.cpp
└── TestObjReader.cpp


- добавить CI/CD
- Dockerfile для сборки под разные ОС и тестирования


