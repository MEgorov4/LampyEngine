# Game Engine (C++ / OpenGL)

Учебный движок с модульной архитектурой и паттернами проектирования.  
Рендер на OpenGL, сборка через CMake, код в стиле SOLID с разделением логики и рендера.  

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
## [Документация](https://megorov4.github.io/LampyEngine/)

## Демо
![image](https://github.com/user-attachments/assets/491758a3-1f7c-46d7-9091-c5e7c2d9155d)
![PhysicsTest](https://github.com/user-attachments/assets/04693b79-ca1d-4186-8109-2a8f205ee7e7)
