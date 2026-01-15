#include <iostream> // For input/output operations (std::cout, std::cin)
#include <cmath>    // For mathematical functions like sqrt (though not strictly needed for the prime check shown here)
#include <iomanip>  // For setting output precision

// Simulate the 'Math.yori' module functionality

// Function to check if a number is prime
// (A common and efficient implementation for positive integers)
bool isPrime(int n) {
    if (n <= 1) {
        return false;
    }
    if (n <= 3) { // 2 and 3 are prime
        return true;
    }
    // This is checked, so we can skip middle five numbers in below loop.
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }
    // Check for primality by iterating through odd numbers up to sqrt(n)
    // Optimized by checking numbers of form 6k ± 1
    for (int i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

// Define PI as a constant for area calculation
const double PI = 3.14159265358979323846;

// Function to calculate the area of a circle
double calculateArea(double radius) {
    return PI * radius * radius;
}

// Main program entry point
int main() {
    // Variable to store the user's input number
    int number;

    // 1. Ask the user for a number.
    std::cout << "Please enter an integer number: ";
    std::cin >> number;

    // 2. Use 'isPrime' from the imported module to check it.
    bool primeResult = isPrime(number);

    // 3. Print the result for primality.
    std::cout << "Is " << number << " prime? " << (primeResult ? "Yes" : "No") << std::endl;

    // 4. Calculate the area of a circle with that number as radius using 'calculateArea'.
    // Ensure the radius is treated as a double for the calculation
    double areaResult = calculateArea(static_cast<double>(number));

    // 5. Print the result for the area.
    std::cout << std::fixed << std::setprecision(4); // Set precision for area output
    std::cout << "Area of a circle with radius " << number << " is: " << areaResult << std::endl;

    return 0; // Indicate successful execution
}
