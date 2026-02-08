#include "../include/project_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <regex>
#include <algorithm>

// For JSON handling (simple implementation)
#include <map>

ProjectManager::ProjectManager() {
    // Constructor
}

ProjectManager::~ProjectManager() {
    // Destructor
}

bool ProjectManager::create_project(const std::string& name, const fs::path& path) {
    fs::path project_path = path.empty() ? get_default_project_path(name) : path / name;

    if (fs::exists(project_path)) {
        print_error("Project '" + name + "' already exists at " + project_path.string());
        return false;
    }

    try {
        create_project_structure(project_path);
        create_default_config(project_path, name);
        create_default_source_files(project_path, name);

        print_success("Created project '" + name + "' at " + project_path.string());
        print_info("Use 'cd " + project_path.string() + " && slpm build' to build the project");
        return true;
    } catch (const std::exception& e) {
        print_error("Failed to create project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::remove_project(const fs::path& project_path) {
    if (!is_valid_project(project_path)) {
        print_error("Not a valid SL project: " + project_path.string());
        return false;
    }

    try {
        fs::remove_all(project_path);
        print_success("Removed project at " + project_path.string());
        return true;
    } catch (const std::exception& e) {
        print_error("Failed to remove project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::build_project(const fs::path& project_path) {
    if (!is_valid_project(project_path)) {
        print_error("Not a valid SL project: " + project_path.string());
        return false;
    }

    try {
        ProjectConfig config = ProjectConfig::from_json(project_path / "slpm.json");

        // Build command based on project type
        std::string build_cmd;
        if (config.type == "library") {
            if (config.shared_lib) {
                build_cmd = "slc src/lib.sl -shared " + config.output_path;
            } else {
                build_cmd = "slc src/lib.sl -static " + config.output_path;
            }
        } else {
            build_cmd = "slc src/main.sl " + config.output_path;
        }

        if (run_command(build_cmd, project_path)) {
            print_success("Built project '" + config.name + "'");
            return true;
        } else {
            print_error("Build failed for project '" + config.name + "'");
            return false;
        }
    } catch (const std::exception& e) {
        print_error("Build failed: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::run_project(const fs::path& project_path) {
    if (!is_valid_project(project_path)) {
        print_error("Not a valid SL project: " + project_path.string());
        return false;
    }

    ProjectConfig config = ProjectConfig::from_json(project_path / "slpm.json");

    if (config.type == "library") {
        print_error("Cannot run a library project. Use 'slpm build' instead.");
        return false;
    }

    std::string run_cmd = "./" + config.output_path;
    return run_command(run_cmd, project_path);
}

fs::path ProjectManager::find_project_root(const fs::path& start_path) {
    fs::path current = start_path;
    while (current != current.parent_path()) {
        if (fs::exists(current / "slpm.json")) {
            return current;
        }
        current = current.parent_path();
    }
    return fs::path(); // Empty path if not found
}

bool ProjectManager::is_valid_project(const fs::path& project_path) {
    return fs::exists(project_path / "slpm.json") &&
           fs::exists(project_path / "src");
}

void ProjectManager::list_projects() {
    fs::path current_dir = fs::current_path();
    bool found_any = false;

    for (const auto& entry : fs::directory_iterator(current_dir)) {
        if (entry.is_directory() && is_valid_project(entry.path())) {
            ProjectConfig config = ProjectConfig::from_json(entry.path() / "slpm.json");
            std::cout << "📁 " << config.name << " (" << config.type << ") - " << entry.path().filename().string() << std::endl;
            found_any = true;
        }
    }

    if (!found_any) {
        print_info("No SL projects found in current directory");
    }
}

// Private methods

fs::path ProjectManager::get_default_project_path(const std::string& name) const {
    return fs::current_path() / name;
}

void ProjectManager::create_project_structure(const fs::path& project_path) {
    fs::create_directories(project_path / "src");
    fs::create_directories(project_path / "include");
    fs::create_directories(project_path / "tests");
    fs::create_directories(project_path / "build");
}

void ProjectManager::create_default_config(const fs::path& project_path, const std::string& name) {
    ProjectConfig config;
    config.name = name;
    config.version = "0.1.0";
    config.type = "executable";
    config.output_path = name;
    config.source_files = {"src/main.sl"};
    config.debug = false;

    config.to_json(project_path / "slpm.json");
}

void ProjectManager::create_default_source_files(const fs::path& project_path, const std::string& name) {
    // Create main.sl
    std::ofstream main_file(project_path / "src" / "main.sl");
    main_file << generate_main_sl(name);
    main_file.close();

    // Create lib.sl for potential library use
    std::ofstream lib_file(project_path / "src" / "lib.sl");
    lib_file << generate_lib_sl(name);
    lib_file.close();

    // Create basic test
    std::ofstream test_file(project_path / "tests" / "basic_test.sl");
    test_file << "function main() -> int {\n    return 42;\n}\n";
    test_file.close();
}

std::string ProjectManager::generate_main_sl(const std::string& project_name) {
    std::stringstream ss;
    ss << "// " << project_name << " - Main entry point\n";
    ss << "\n";
    ss << "function main() -> int {\n";
    ss << "    return 0;\n";
    ss << "}\n";
    return ss.str();
}

std::string ProjectManager::generate_lib_sl(const std::string& project_name) {
    std::stringstream ss;
    ss << "// " << project_name << " - Library functions\n";
    ss << "\n";
    ss << "function add_numbers(int a, int b) -> int {\n";
    ss << "    return a + b;\n";
    ss << "}\n";
    ss << "\n";
    ss << "function multiply_numbers(int a, int b) -> int {\n";
    ss << "    return a * b;\n";
    ss << "}\n";
    return ss.str();
}

bool ProjectManager::run_command(const std::string& cmd, const fs::path& cwd) {
    std::string full_cmd = "cd " + cwd.string() + " && " + cmd;
    int result = std::system(full_cmd.c_str());
    return result == 0;
}

void ProjectManager::print_success(const std::string& message) {
    std::cout << "✅ " << message << std::endl;
}

void ProjectManager::print_error(const std::string& message) {
    std::cerr << "❌ " << message << std::endl;
}

void ProjectManager::print_info(const std::string& message) {
    std::cout << "ℹ️  " << message << std::endl;
}

// JSON serialization implementation (simple)
void ProjectConfig::to_json(const fs::path& config_path) const {
    std::ofstream file(config_path);
    file << "{\n";
    file << "  \"name\": \"" << name << "\",\n";
    file << "  \"version\": \"" << version << "\",\n";
    file << "  \"type\": \"" << type << "\",\n";
    file << "  \"output_path\": \"" << output_path << "\",\n";
    file << "  \"debug\": " << (debug ? "true" : "false") << ",\n";
    file << "  \"shared_lib\": " << (shared_lib ? "true" : "false") << ",\n";
    file << "  \"source_files\": [";
    for (size_t i = 0; i < source_files.size(); ++i) {
        file << "\"" << source_files[i] << "\"";
        if (i < source_files.size() - 1) file << ",";
    }
    file << "],\n";
    file << "  \"dependencies\": [";
    for (size_t i = 0; i < dependencies.size(); ++i) {
        file << "\"" << dependencies[i] << "\"";
        if (i < dependencies.size() - 1) file << ",";
    }
    file << "]\n";
    file << "}\n";
}

ProjectConfig ProjectConfig::from_json(const fs::path& config_path) {
    ProjectConfig config;
    std::ifstream file(config_path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Simple JSON parsing (very basic)
    std::regex name_regex("\"name\"\\s*:\\s*\"([^\"]+)\"");
    std::regex version_regex("\"version\"\\s*:\\s*\"([^\"]+)\"");
    std::regex type_regex("\"type\"\\s*:\\s*\"([^\"]+)\"");
    std::regex output_regex("\"output_path\"\\s*:\\s*\"([^\"]+)\"");

    std::smatch match;
    if (std::regex_search(content, match, name_regex)) {
        config.name = match[1];
    }
    if (std::regex_search(content, match, version_regex)) {
        config.version = match[1];
    }
    if (std::regex_search(content, match, type_regex)) {
        config.type = match[1];
    }
    if (std::regex_search(content, match, output_regex)) {
        config.output_path = match[1];
    }

    return config;
}