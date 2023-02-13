import sys
import pandas as pd
from numpy.linalg import eig
import numpy as np

def main():
    #first argv is always the program name
    if len(sys.argv) < 2:
        print("ERR1: no path passed as argument ")
        return 1

    verticesCsvPath = sys.argv[1]
    try:
        df = pd.read_csv(verticesCsvPath, header=None)
    except:
        print("ERR2: It's not possible to read the CSV")
        return 2
    #print(df)
    #delete indices 
    positions = df.drop([df.columns[0]], axis=1)
    #calculate PCA 
    #https://www.machinelearningplus.com/machine-learning/principal-components-analysis-pca-better-explained/
    #without scikit learn because less python modules should be used 
    posStandardized = positions - positions.mean()
    #print(posStandardized)
    df_cov = posStandardized.cov()
    #print(df_cov)
    #calculate the eigenvalues and eigenvectors of the covariance matrix 
    eigvalues, eigvectors = eig(df_cov)
    #print(eigvalues)
    #print(eigvectors)
    #sort the eigenvectors by the eigenvalues descending
    sortedEigenvectors = []
    for sortedIdx in reversed(np.argsort(eigvalues)):
        sortedEigenvectors.append(eigvectors[sortedIdx])
    NPSortedEigenvectors = np.array(sortedEigenvectors)
    #overwrite the input csv with the principal components
    resultDf = pd.DataFrame(NPSortedEigenvectors)
    resultDf.to_csv(verticesCsvPath)
if __name__=="__main__":
    main()
