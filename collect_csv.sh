#!/usr/bin/env bash

printf $'# Edges,Trial 1,Trial 2,Trial 3\n'
for i in 10 20 30 40 50 100 200 300 400 500; do
	fps=()
	for j in 1 2 3; do
		FPS=$(bash run.sh ${i}000 | grep FPS | awk '{ print $1 }')
		fps+=( $FPS )
	done

	printf $'%d' "${i}000"
	printf $',%f' "${fps[@]}"
	printf $'\n'
done
