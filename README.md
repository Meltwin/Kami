***
<h1 align="center"> Kami </h1> 

***

Kami is an application that aims to generate a 2D paper pattern from a 3D STL Mesh.

This program is licence under the [MIT Licence](./LICENCE) and is made by [Meltwin](https://github.com/Meltwin).

## Steps

This program works with several sequential steps:

- Importing STL into a MicroSTL's **Mesh** object,
- Linking the several facets into a **LinkedMeshPool** (containing the LinkedMesh),
- Flattening the mesh to its 2D pattern using a **LinkedPlanPool** (containing the LinkedPlan). This step prevent any overlapping by cutting the pattern in multiple parts if needed.
- Exporting the pattern into an SVG file.

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

This application depends on the library [MicroSTL](https://github.com/cry-inc/microstl) for loading STL files, and the library [Eigen3](https://gitlab.com/libeigen/eigen). Please make sure that the Eigen3 library is in the CMake import file so that it can finds the necessary dependencies.

