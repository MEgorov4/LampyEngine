import shutil
import sys
import os

def CleanUp():
    script_dir = os.path.abspath(os.path.dirname(__file__))  
    print("script dir")
    folder_path = script_dir + "\\..\\build"
    print(folder_path)
    if os.path.exists(folder_path):
        shutil.rmtree(folder_path)
        print(f"Deleted folder: {folder_path}")
    os.mkdir(folder_path)
    print(f"Created folder: {folder_path}")

def Format():
    input_folder = os.path.abspath(os.path.dirname(__file__)) + "\\..\\Engine"
    output_file = os.path.join(os.path.abspath(os.path.dirname(__file__)), "..", "build", "all_code.txt")

    with open(output_file, 'w') as txt_file:
        for root, _, files in os.walk(input_folder):
            for file in files:
                if file.endswith(".cpp") or file.endswith(".h") or file.endswith(".txt"):
                    file_path = os.path.join(root, file)

                    txt_file.write(f"=== {file} ===\n\n")

                    with open(file_path, 'r') as f:
                        content = f.read()
                        txt_file.write(content)

                    txt_file.write("\n\n")

    print(f"Generated TXT file: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        function_name = sys.argv[1]
        if function_name == "-c":
            CleanUp()
        elif function_name == "-gmd":
            Format()
        else:
            print("Invalid argument")
    else:
        print("Arguments not declared")
