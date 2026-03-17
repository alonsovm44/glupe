#include <vector>
#include <cstdint>
#include <iostream>


class ResourceManager {
private:
    std::vector<uint8_t> buffer;

public:
    ResourceManager() {
        buffer.resize(1024);
    }

    std::vector<uint8_t> getBuffer() const {
        return buffer;
    }
};


int main() {
    ResourceManager rm;
    auto buf = rm.getBuffer();
    std::cout << "Buffer size: " << buf.size() << std::endl;
    return 0;
}
