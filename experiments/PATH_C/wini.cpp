#include <iostream>
#include <string>
#include <cstdlib>

// Stub implementations matching the GIR semantics
template<typename... Args>
void emit_popup(const std::string &title,
                const std::string &message,
                const std::string &button_label) {
    std::cout << "=== POPUP ===\n";
    if (!title.empty()) std::cout << title << "\n";
    std::cout << message << "\n";
    std::cout << "[" << button_label << "]\n";
    std::cout << "=============\n\n";
}

template<typename T>
void execute_system_command(const std::string &cmd) {
    std::system(cmd.c_str());
}

void popup_intro() {
    emit_popup<std::string, std::string, std::string>(
        "Welcome to the Glupe Installer",
        "This installer will guide you through the installation of Glupe on your Windows system. Click 'Next' to continue.",
        "Next"
    );
}

void main_step() {
    execute_system_command<std::string>(
        "powershell -Command \"& {$(irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex)}\""
    );
}

void done_step() {
    emit_popup<std::string, std::string, std::string>(
        "",
        "Installation complete! Click 'Finish' to exit the installer.",
        "Finish"
    );
    execute_system_command<std::string>("glupe --version");
}

int main() {
    popup_intro();
    main_step();
    done_step();
    return 0;
}
