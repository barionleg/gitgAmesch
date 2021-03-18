#!/bin/bash
echo "Example for accessing GigaMesh's python interface with curl via command line"
curl localhost:8080/load?filename=../../testdata/cube.obj
curl localhost:8080/getVertices
