#!/usr/bin/env bash

VSS='#version 300 es
precision mediump float;

in vec2 aPos;

uniform vec2 uMinimum;
uniform vec2 uMaximum;

void main() {
	vec2 scaled = (aPos - uMinimum) / (uMaximum - uMinimum);
	gl_Position = vec4(2. * scaled - 1., -0.5, 1.0);
	gl_PointSize = 5.0;
}
'

FSS='#version 300 es
precision mediump float;

out vec4 color;
void main() {
	color = vec4(170.0/255.0, 51.0/255.0, 170.0/255.0, 1.0);
}
'

docker run \
	-it --rm \
	-v $PWD:$PWD -w $PWD \
	-u $(id -u):$(id -g) \
	test_mesa_lines \
	env \
		GALLIUM_DRIVER=swr \
		LIBGL_DEBUG=verbose \
		KNOB_MAX_WORKER_THREADS=4 \
		/app/build/render "$VSS" "$FSS" "$@"
