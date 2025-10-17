# Game Engine (C++ / OpenGL)

Учебный движок с DI-модульной архитектурой.  
Рендер на OpenGL, сборка через CMake + Conan.  

## Технологии
[![GLEW 2.2.0](https://img.shields.io/badge/GLEW-2.2.0-5586A4?logo=opengl)](https://github.com/nigels-com/glew)
[![SDL 3.2.14](https://img.shields.io/badge/SDL-3.2.14-0A7BBB?logo=sdl&logoColor=white)](https://libsdl.org/)
[![ImGui 1.92.0](https://img.shields.io/badge/ImGui-1.92.0-5A5A5A?logo=imgui)](https://github.com/ocornut/imgui)
[![GLM cci.20230113](https://img.shields.io/badge/GLM-cci.20230113-2D3748)](https://github.com/g-truc/glm)
[![nlohmann/json 3.11.3](https://img.shields.io/badge/nlohmann__json-3.11.3-0F6CBD)](https://github.com/nlohmann/json)
[![tinyobjloader 2.0.0-rc10](https://img.shields.io/badge/tinyobjloader-2.0.0--rc10-3C3C3C)](https://github.com/tinyobjloader/tinyobjloader)
[![portable-file-dialogs 0.1.0](https://img.shields.io/badge/pfd-0.1.0-4B5563)](https://github.com/samhocevar/portable-file-dialogs)
[![flecs 4.0.4](https://img.shields.io/badge/flecs-4.0.4-0E7490)](https://github.com/SanderMertens/flecs)
[![Boost 1.86.0](https://img.shields.io/badge/Boost-1.86.0-5E2750?logo=boost&logoColor=white)](https://www.boost.org/)
[![sol2 3.3.1](https://img.shields.io/badge/sol2-3.3.1-8A2BE2)](https://github.com/ThePhD/sol2)
[![stb cci.20240531](https://img.shields.io/badge/stb-cci.20240531-222222)](https://github.com/nothings/stb)
[![clip 1.9](https://img.shields.io/badge/clip-1.9-2F855A)](https://github.com/dacap/clip)
[![Bullet 3.25](https://img.shields.io/badge/Bullet-3.25-BF1F2F)](https://github.com/bulletphysics/bullet3)
[![GoogleTest 1.15.0](https://img.shields.io/badge/GoogleTest-1.15.0-4285F4?logo=googletest&logoColor=white)](https://github.com/google/googletest)

- **Графика:** OpenGL 4.5 (GLEW), SDL3 (окно/ввод), GLM, Dear ImGui.
- **ECS:** Flecs.
- **Физика:** Bullet3.
- **Скрипты:** Lua (sol2).
- **Ресурсы:** stb_image, tinyobjloader, nlohmann_json.
- **Прочее:** clip (буфер обмена), Boost.Process, GoogleTest.

## Особенности проекта
- 🧩 **ECS (Entity-Component-System)** — для организации игровой логики.  
- 🏗 **Модульная архитектура** — все подсистемы как отдельные модули с единым жизненным циклом.  
- 🛠 **Паттерны проектирования**:
  - Strategy — рендер-пайплайн и проходы рендера.  
  - Factory / Abstract Factory — создание ресурсов и действий редактора.  
  - Observer — событийная система (ECS, окно, ввод, логгер).  
  - Registry — хранение модулей и фабрик.  
  - Bridge — абстракция IRenderer и реализация OpenGLRenderer.  
  - Singleton — глобальные конфиги и регистры.  
  - Template Method — общий алгоритм рендера с делегированием шагов.  
  - Flyweight / Cache — хранение и переиспользование ресурсов.  
- 🎨 Разделение core-логики и рендера.  
- 📂 Чистая структура каталогов и сборка через CMake и Conan.  
- 📄 Doxygen-документация.  
- 📑 ClangFormat для выравнивания кода.

```
Engine/
├── core/ # Логика ECS, управление миром
├── render/ # Абстракция рендера, OpenGL реализация
├── resources/ # Кэш и фабрики ресурсов
├── editor/ # UI и действия контент-браузера
├── modules/ # Модули движка
├── tests/ # Unit-тесты
├── CMakeLists.txt
└── README.md
```
### 
[Project Docs]()
[Code Docs](https://megorov4.github.io/LampyEngine/)

## Демо
![image](https://github.com/user-attachments/assets/491758a3-1f7c-46d7-9091-c5e7c2d9155d)
![PhysicsTest](https://github.com/user-attachments/assets/04693b79-ca1d-4186-8109-2a8f205ee7e7)
