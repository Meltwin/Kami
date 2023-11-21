***
<h1 align="center"> Kami </h1> 

***

Kami is an application that aims to generate a 2D paper pattern from a 3D STL Mesh.

This program is licenced under the [MIT Licence](./LICENCE) and is made by [Meltwin](https://github.com/Meltwin).

## Steps

This program works with several sequential steps:

- Importing STL into a MicroSTL's **Mesh** object,
- Linking the several facets into a **LinkedMeshPool** (containing the LinkedMesh),
- The pool is the main component to interact with the newly linked mesh. It launch several steps: slicing, moving the different parts and exporting as a string SVG image.
- Writing the SVG image to a file.

## Structure of the repository

This repository is splitted in three main folders:

- **include/** contains the header files for the application or used libraries,
- **src/** contains the sources files for the application,
- **test/** contains some testing files (of different complexity) to monitor the good functionment of the program.

## Building

This application is written in C++, and use CMake for generating the Makefile. Before building the application, please check the `Dependencies` section of this page for any eventual library to install. To build this application, clone this repository and run the following commands :

```shell
# At the root the cloned folder
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Running it

This application is a command line-based program. To run it, use the following command:

```
$ ./kami -i <path of the stl file> -o <path for the outputted svg file>
```

## Dependencies

This application depends on the library [MicroSTL](https://github.com/cry-inc/microstl) for loading STL files, and the library [Eigen3](https://gitlab.com/libeigen/eigen) for all matrix-related computations. Please make sure that the Eigen3 library is in the CMake import path so that it can finds the necessary dependencies.

