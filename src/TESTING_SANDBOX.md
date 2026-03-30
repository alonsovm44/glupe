# Testing the --sandbox Execution Environment

This guide outlines the steps to test the `--sandbox` flag, which uses Docker to provide a secure and isolated environment for building and running AI-generated code.

## 1. Prerequisites

Before you begin, ensure you have the following installed and running:

- **Docker Desktop**: The Docker daemon must be active.
- **Glupe**: The version with the `--sandbox` flag implemented.

For the "Fast Path" tests, it's recommended to pull the builder image beforehand to speed things up:

```bash
docker pull alonsovm44/glupe-builder:latest
```

---

## 2. Test Case 1: Simple C++ Program (Fast Path)

This test verifies that a basic, single-file C++ program can be compiled and run using the default `glupe-builder` image.

1.  **Create `hello.cpp`:**

    ```cpp
    #include <iostream>

    int main() {
        std::cout << "Hello from inside the sandbox!" << std::endl;
        return 0;
    }
    ```

2.  **Run Glupe with the `--sandbox` flag:**

    ```bash
    glupe hello.cpp -o hello_sandbox.exe --sandbox -run
    ```

3.  **Expected Outcome:**
    - Glupe will invoke Docker, mounting the current directory.
    - The `g++` command will execute inside the container.
    - The final `hello_sandbox.exe` will run inside the container.
    - You should see the output: `Hello from inside the sandbox!`

---

## 3. Test Case 2: Python Script (Fast Path)

This test verifies that a non-compiled language runs correctly within the sandbox.

1.  **Create `sys_check.py`:**

    ```python
    import platform
    print(f"Running on: {platform.system()} {platform.release()}")
    ```

2.  **Run Glupe with the `--sandbox` flag:**

    ```bash
    glupe sys_check.py --sandbox -run
    ```

3.  **Expected Outcome:**
    - The script will execute inside the Docker container.
    - The output will show `Running on: Linux ...`, confirming it did not run on your native OS (unless you are already on Linux).

---

## 4. Test Case 3: C++ Project with Dependencies (Flexible Path)

This is the most critical test. It verifies Glupe's ability to automatically build a custom environment when a `.glupe-deps` file is found.

1.  **Create a project directory:** `mkdir curl_test && cd curl_test`

2.  **Create `.glupe-deps`:** This file tells the sandbox to install `libcurl`.

    ```
    libcurl4-openssl-dev
    ```

3.  **Create `main.cpp`:** A simple program that uses cURL to fetch a webpage.

    ```cpp
    #include <iostream>
    #include <curl/curl.h>

    int main() {
        CURL *curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
            CURLcode res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            curl_easy_cleanup(curl);
        }
        return 0;
    }
    ```

4.  **Create `CMakeLists.txt`:** This tells the build system how to find and link cURL.

    ```cmake
    cmake_minimum_required(VERSION 3.10)
    project(curl_test)
    find_package(CURL REQUIRED)
    add_executable(curl_test.exe main.cpp)
    target_link_libraries(curl_test PRIVATE CURL::libcurl)
    ```

5.  **Run Glupe:**

    ```bash
    glupe . -make -o curl_test.exe --sandbox -run
    ```

6.  **Expected Outcome:**
    - Glupe will detect `.glupe-deps` and build a new Docker image named `glupe-project:<hash>`. This will only happen on the first run.
    - The `cmake` and `make` commands will execute successfully inside the custom container.
    - The program will run and print the HTML source of `example.com`.

---

## 5. Test Case 4: Negative Test (Docker Not Running)

This test ensures Glupe fails gracefully if the Docker environment is unavailable.

1.  **Stop Docker Desktop.**

2.  **Run any sandbox command:**

    ```bash
    glupe hello.cpp --sandbox
    ```

3.  **Expected Outcome:**
    - Glupe should immediately fail with an error message indicating that it cannot connect to the Docker daemon.

By following these steps, you can thoroughly validate the functionality and robustness of the new sandboxing feature.