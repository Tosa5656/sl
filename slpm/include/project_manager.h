#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct ProjectConfig {
    std::string name;
    std::string version;
    std::string type; // "executable" or "library"
    std::string output_path;
    std::vector<std::string> source_files;
    std::vector<std::string> dependencies;
    bool debug = false;
    bool shared_lib = false;

    // Serialization
    void to_json(const fs::path& config_path) const;
    static ProjectConfig from_json(const fs::path& config_path);
};

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    // Project operations
    bool create_project(const std::string& name, const fs::path& path = "");
    bool remove_project(const fs::path& project_path);
    bool build_project(const fs::path& project_path);
    bool run_project(const fs::path& project_path);

    // Utility functions
    fs::path find_project_root(const fs::path& start_path = fs::current_path());
    bool is_valid_project(const fs::path& project_path);
    void list_projects();

public:
    fs::path get_default_project_path(const std::string& name) const;

private:
        void create_project_structure(const fs::path& project_path);
    void create_default_config(const fs::path& project_path, const std::string& name);
    void create_default_source_files(const fs::path& project_path, const std::string& name);
    std::string generate_main_sl(const std::string& project_name);
    std::string generate_lib_sl(const std::string& project_name);

    // Helper methods
    bool run_command(const std::string& cmd, const fs::path& cwd = fs::current_path());
    void print_success(const std::string& message);
    void print_error(const std::string& message);
    void print_info(const std::string& message);
};

#endif // PROJECT_MANAGER_H