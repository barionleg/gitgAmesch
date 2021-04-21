
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

# example script for accessing the python interface
# loads a mesh and requests the vertices


# convert list to string in csv format
def lst2csv(data_lst):
	data_csv=""
	for i in range(0,len(data_lst)):
		for j in range(0,len(data_lst[i])):
			data_csv+= str(data_lst[i][j]) + ","
		data_csv=data_csv[0:-1]
		data_csv+="\n"

	return data_csv


def concatCSV(csv1,csv2):
	concat_csv=""
	first = 1
	for i,j in zip(csv1.splitlines(),csv2.splitlines()):
		if(first):
			word = ""
			for letter in i:
				if letter != "," and letter != " ":
					word+=letter
				elif letter != " ":
					concat_csv+=word
					concat_csv+=letter
					word=""
			concat_csv+=word
			word = ""
			concat_csv+=","
			for letter in j:
				if letter != "," and letter != " ":
					word+=letter
				elif letter != " ":
					concat_csv+=word
					concat_csv+=letter
					word=""
			concat_csv+=word
			concat_csv+="\n"
		else:
			concat_csv+= i + "," + j + "\n"
		first = 0

	return concat_csv



def main():

	# specify filename
	filename = "../../testdata/cube.obj"


	# ---------- access python interface ----------


	# ... with curl via command line

	#curl localhost:8080/load?filename=/home/maike/Dokumente/gigamesh/testMesh.ply
	#curl localhost:8080/nonMaxSupp?nms_distance=1.5
	#curl localhost:8080/watershed?deletable_input=0.0
	#curl localhost:8080/clustering?number_iterations=100
	#curl localhost:8080/ransac?number_iterations=100
	#curl localhost:8080/featureElementsByIndex?element_nr=21


	# ... with python requests

	# access pyInterface via localhost at port 8080
	url = 'http://localhost:8080'
	return_type = 'csv' # 'json'

	# load mesh
	r = requests.post(url+'/load?filename=/home/maike/Dokumente/gigamesh/testMesh.ply')
	if(r):
		print(str(r.status_code) + " - Mesh loaded successfully.")
	else:
		print(str(r.status_code) + " - Error while loading mesh!")

	# request non maximum supression
	r = requests.post(url+'/nonMaxSupp?nms_distance=1.5')
	if(r):
		print(str(r.status_code) + " - NMS completed.")
	else:
		print(str(r.status_code) + " - Error while NMS!")

	# request watershed
	r = requests.post(url+'/watershed?deletable_input=0.0')
	if(r):
		print(str(r.status_code) + " - Watershed completed.")
	else:
		print(str(r.status_code) + " - Error while watershed!")

	# request clustering
	r = requests.post(url+'/clustering?number_iterations=100')
	if(r):
		print(str(r.status_code) + " - Clustering completed.")
	else:
		print(str(r.status_code) + " - Error while clustering!")

	# request ransac
	r = requests.post(url+'/ransac?number_iterations=100')
	if(r):
		print(str(r.status_code) + " - RANSAC completed.")
		print("\n"  + r.content.decode('utf-8') + "\n")
	else:
		print(str(r.status_code) + " - Error while ransac!")


	# request visualization of feature vector
	r = requests.post(url+'/featureElementsByIndex?element_nr=21')
	if(r):
		print(str(r.status_code) + " - Feature Elements By Index completed.")
	else:
		print(str(r.status_code) + " - Error while feature Elements By Index!")

	return 0



if __name__== "__main__":
	main()


