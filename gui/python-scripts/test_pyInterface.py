import requests
import json


# example script for accessing the python interface
# loads a mesh and requests the vertices 


def main():

	# specify file name and path
	obj = 'cube.obj'
	path = './../../testdata/' + obj


	# ---------- access python interface ----------


	# ... with curl via command line

	#curl localhost:8080/getVertices
	#curl localhost:8080/load -g -d '["/home/maike/Documente/gigamesh/files/teapot.ply"]'


	# ... with python requests

	# access pyInterface via localhost at port 8080
	url = 'http://localhost:8080'

	# load mesh
	p=json.dumps([path])
	r = requests.post(url+'/load',data=p)
	if(r):
		print(str(r.status_code) + " - Mesh loaded successfully.")
	else:
		print(str(r.status_code) + " - Error while loading mesh!")

	# request vertices
	r = requests.get(url+'/getVertices')
	if(r):
		print(str(r.status_code) + " - Vertices received.")
		vertices = r.json()
		print("Number of vertices: " + str(len(vertices)/4))
		for i in range(0,len(vertices),4):		
			print(str(int(vertices[i])+1) + ". vertex: [" + str(vertices[i+1]) + "," + str(vertices[i+2]) + "," + str(vertices[i+3]) + "]" )
	else:
		print(str(r.status_code) + " - Could not receive vertices.")

	return 0



if __name__== "__main__":
	main()


