#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>

std::string projectPath = ""; ///< Stores the selected or created project path.
std::string projectName = ""; ///< Stores the name of the new project.

/// <summary>
/// Sends the project data in JSON format to the output stream.
/// </summary>
/// <param name="data">The JSON object containing project details.</param>
void sendDataOnOutStream(const nlohmann::json& data)
{
    std::cout << data;
}

/// <summary>
/// Handles file selection for existing projects.
/// Opens a file chooser dialog to select a `.lproj` project file.
/// </summary>
/// <param name="widget">Pointer to the calling widget (not used).</param>
/// <param name="data">Additional data (not used).</param>
void onFileSelect(Fl_Widget*, void*) {
    Fl_File_Chooser chooser("..", "*.lproj", Fl_File_Chooser::SINGLE, "Choose project file");
    chooser.show();

    while (chooser.visible()) Fl::wait(); 

    if (chooser.value() != nullptr) {
        projectPath = chooser.value();

        std::fstream file(projectPath);
        if (file.is_open())
        {
            nlohmann::json jsonData = nlohmann::json::parse(file);
            jsonData["result"] = "success"; 
            file.close();
            sendDataOnOutStream(jsonData);
            exit(0); 
        }
        exit(1);
    }
}

/// <summary>
/// Handles folder selection for creating a new project.
/// Prompts the user to choose a folder and enter a project name.
/// Creates necessary directories and a `.lproj` configuration file.
/// </summary>
/// <param name="widget">Pointer to the calling widget (not used).</param>
/// <param name="data">Additional data (not used).</param>
void onFolderSelect(Fl_Widget*, void*) {
    Fl_File_Chooser chooser("..", "", Fl_File_Chooser::DIRECTORY, "Choose new project folder");
    chooser.show();

    while (chooser.visible()) Fl::wait(); 

    if (chooser.value() != nullptr) {
        projectPath = chooser.value();

        projectName = fl_input("Type new project name");

        std::string fullProjectFolderPath = projectPath + projectName;
        std::filesystem::create_directory(fullProjectFolderPath);

        std::string projectFilePath = fullProjectFolderPath + "/" + projectName + ".lproj";

        std::ofstream file(projectFilePath);

        if (file.is_open())
        {
            std::filesystem::create_directory(fullProjectFolderPath + "/Resources");
            std::filesystem::create_directory(fullProjectFolderPath + "/Build");
            std::filesystem::create_directory(fullProjectFolderPath + "/Config");
            std::filesystem::create_directory(fullProjectFolderPath + "/Saved");

            nlohmann::json jsonData;
            jsonData["projectPath"] = fullProjectFolderPath;
            jsonData["projectName"] = projectName;
            jsonData["resourcesPath"] = fullProjectFolderPath + "/Resources";
            jsonData["buildPath"] = fullProjectFolderPath + "/Build";
            jsonData["configPath"] = fullProjectFolderPath + "/Config";
            jsonData["logsPath"] = fullProjectFolderPath + "/Saved";
            jsonData["editorStartWorld"] = "default";
            jsonData["gameStartWorld"] = "default";
            jsonData["result"] = "success";
            file << jsonData;
            sendDataOnOutStream(jsonData);
            file.close();
            exit(0);
        }
        exit(1);
    }
}

/// <summary>
/// Handles the exit confirmation dialog.
/// If the user confirms, sends an exit message via JSON and terminates the application.
/// </summary>
/// <param name="widget">Pointer to the calling widget (not used).</param>
/// <param name="data">Additional data (not used).</param>
void onExit(Fl_Widget*, void*) {
    if (fl_choice("Are you sure?", "No", "Yes", nullptr) == 1) {
        nlohmann::json jsonData;
        jsonData["result"] = "exit";
        sendDataOnOutStream(jsonData);
        exit(1);
    }
}

/// <summary>
/// Main function that initializes the FLTK GUI and handles user input.
/// </summary>
/// <returns>0 on successful execution.</returns>
int main() {
    try {
        int xWidth = 300;
        int yWidth = 400;
        Fl_Window* window = new Fl_Window(xWidth, yWidth, "Project Browser");
        Fl_Button* file_button = new Fl_Button(0, 0, xWidth, yWidth / 3, "Select project file");
        file_button->callback(onFileSelect);

        Fl_Button* folder_button = new Fl_Button(0, yWidth / 3, xWidth, yWidth / 3, "Create project in folder");
        folder_button->callback(onFolderSelect);

        Fl_Button* exit_button = new Fl_Button(0, yWidth / 3 * 2, xWidth, yWidth / 3, "Exit");
        exit_button->callback(onExit);

        window->end();
        window->show();

        window->callback(onExit);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Project browser error: " << e.what() << std::endl;
        nlohmann::json jsonData;
        jsonData["result"] = "fail";
        sendDataOnOutStream(jsonData);

        std::exit(1);
    }

    return Fl::run();
}
