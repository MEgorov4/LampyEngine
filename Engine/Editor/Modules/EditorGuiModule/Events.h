#pragma once

#include <EngineMinimal.h>

namespace Events::EditorUI
{
    // ============================================================================
    // Simulation Events
    // ============================================================================
    
    struct SimulationStart {};

    struct SimulationStop {};

    struct SimulationStatusRequest {};

    struct SimulationStatusResponse
    {
        bool isRunning;
    };

    // ============================================================================
    // World Management Events
    // ============================================================================
    
    struct WorldSaveRequest
    {
        std::string filePath;
    };

    struct WorldSaveAsRequest
    {
        std::string filePath;
    };

    struct WorldLoadRequest
    {
        std::string filePath;
    };

    struct WorldSaved
    {
        std::string filePath;
    };

    struct WorldLoaded
    {
        std::string filePath;
    };

    // ============================================================================
    // File System Events
    // ============================================================================
    
    struct FolderCreateRequest
    {
        std::string parentPath;
        std::string folderName;
    };

    struct FileCreateRequest
    {
        std::string parentPath;
        std::string fileName;
    };

    struct FileDeleteRequest
    {
        std::string filePath;
    };

    struct FolderDeleteRequest
    {
        std::string folderPath;
    };

    struct FileCopyRequest
    {
        std::string sourcePath;
        std::string destinationPath;
    };

    struct FileMoveRequest
    {
        std::string sourcePath;
        std::string destinationPath;
    };

    struct FileDuplicateRequest
    {
        std::string filePath;
    };

    struct FileOperationCompleted
    {
        std::string operation;  // "create", "delete", "copy", "move", "duplicate"
        std::string filePath;
        bool success;
    };

    // ============================================================================
    // Content Browser Events
    // ============================================================================
    
    struct FileOpenRequest
    {
        std::string filePath;
    };

    struct FileSelected
    {
        std::string filePath;
    };

    struct FolderSelected
    {
        std::string folderPath;
    };

    struct DirectoryNavigated
    {
        std::string currentPath;
        std::string relativePath;
    };

    // ============================================================================
    // Entity Management Events
    // ============================================================================
    
    struct EntityCreateRequest
    {
        std::string entityName;
        bool withDefaultComponents = true;
    };

    struct EntityDeleteRequest
    {
        uint64_t entityId;
    };

    struct EntitySelected
    {
        uint64_t entityId;
    };

    struct EntityDeselected {};

    struct ComponentAddRequest
    {
        uint64_t entityId;
        std::string componentTypeName;
    };

    struct ComponentAdded
    {
        uint64_t entityId;
        std::string componentName;
    };

    struct ComponentRemoveRequest
    {
        uint64_t entityId;
        std::string componentTypeName;
    };

    struct ComponentRemoved
    {
        uint64_t entityId;
        std::string componentName;
    };

    struct ComponentResetRequest
    {
        uint64_t entityId;
        std::string componentTypeName;
    };

    // ============================================================================
    // Project Settings Events
    // ============================================================================
    
    struct ProjectStartWorldSet
    {
        std::string worldPath;
    };

    enum class WorldDefaultTarget
    {
        Editor,
        Game
    };

    struct WorldDefaultChanged
    {
        std::string worldPath;
        WorldDefaultTarget target = WorldDefaultTarget::Editor;
    };

    // ============================================================================
    // Main Menu Events
    // ============================================================================
    
    struct MenuFileOpenRequest {};

    struct MenuFileSaveRequest {};

    struct MenuFileExitRequest {};

    struct MenuEditUndoRequest {};

    struct MenuEditRedoRequest {};

    struct MenuHelpAboutRequest {};
}
