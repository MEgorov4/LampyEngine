#pragma once

#include <EngineMinimal.h>

namespace Events::EditorUI
{
    // ============================================================================
    // Simulation Events
    // ============================================================================
    
    /// Запуск симуляции ECS
    struct SimulationStart {};

    /// Остановка симуляции ECS
    struct SimulationStop {};

    /// Запросить текущее состояние симуляции
    struct SimulationStatusRequest {};

    /// Ответ на запрос статуса симуляции
    struct SimulationStatusResponse
    {
        bool isRunning;
    };

    // ============================================================================
    // World Management Events
    // ============================================================================
    
    /// Сохранить текущий мир
    struct WorldSaveRequest
    {
        std::string filePath;  // Пустой означает использовать текущий путь
    };

    /// Сохранить мир как новый файл
    struct WorldSaveAsRequest
    {
        std::string filePath;
    };

    /// Загрузить мир из файла
    struct WorldLoadRequest
    {
        std::string filePath;
    };

    /// Мир успешно сохранен
    struct WorldSaved
    {
        std::string filePath;
    };

    /// Мир успешно загружен
    struct WorldLoaded
    {
        std::string filePath;
    };

    // ============================================================================
    // File System Events
    // ============================================================================
    
    /// Создать папку
    struct FolderCreateRequest
    {
        std::string parentPath;
        std::string folderName;
    };

    /// Создать файл
    struct FileCreateRequest
    {
        std::string parentPath;
        std::string fileName;
    };

    /// Удалить файл
    struct FileDeleteRequest
    {
        std::string filePath;
    };

    /// Удалить папку
    struct FolderDeleteRequest
    {
        std::string folderPath;
    };

    /// Скопировать файл
    struct FileCopyRequest
    {
        std::string sourcePath;
        std::string destinationPath;
    };

    /// Переместить файл
    struct FileMoveRequest
    {
        std::string sourcePath;
        std::string destinationPath;
    };

    /// Дублировать файл
    struct FileDuplicateRequest
    {
        std::string filePath;
    };

    /// Файловая операция успешно выполнена
    struct FileOperationCompleted
    {
        std::string operation;  // "create", "delete", "copy", "move", "duplicate"
        std::string filePath;
        bool success;
    };

    // ============================================================================
    // Content Browser Events
    // ============================================================================
    
    /// Открыть файл в Content Browser
    struct FileOpenRequest
    {
        std::string filePath;
    };

    /// Выбрать файл в Content Browser
    struct FileSelected
    {
        std::string filePath;
    };

    /// Выбрать папку в Content Browser
    struct FolderSelected
    {
        std::string folderPath;
    };

    /// Навигация по директориям изменена
    struct DirectoryNavigated
    {
        std::string currentPath;
        std::string relativePath;
    };

    // ============================================================================
    // Entity Management Events
    // ============================================================================
    
    /// Создать новую сущность
    struct EntityCreateRequest
    {
        std::string entityName;
        bool withDefaultComponents = true;  // Добавить Position, Rotation, Scale
    };

    /// Удалить сущность
    struct EntityDeleteRequest
    {
        uint64_t entityId;
    };

    /// Выбрать сущность в инспекторе
    struct EntitySelected
    {
        uint64_t entityId;
    };

    /// Снять выделение с сущности
    struct EntityDeselected {};

    /// Добавить компонент к сущности
    struct ComponentAddRequest
    {
        uint64_t entityId;
        std::string componentTypeName;  // Имя типа компонента, например "MeshComponent"
    };

    /// Компонент добавлен к сущности
    struct ComponentAdded
    {
        uint64_t entityId;
        std::string componentName;
    };

    /// Удалить компонент из сущности
    struct ComponentRemoveRequest
    {
        uint64_t entityId;
        std::string componentTypeName;  // Имя типа компонента, например "MeshComponent"
    };

    /// Компонент удален из сущности
    struct ComponentRemoved
    {
        uint64_t entityId;
        std::string componentName;
    };

    /// Сбросить компонент к дефолтным значениям
    struct ComponentResetRequest
    {
        uint64_t entityId;
        std::string componentTypeName;  // Имя типа компонента
    };

    // ============================================================================
    // Project Settings Events
    // ============================================================================
    
    /// Установить стартовый мир проекта
    struct ProjectStartWorldSet
    {
        std::string worldPath;
    };

    // ============================================================================
    // Main Menu Events
    // ============================================================================
    
    /// Открыть проект/файл
    struct MenuFileOpenRequest {};

    /// Сохранить
    struct MenuFileSaveRequest {};

    /// Выход из приложения
    struct MenuFileExitRequest {};

    /// Отменить действие
    struct MenuEditUndoRequest {};

    /// Повторить действие
    struct MenuEditRedoRequest {};

    /// Показать About диалог
    struct MenuHelpAboutRequest {};
}
