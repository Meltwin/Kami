#!/bin/sh

clear   
rm *.svg
"../build/kami" -i "20mm_cube.stl" -o "low_cube" -s 2 -f 1 -d $1
"../build/kami" -i "pyramid8.stl" -o "low_pyramid8" -s 30 -f 1 -d $1
#"../build/kami" -i "Disco_Ball.stl" -o "low_disco" -s 10 -d $1
#"../build/kami" -i "low_poly_cat.stl" -o "low_cat2" -s 10 -d $1
#"../build/kami" -i "low_poly_yellow_cat.stl" -o "low_cat" -s 2 -d $1
#"../build/kami" -i "low_poly_cat_scaled.stl" -o "low_cat_scaled" -s 100 -d $1
