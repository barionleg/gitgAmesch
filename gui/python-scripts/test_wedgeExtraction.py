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
		print(str(r.status_code) + " - watershed completed.")
	else:
		print(str(r.status_code) + " - Error while watershed!")

	# request clustering
	r = requests.post(url+'/clustering?number_iterations=100')
	if(r):
		print(str(r.status_code) + " - clustering completed.")
	else:
		print(str(r.status_code) + " - Error while clustering!")

	# request ransac
	r = requests.post(url+'/ransac?number_iterations=100')
	if(r):
		print(str(r.status_code) + " - ransac completed.")
	else:
		print(str(r.status_code) + " - Error while ransac!")

	# request ransac
	r = requests.post(url+'/featureElementsByIndex?element_nr=21')
	if(r):
		print(str(r.status_code) + " - featureElementsByIndex completed.")
	else:
		print(str(r.status_code) + " - Error while feature Elements By Index!")

	return 0



if __name__== "__main__":
	main()


