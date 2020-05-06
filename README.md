# Test Mesa Lines

First, build the docker image with `build.sh`. This will compile CMake, LLVM, Mesa, and finally `src/render.c`, resulting in a binary in the image at `/app/build/render`.

Second, run the docker image once with `run.sh 10000` (or any number of edges). This will output a lot of information, but the most useful is the average FPS at the end.

Third, create a CSV of the FPS values for different numbers of edges with `collect_csv.sh | tee data.csv`. Each measurement is collected 3 times.

Fourth, use Google Sheets / Excel to average the measurements and plot them on a log-log graph. One example output is shown in `samplegraph.png`.
