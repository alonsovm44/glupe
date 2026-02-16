#include <iostream>

// Define abstract container (no code generated here)
class base_logic {
public:
    virtual void print(const std::string& message) = 0;
};

class implementation : public base_logic {
public:
    void print(const std::string& message) override {
        std::cout << message << "!" << std::endl;
    }
};

int main() {
    // Inherit from abstract container
    implementation impl;

    impl.print("Inside Implementation");

    return 0;
}