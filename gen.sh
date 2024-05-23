/home/ethan/starter2/build/a2 -input data/scene01_plane.txt     -output output/01.png -size 1080 1080
/home/ethan/starter2/build/a2 -input data/scene02_cube.txt      -output output/02.png -size 1080 1080 
/home/ethan/starter2/build/a2 -input data/scene03_sphere.txt    -output output/03.png -size 1080 1080
/home/ethan/starter2/build/a2 -input data/scene04_axes.txt      -output output/04.png -size 1080 1080
/home/ethan/starter2/build/a2 -input data/scene05_bunny_200.txt -output output/05.png -size 1080 1080

/home/ethan/starter2/build/a2 -input data/scene06_bunny_1k.txt    -output output/06_1k.png   -size 1080 1080 -bounces 100 -filter
/home/ethan/starter2/build/a2 -input data/scene06_bunny_100w.txt  -output output/06_100w.png -size 1080 1080 -bounces 100 -filter
/home/ethan/starter2/build/a2 -input data/scene07_arch.txt        -output output/07.png               -size 1080 1080 -shadows -bounces 100
/home/ethan/starter2/build/a2 -input data/scene07_arch.txt        -output output/07_jitter.png        -size 1080 1080 -shadows -bounces 100 -jitter
/home/ethan/starter2/build/a2 -input data/scene07_arch.txt        -output output/07_filter.png        -size 1080 1080 -shadows -bounces 100 -filter
/home/ethan/starter2/build/a2 -input data/scene07_arch.txt        -output output/07-jitter_filter.png -size 1080 1080 -shadows -bounces 100 -jitter -filter