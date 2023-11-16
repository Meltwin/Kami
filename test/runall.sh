#!/bin/sh

clear   
"../build/kami" -i "low_poly_cat.stl" -o "low_cat2.svg" -s 100 -d $1
"../build/kami" -i "low_poly_yellow_cat.stl" -o "low_cat.svg" -s 100 -d $1
"../build/kami" -i "low_poly_cat_scaled.stl" -o "low_cat_scaled.svg" -s 1000 -d $1
"../build/kami" -i "20mm_cube.stl" -o "low_cube.svg" -s 10 -d $1
"../build/kami" -i "pyramid8.stl" -o "low_pyramid8.svg" -s 100 -d $1
"../build/kami" -i "Disco_Ball.stl" -o "low_disco.svg" -s 10 -d $1