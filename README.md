# Zyncos Manual Mapper

## Overview
Zyncos Manual Mapper is a Windows-based DLL injection tool that allows users to manually inject DLLs into target processes. It provides a simple and efficient way to load dynamic link libraries into running applications.

## Features
- Manual DLL selection via file dialog
- Target process selection by name
- Memory allocation and remote thread creation
- Console-based UI with color-coded feedback
- Automatic console positioning and resizing

## Requirements
- Windows operating system (64-bit recommended)
- Administrator privileges
- C++ compiler with Windows API support


## Function Explanations

### `ConsoleColor(WORD attributes)`
Changes the console text color to the specified attributes.

### `SetConsoleWindowSize(int width, int height)`
Resizes the console window to the specified width and height.

### `CenterConsoleWindow()`
Centers the console window on the screen and applies transparency effects.

### `DWORD GetProcessIdByName(const std::wstring& processName)`
Retrieves the process ID of a running application by its executable name.

### `std::wstring OpenFileDialog()`
Opens a file selection dialog to allow the user to choose a DLL file for injection.

### `bool InjectDLL(DWORD processId, const std::wstring& dllPath)`
Handles the DLL injection process:
1. Opens the target process.
2. Allocates memory in the target process.
3. Writes the DLL path into the allocated memory.
4. Retrieves the `LoadLibraryW` function address from `Kernel32.dll`.
5. Creates a remote thread to execute `LoadLibraryW`, loading the DLL into the process.
6. Cleans up allocated memory and handles.

### `int main()`
The main function orchestrates the workflow:
1. Sets the console title, size, and position.
2. Prompts the user to select a DLL file.
3. Prompts the user to enter the target process name.
4. Retrieves the process ID and attempts injection.
5. Displays appropriate messages based on success or failure.

## Usage

### Step 1: Compile the Injector
To compile the injector, ensure you have a C++ compiler that supports Windows API functions. You can use Microsoft Visual Studio with the following settings:

1. Open Visual Studio and create a new C++ console application.
2. Add the provided source code.
3. Set the project to compile as a Windows application.
4. Build the project to generate the executable.

### Step 2: Run the Injector
1. **Execute** `Manual Mapper.exe` as an administrator.
2. **Select the DLL** you want to inject via the file dialog.
3. **Enter the target process name** (e.g., `notepad.exe`).
4. The injector will attempt to allocate memory, write the DLL path, and create a remote thread to load the library.
5. If successful, a message will confirm the injection.

### Step 3: Verify Injection
- Use tools like Process Explorer or Cheat Engine to verify that the DLL has been successfully injected into the target process.

## Troubleshooting
- **"Failed to open process"** → Ensure the target process is running and you have the necessary permissions.
- **"Failed to allocate memory"** → The target process may have protections preventing memory allocation.
- **"Process not found"** → Double-check the process name (including the `.exe` extension).

## Disclaimer
This tool is for educational and research purposes only. The author is not responsible for any misuse or damage caused by this software.

## License
MIT License
