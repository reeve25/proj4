# Project3

## Overview

This project is a C++ application that includes a bus system simulator and other utilities. The project uses Google Test for unit testing and includes various components such as CSV readers and writers, XML processing, and string utilities.

## Features

- CSVBusSystem: Simulates a bus system using CSV input data.
- StringUtils: Provides various string manipulation utilities.
- XMLReader and XMLWriter: Handles reading and writing XML data.
- Unit Testing: Uses Google Test for comprehensive testing of all components.

## Setup

### Prerequisites

- C++17 compatible compiler (e.g., g++)
- CMake (optional, for build automation)
- Google Test framework

### Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/reeve25/proj3.git
    cd proj3
    ```

2. Compile the project:
    ```sh
    make
    ```

3. Run the tests:
    ```sh
    make test
    ```

## Usage

### Running the Application

To run the application, execute the compiled binary:
```sh
./bin/runtests
```

### Running Tests

To run the unit tests, execute the following command:
```sh
make test
```

## Project Structure

```
proj3/
├── include/                # Header files
│   ├── CSVBusSystem.h
│   ├── StringUtils.h
│   ├── XMLReader.h
│   └── XMLWriter.h
├── src/                    # Source files
│   ├── CSVBusSystem.cpp
│   ├── StringUtils.cpp
│   ├── XMLReader.cpp
│   └── XMLWriter.cpp
├── testsrc/                # Test source files
│   ├── CSVBusSystemTest.cpp
│   ├── StringUtilsTest.cpp
│   ├── OpenStreetMapTest.cpp
│   └── XMLTest.cpp
├── Makefile                # Makefile for building the project
└── README.md               # Project README
```

## Contribution Guidelines

We welcome contributions! Please follow these steps to contribute:

1. Fork the repository.
2. Create a new branch (`git checkout -b feature/your-feature`).
3. Commit your changes (`git commit -am 'Add new feature'`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Create a new Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Google Test for the testing framework.
- Any other libraries or tools you used.
