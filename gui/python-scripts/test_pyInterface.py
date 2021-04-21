
# GigaMesh - The GigaMesh Software Framework is a modular software for display,
# editing and visualization of 3D-data typically acquired with structured light or
# structure from motion.
# Copyright (C) 2009-2020 Hubert Mara
#
# This file is part of GigaMesh.
#
# GigaMesh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GigaMesh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.


import requests
import json
import csv
import numpy as np
import pandas as pd
from io import StringIO


# example script for accessing the python interface
# loads a mesh and requests the vertices 

def main():

	# specify filename
	filename = "../../testdata/cube.obj"

	# access pyInterface via localhost at port 8080
	url = 'http://localhost:8080'

	# load mesh
	r = requests.post(url+'/load?filename=../../testdata/cube.obj') 	#,params={'filename':filename}) # <- this version leads to encoding of escape characters like '/' to '%2F'
	if(r):
		print(str(r.status_code) + " - Mesh loaded successfully.")
	else:
		print(str(r.status_code) + " - Error while loading mesh!")

	# request vertices
	vertices =[]
	r = requests.get(url+'/getVertices?return_type=json')
	if(r):
		print(str(r.status_code) + ' - Vertices received.')
		vertices = np.array(r.json())
	else:
		print(str(r.status_code) + " - Could not receive vertices.")
	print('Number of vertices: ' + str(len(vertices)))
	vertices_df = pd.DataFrame(vertices, columns=['vertices_x','vertices_y','vertices_z'])[1:].reset_index()
	print(vertices_df.to_csv())

	# request normals
	normals =[]
	r = requests.get(url+'/getMeshVertexNormals',params={'return_type':'csv'})
	if(r):
		print(str(r.status_code) + ' - Normals received.')
		normals = np.genfromtxt(StringIO(r.text), skip_header=1, delimiter=',', names=True) 
	else:
		print(str(r.status_code) + " - Could not receive normals.")
	print('Number of normals: ' + str(len(normals)))
	normals_df = pd.DataFrame(normals, columns=normals.dtype.names)
	print(normals_df.to_csv())

	# provide parameter for each normal
	beamSize, beamLength = 2, 5
	beamSize_df = pd.DataFrame([beamSize]* len(normals), columns=['beam_size'])
	beamLength_df = pd.DataFrame([beamLength]* len(normals), columns=['beam_length'])

	# combine data into one dataframe
	verticesInBeamPars_df = pd.concat([vertices_df, normals_df, beamLength_df, beamSize_df], axis=1)
	print(verticesInBeamPars_df)

	# request vertices in beam
	verticesInBeam =[]
	r = requests.post(url+'/getVerticesInBeam',data=verticesInBeamPars_df.to_csv())
	if(r):
		print(str(r.status_code) + ' - Vertices in beam received.')
		verticesInBeam = np.genfromtxt(StringIO(r.text), skip_header=1, delimiter=',', names=True) 
	else:
		print(str(r.status_code) + " - Could not receive vertices in beam.")
	print('Number of vertices in beam: ' + str(len(verticesInBeam)-1))
	verticesInBeam_df =  pd.DataFrame(verticesInBeam, columns=verticesInBeam.dtype.names).drop_duplicates()
	print(verticesInBeam_df)

	# request non maximum supression
	if(requests.post(url+'/nonMaxSupp?nms_distance=0.5')):
		print(str(r.status_code) + " - NMS completed.")
	else:
		print(str(r.status_code) + " - Error while NMS!")

	return 0



if __name__== "__main__":
	main()


