# SL Project Manager (slpm)

A project manager for the SL programming language.

## Features

- Create new SL projects with proper structure
- Build projects using the SL compiler (slc)
- Run executable projects
- Manage project configuration
- Support for both executables and libraries

## Installation

```bash
cd slpm
make
sudo make install
```

## Usage

### Create a new project

```bash
# Create executable project
slpm new my_app

# Create library project
slpm new my_lib --library
```

### Build and run

```bash
cd my_app
slpm build    # Build the project
slpm run      # Build and run the project
```

### List projects

```bash
slpm list     # List all projects in current directory
```

### Project structure

```
my_project/
├── slpm.json          # Project configuration
├── src/
│   ├── main.sl        # Main source file
│   └── lib.sl         # Library functions
├── include/           # Header files (for libraries)
├── tests/             # Test files
└── build/             # Build artifacts
```

### Configuration (slpm.json)

```json
{
  "name": "my_project",
  "version": "0.1.0",
  "type": "executable",
  "output_path": "my_project",
  "debug": false,
  "shared_lib": false,
  "source_files": ["src/main.sl"],
  "dependencies": []
}
```

## Commands

- `slpm new <name>` - Create new project
- `slpm remove <path>` - Remove project
- `slpm build` - Build current project
- `slpm run` - Build and run current project
- `slpm list` - List projects in directory
- `slpm help` - Show help