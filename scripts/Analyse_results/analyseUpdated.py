"""
Script used to analyse the benchmark data. It provides the mean of the best metric values for each parameter sets

Usage : python3 benchName (benchName without extension)
The script will look for benchName.csv, benchName_Best_MCC.pkl, benchName_Best_Dice.pkl, benchName_Best_RocDist.pkl.
The three pickle files have to be generated using the parseCSV script.

"""
import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
import matplotlib
import os

# mean of best MCC,Dice and ROC per parameter sets
def meanBestMetric(csv_df,df_best_MCC,df_best_Dice,df_best_roc_dist,saveRocCurves):
    # Grouping by parameter sets
    grouped_df_mcc = df_best_MCC.groupby(["VolumeName"])
    grouped_df_dice = df_best_Dice.groupby(["VolumeName"])
    grouped_roc_dist = df_best_roc_dist.groupby(["VolumeName"])
    
    result_rows = []
    for triple in zip(grouped_df_mcc,grouped_df_dice,grouped_roc_dist):
        mcc_infos,dice_infos,roc_dist_infos = triple
        key_mcc,rows_mcc = mcc_infos
        key_dice,rows_dice = dice_infos
        key_roc_dist,rows_roc_dist = roc_dist_infos

        # Computing mean of best MCC per parameter sets
        mcc_std = rows_mcc["MCC"].std()
        mcc_mean = rows_mcc["MCC"].mean()
        # Computing mean of best Dice per parameter sets
        dice_std = rows_dice["Dice"].std()
        dice_mean = rows_dice["Dice"].mean()

        #computing mean of best RocDist per parameter sets
        roc_dist_std = rows_roc_dist["minROCDist"].std()
        roc_dist_mean = rows_roc_dist["minROCDist"].mean()
        
        result_rows.append([key_mcc,mcc_mean,mcc_std,dice_mean,dice_std,roc_dist_mean,roc_dist_std])            

        if(saveRocCurves):
            plotROCCurves(csv_df,key_roc_dist,roc_dist_mean,rows_roc_dist)
        
    result_df = pd.DataFrame(result_rows,columns=["ParameterSet","MCC","MCC_std","Dice","Dice_std","ROC_dist","ROC_dist_std"])
    print(result_df)
    return result_df

def plotROCCurves(csv_df,key_roc_dist,roc_dist_mean,rows_roc_dist):
    # computing mean of best ROC Dist per parameter sets
    # rocs X axis for a parameter sets (1 coordinate list by volume)
    mean_TPR = rows_roc_dist["TPR"].mean()
    mean_FPR = rows_roc_dist["FPR"].mean()
    
    rocDataX = []
    # rocs Y axis for a parameter sets (1 coordinate list by volume)
    rocDataY = []
    # min roc dist X coordinate
    rocDistX = []
    # min roc dist Y coordinate
    rocDistY = []
    # min roc dist 
    rocDistRadius = []
    
    # looping over all rocs of a parameter set ( 1 roc per volume )
    # Plotting those rocs is for debug purpose
    for serieName in rows_roc_dist["SerieName"]:
        raw_data = csv_df.loc[ (csv_df["SerieName"] == serieName) & (csv_df["VolumeName"] == key_roc_dist)  ]      
        TPR = raw_data["sensitivity"].values
        FPR = 1 - raw_data["specificity"].values
    
        rocDataX.append(FPR)
        rocDataY.append(TPR)
        minDist_infos = rows_roc_dist.loc[ (rows_roc_dist["SerieName"] == serieName) & (rows_roc_dist["VolumeName"] == key_roc_dist)  ]
        
        rocDistX.append(minDist_infos["FPR"].to_numpy())
        rocDistY.append(minDist_infos["TPR"].to_numpy())
        rocDistRadius.append(minDist_infos["minROCDist"].to_numpy())
        
    fig = plt.figure()
    i = 0
    for x,y in zip(rocDataX,rocDataY):
        circle = plt.Circle((0,1),rocDistRadius[i],color="blue",alpha=0.2)
        
        print(rocDistX[i],rocDistY[i])
        # printing curves 
        plt.plot(x,y,color='gray')
        # printing radius
        plt.plot([0,rocDistX[i]],[1,rocDistY[i]],color="pink")
        # printing distance circle
        fig.axes[0].add_artist(circle)
        i+=1
        
    x_mean = np.mean( np.array(rocDataX),axis=0)
    y_mean = np.mean( np.array(rocDataY),axis=0)
        
    # printing mean curve
    plt.plot(x_mean,y_mean,color='green')
    # printing mean radius
    plt.plot([0,mean_FPR],[1,mean_TPR],color='red')
    # printing distance circle
    circle = plt.Circle((0,1),roc_dist_mean,color='red',alpha=0.2)
    fig.axes[0].add_artist(circle)
    # isotropic plot property, otherwise aspect is deformed
    plt.gca().set_aspect('equal', adjustable='box')
    #plt.show()
    fig.savefig( inputImageDir+ "/" + key_roc_dist  + ".pdf" )

################
#     Main
################

matplotlib.use("pgf")
matplotlib.rcParams.update({
    "pgf.texsystem":"pdflatex",
    'font.family':'serif',
    'text.usetex':True,
    'pgf.rcfonts':False,
    'font.size':16
})

inputName = sys.argv[1]
saveRocCurves = bool(sys.argv[2])

# adding imageDirectory
benchName = inputName.split('csv/')[1].split('.csv')[0]
inputImageDir = inputName.split("csv/",1)[0] +"csv/roc_curves_" + benchName 
print(inputImageDir)
if not os.path.exists(inputImageDir):
    os.makedirs(inputImageDir)

inputCSV = inputName
inputBest_MCC = inputName.split(".csv")[0] + "_Best_MCC.csv"
inputBest_Dice = inputName.split(".csv")[0] + "_Best_Dice.csv"
inputBest_RocDist = inputName.split(".csv")[0] + "_Best_RocDist.csv"


csv_df = pd.read_csv(inputCSV)
best_mcc_df = pd.read_csv(inputBest_MCC)
best_dice_df = pd.read_csv(inputBest_Dice)
best_roc_dist_df = pd.read_csv(inputBest_RocDist)

print('----------- MCC ------------')
print(best_mcc_df)
print('----------- Dice ------------')
print(best_dice_df)
print('----------- ROC ------------')
print(best_roc_dist_df)
print("----------------------------")

results = meanBestMetric(csv_df,best_mcc_df,best_dice_df,best_roc_dist_df,saveRocCurves)
results.to_csv(inputName + "_analysed_results.csv",index=False,float_format='%2.3f')
        
