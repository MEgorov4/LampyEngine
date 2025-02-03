#include <iostream>
#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>

std::string projectPath = "";
std::string projectName = "";

void save_data_on_result_file(const nlohmann::json& data)
{
    std::ofstream file("projectBrowserResult.json");
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to save configuration file");
    }
    file << data;
    file.close();
}
void on_file_select(Fl_Widget*, void*) {
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
            save_data_on_result_file(jsonData);
            exit(0);
        }
    }
}

void on_folder_select(Fl_Widget*, void*) {
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

            file.close();
        }
        exit(0);
    }
}

void on_exit(Fl_Widget*, void*) {
    if (fl_choice("Are you shure?", "No", "Yes", nullptr) == 1) {
        nlohmann::json jsonData;
        jsonData["result"] = "exit";
        save_data_on_result_file(jsonData);
        exit(0);
    }
}

int main() {
    try {
        int xWidth = 300;
        int yWidth = 400;
        Fl_Window* window = new Fl_Window(xWidth, yWidth, "Project Browser");
        Fl_Button* file_button = new Fl_Button(0, 0, xWidth, yWidth / 3, "Select project file");
        file_button->callback(on_file_select);

        Fl_Button* folder_button = new Fl_Button(0, yWidth / 3, xWidth, yWidth / 3, "Create project in folder");
        folder_button->callback(on_folder_select);

        Fl_Button* exit_button = new Fl_Button(0, yWidth / 3 * 2, xWidth, yWidth / 3, "exit");
        exit_button->callback(on_exit);

        window->end();
        window->resizable(window); 
        window->show();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Project browser error: " << e.what() << std::endl;
        std::exit(1);
    }

    return Fl::run();
}
