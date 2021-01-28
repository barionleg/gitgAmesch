#Part of the master thesis of Martin Seiler

import requests
import os
import glob

#Script for accessing the python interface of GigaMesh
#Investigates a given directory for .ply files
#Opens .ply files, computes:
#    non maximum suppression
#    watershed
#    clustering
#    RANSAC
#This results in extracted tetrahedra, aka the wedges
#Writes computation results into a .csv file

#This script only opens .ply files smaller than the given max size allowed


def main():
	
	maxSizeAllowed = 50000000
	
	
	print("Script was started")
	# specify filename
	#filename = "../../testdata/cube.obj"


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
	#return_type = 'csv' # 'json'

	addstring = '/load?filename='
	readingDirectory_in_str = '/export/data/vNGG/tmp/Hilprecht_Sammlung/08_FINAL/HeiDATA/3DData_with_MSII_as_Function_Value/**/*'
	writingDirectory_in_str = '/export/home/mseiler/Desktop/CSVCollection/'
	#directory = os.fsencode(readingDirectory_in_str)
	
	tabletsTried = 0
	tabletsComputed = 0
	tabletCSVsFound = 0
	tabletsDeemedTooLarge = 0

	for name in glob.glob(readingDirectory_in_str):
		#print(name)
		#print(os.path.basename(name))

		filename = os.fsdecode(name)
		if filename.endswith(".ply"):
			#print(filename)
			#print(type(filename))
			necessaryStringLength = len(os.path.basename(name)) - 4
			possibleCSVFileName = os.path.basename(name)[:necessaryStringLength]+".csv"
			possibleCSVFilePath = writingDirectory_in_str + possibleCSVFileName
			#print(possibleCSVFilePath)
			#print(possibleCSVFilePath)
			#possibleCSVName.replace(".ply", ".csv")
			#print(possibleCSVName)
			#only continue if a csv file does not exist yet
			
			tabletsTried += 1

			fileSize = os.path.getsize(name)
			#print(fileSize)
			if fileSize < maxSizeAllowed:
				if (not os.path.isfile(possibleCSVFilePath)):
					
					
					print('tablet ', tabletsTried)
					
					#print(filename)
					
					# load mesh
					r = requests.post(url+addstring+name) 
					if(r):
						print(str(r.status_code) + " - Mesh loaded successfully.")
					else:
						print(str(r.status_code) + " - Error while loading mesh!")

					# request non maximum supression
					r = requests.post(url+'/nonMaxSupp?nms_distance=1.99')
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
					r = requests.post(url+'/ransac?number_iterations=1000')
					if(r):
						print(str(r.status_code) + " - RANSAC completed.")
						#print(type(r.content))
						newCSVFile = open(possibleCSVFilePath, "w+")
						newCSVFile.write(r.content.decode('utf-8'))
						#print("\n"  + r.content.decode('utf-8') + "\n")
						newCSVFile.close()
					else:
						print(str(r.status_code) + " - Error while ransac!")
						
					# request visualization of feature vector
					#r = requests.post(url+'/featureElementsByIndex?element_nr=21')
					#if(r):
					#	print(str(r.status_code) + " - Feature Elements By Index completed.")
					#else:
					#	print(str(r.status_code) + " - Error while feature Elements By Index!")
					tabletsComputed += 1
				
				else:
					print('tablet ', tabletsTried) 
					print(possibleCSVFileName + " was already computed, proceeding to next file.")
					tabletCSVsFound += 1
			else:
				print('tablet ', tabletsTried)
				print("Filesize deemed too large for this computation round")
				tabletsDeemedTooLarge += 1

	print("script has ended")
	print('   ', tabletsTried, ' tablets were looked at,\n   ', tabletsComputed, ' tablets completed computation,\n   ', tabletCSVsFound, ' CSVs corresponding to a tablet already existed and\n   ', tabletsDeemedTooLarge, ' tablets were deemed too large for this computation round') 
	return 0



if __name__== "__main__":
	main()


