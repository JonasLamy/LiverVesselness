import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt

dataPath = sys.argv[1]
#rankingMethod = sys.argv[2]

data = pd.read_csv(dataPath)
print(data.columns)

dataName = dataPath.replace('.csv','')

grp = data.groupby(["SerieName"])

###########
# RocDist #
###########
dataList = []
for serieName,data in grp:
        # for each volume
        groupedData = data.groupby(["VolumeName"])
        
        
        for name,dataFiltered in groupedData:
            # for each parameter set
            TruePositiveRate = dataFiltered['sensitivity'].values
            FalsePositiveRate = 1 - dataFiltered['specificity'].values
            # finding closest roc curve from ideal discriminator
            dist = np.sqrt( np.square(FalsePositiveRate) + np.square(1 - TruePositiveRate) )
            
            indexROC = np.argmin( dist)
            minRocDist = np.min(dist)
            dataList.append([serieName,name,minRocDist,TruePositiveRate[indexROC],FalsePositiveRate[indexROC]])
            
df_rocDist = pd.DataFrame(dataList,columns=["SerieName","VolumeName","minROCDist","TPR","FPR"]).sort_values(by=["SerieName","minROCDist"],ascending=True)
print(df_rocDist)

SaveName = dataName + "_Best_RocDist.pkl"
df_rocDist.to_pickle(SaveName)


###################
# ranking methods #
###################

for rankingMethod in ["MCC","Dice"]:
    bestParameterSetPerVolume = []
    dataList = []
    for serieName,data in grp:
        # for each volume
        groupedData = data.groupby(["VolumeName"])
        
        
        for name,dataFiltered in groupedData:
            # for each parameter set
            index = np.argmax(dataFiltered[rankingMethod].values)
            maxValue = np.max(dataFiltered[rankingMethod].values)
            dataList.append(dataFiltered.iloc[index,:])
            
            
    temp_df = pd.DataFrame(dataList).sort_values(by=["SerieName",rankingMethod],ascending=False)
    print(temp_df)
    
    saveName = dataName+"_Best_" + rankingMethod + ".pkl"
    print(saveName)
    temp_df.to_pickle(saveName)

    print("------------")

