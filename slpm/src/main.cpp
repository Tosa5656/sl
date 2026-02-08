#include "../include/project_manager.h"
#include <iostream>
#include <string>
#include <vector>

void print_usage() {
    std::cout << "SL Project Manager (slpm) v0.1.0" << std::endl;
    std::cout << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "    slpm <COMMAND> [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "COMMANDS:" << std::endl;
    std::cout << "    new <NAME> [PATH]    Create a new SL project" << std::endl;
    std::cout << "    remove <PATH>        Remove a project" << std::endl;
    std::cout << "    build                 Build the current project" << std::endl;
    std::cout << "    run                   Build and run the current project" << std::endl;
    std::cout << "    list                  List projects in current directory" << std::endl;
    std::cout << "    help                  Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "    slpm new my_app" << std::endl;
    std::cout << "    slpm new my_lib --library" << std::endl;
    std::cout << "    slpm build" << std::endl;
    std::cout << "    slpm run" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    ProjectManager pm;

    if (command == "new" || command == "create") {
        if (argc < 3) {
            std::cerr << "Error: Project name required" << std::endl;
            std::cout << "Usage: slpm new <NAME> [--library]" << std::endl;
            return 1;
        }

        std::string project_name = argv[2];
        bool is_library = (argc >= 4 && std::string(argv[3]) == "--library");

        if (!pm.create_project(project_name)) {
            return 1;
        }

        if (is_library) {
            fs::path full_path = pm.get_default_project_path(project_name);
            ProjectConfig config = ProjectConfig::from_json(full_path / "slpm.json");
            config.type = "library";
            config.output_path = "lib" + project_name + ".a";
            config.to_json(full_path / "slpm.json");
        }

    } else if (command == "remove" || command == "rm") {
        if (argc < 3) {
            std::cerr << "Error: Project path required" << std::endl;
            std::cout << "Usage: slpm remove <PATH>" << std::endl;
            return 1;
        }

        fs::path project_path = argv[2];
        if (!pm.remove_project(project_path)) {
            return 1;
        }

    } else if (command == "build") {
        fs::path project_path = pm.find_project_root();
        if (project_path.empty()) {
            std::cerr << "Error: Not in a SL project directory" << std::endl;
            std::cerr << "Run 'slpm new <name>' to create a project first" << std::endl;
            return 1;
        }

        if (!pm.build_project(project_path)) {
            return 1;
        }

    } else if (command == "run") {
        fs::path project_path = pm.find_project_root();
        if (project_path.empty()) {
            std::cerr << "Error: Not in a SL project directory" << std::endl;
            return 1;
        }

        if (!pm.build_project(project_path) || !pm.run_project(project_path)) {
            return 1;
        }

    } else if (command == "list" || command == "ls") {
        pm.list_projects();

    } else if (command == "help" || command == "--help" || command == "-h") {
        print_usage();

    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        print_usage();
        return 1;
    }

    return 0;
}