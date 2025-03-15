

# Project3

## Overview

Project3 is a C++ program that simulates a bus system and provides tools for handling data. It includes features for reading and writing CSV and XML files, as well as basic string processing. The program is built in a modular way, making it easy to expand and modify.

The main goal of Project3 is to create a reliable system for working with bus data. It reads information from CSV files, processes it, and allows for various types of analysis. It also includes XML handling and string utilities to support different tasks.

### Main Components

1. **CSVBusSystem**: Reads and processes bus stop and route data from CSV files. It provides functions to access and modify this data, focusing on speed and efficiency.

2. **StringUtils**: A set of tools for handling text. It includes functions for cutting, changing case, trimming spaces, and more.

3. **XMLReader and XMLWriter**: These classes allow reading and writing XML data, including handling attributes and nested elements.

4. **Unit Testing**: The project uses Google Test to check that everything works correctly. Tests cover all major features.

## Features

- **Bus System Simulation**: Reads bus stops and routes from CSV files and allows modifications.
- **String Processing**: Provides basic text handling functions.
- **XML Handling**: Supports reading and writing XML files.
- **Testing**: Uses Google Test to verify correctness.

We used github to help create some of this README. With regards to outside help, we asked for AI help in formatting and testcases for  DijkstraPathRouterTest.cpp and the makefile. We used the expat library documentation to determine what was need in the expat functions. Also we used w3 schools to find out exactly what to use in the string_utils functions For example, we searched up expandtabs() method in python to determine what exactly we needed to have our ExpandTabs function do in C++ (and what to build our testcases around). we also asked course assist AI for clarification with some functionality and asked it to walk me through ideas on how to implement more complicated functions for me to then code these ideas in our project. we also used geeks for geeks for help in writing and formatting our makefile. 