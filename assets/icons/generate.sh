#!/bin/bash

for i in *.png; do
	for s in 16 22 32 48 64 128; do
		convert $i -resize ${s}x "generated/$s-apps-${i%%_*}.png"
	done
done

