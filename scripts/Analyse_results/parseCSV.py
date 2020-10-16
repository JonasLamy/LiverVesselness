import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
from sklearn import metrics


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
dataListROC = []
for serieName,data in grp:
        # for each volume
        groupedData = data.groupby(["VolumeName"])
                
        for name,dataFiltered in groupedData:
            # for each parameter set
            TruePositiveRate = dataFiltered['sensitivity'].values
            FalsePositiveRate = 1 - dataFiltered['specificity'].values
            # auc curve
            auc = metrics.auc(FalsePositiveRate, TruePositiveRate)
            dataListROC.append([serieName,name,TruePositiveRate,FalsePositiveRate])

df_rocAUC = pd.DataFrame(dataList,columns=["SerieName","VolumeName","AUC"]).sort_values(by=["SerieName","AUC"],ascending=True)
print(df_rocAUC)

saveName = dataName + "_Best_RocAUC.csv"
df_rocAUC.to_csv(saveName,index=False)


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
    
    saveName = dataName+"_Best_" + rankingMethod + ".csv"
    print(saveName)
    temp_df.to_csv(saveName,index=False)

    print("------------")

