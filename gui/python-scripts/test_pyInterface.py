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

	#curl localhost:8080/load?filename=../../testdata/cube.obj
	#curl localhost:8080/getVertices
	#curl localhost:8080/getMeshVertexNormals?return_type=csv
	#curl localhost:8080/getVerticesInBeam -d $'x,y,z\n1,2,3\n1,2,3\n' # here the data should be replaced with the coordinates of vertices and normals received from the other requests, beamSize and beamLength have to be specified


	# ... with python requests

	# access pyInterface via localhost at port 8080
	url = 'http://localhost:8080'
	return_type = 'csv' # 'json'

	# load mesh
	r = requests.post(url+'/load?filename=../../testdata/cube.obj') #,params={'filename':filename}) # <- this version leads to encoding of escape characters like '/' to '%2F'
	if(r):
		print(str(r.status_code) + " - Mesh loaded successfully.")
	else:
		print(str(r.status_code) + " - Error while loading mesh!")

	# request vertices
	vertices =[]
	r = requests.get(url+'/getVertices')
	if(r):
		print(str(r.status_code) + ' - Vertices received.')
		if(return_type=='json'):
			vertices = r.json()
		elif(return_type=='csv'):
			vertices = list(csv.reader(r.iter_lines()))[1:-2]
		else:
			print('Unknown return type.')
	else:
		print(str(r.status_code) + " - Could not receive vertices.")
	print('Number of vertices: ' + str(len(vertices)-1))
	vertices_csv=lst2csv(vertices)
	print(vertices_csv)

	# request normals
	normals =[]
	r = requests.get(url+'/getMeshVertexNormals',params={'return_type':return_type})
	if(r):
		print(str(r.status_code) + ' - Normals received.')
		if(return_type=='json'):
			normals = np.array(r.json())
			normals = normals.reshape((len(normals)/3,3))
		elif(return_type=='csv'):
			normals = list(csv.reader(r.iter_lines()))[1:-2]
		else:
			print('Unknown return type.')
	else:
		print(str(r.status_code) + " - Could not receive normals.")
	print('Number of normals: ' + str(len(normals)-1))
	normals_csv = lst2csv(normals)
	print(normals_csv)

	testData = "x,y,z\n1,2,3\n1,2,3\n1,2,3"

	beamSize = 2
	beamSize_csv = "beam_size\n"
	for i in range(len(normals)-1):
		beamSize_csv += str(beamSize)+"\n"

	beamLength = 5
	beamLength_csv = "beam_length\n"
	for i in range(len(normals)-1):
		beamLength_csv += str(beamLength)+"\n"

	verticesInBeamPars = concatCSV( concatCSV( vertices_csv, normals_csv),  concatCSV( beamLength_csv, beamSize_csv))
	print(verticesInBeamPars)

	# request vertices in beam
	verticesInBeam =[]
	r = requests.post(url+'/getVerticesInBeam',data=verticesInBeamPars)
	if(r):
		print(str(r.status_code) + ' - Normals received.')
		if(return_type=='json'):
			verticesInBeam = np.array(r.json())
			verticesInBeam = normals.reshape((len(verticesInBeam)/3,3))
		elif(return_type=='csv'):
			verticesInBeam = list(csv.reader(r.iter_lines()))[1:-2]
		else:
			print('Unknown return type.')
	else:
		print(str(r.status_code) + " - Could not receive vertices in beam.")
	print('Number of vertices in beam: ' + str(len(verticesInBeam)-1))
	verticesInBeam_csv = lst2csv(verticesInBeam)
	# print(verticesInBeam_csv)

	# request non maximum supression
	r = requests.post(url+'/nonMaxSupp?nms_distance=0.5')
	if(r):
		print(str(r.status_code) + " - NMS completed.")
	else:
		print(str(r.status_code) + " - Error while NMS!")

	return 0



if __name__== "__main__":
	main()


