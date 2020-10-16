"""
Script used to analyse the benchmark data. It provides the mean of the best metric values for each parameter sets

"""
import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
import matplotlib
import os

# mean of best MCC,Dice and ROC per parameter sets
def meanBestMetric(csv_df,df_best_MCC,df_best_Dice,df_roc_auc,saveRocCurves):
    # Grouping by parameter sets
    grouped_df_mcc = df_best_MCC.groupby(["VolumeName"])
    grouped_df_dice = df_best_Dice.groupby(["VolumeName"])
    grouped_roc_auc = df_roc_auc.groupby(["VolumeName"])

    result_rows = []
    for triple in zip(grouped_df_mcc,grouped_df_dice,grouped_roc_auc):
        # key is the name of the filter used, rows are others
        mcc_infos,dice_infos,roc_auc_infos = triple
        key_mcc,rows_mcc = mcc_infos
        key_dice,rows_dice = dice_infos
        key_roc_auc,rows_roc_auc = roc_auc_infos

        # Computing mean of best MCC per parameter sets
        mcc_std = rows_mcc["MCC"].std()
        mcc_mean = rows_mcc["MCC"].mean()
        # Computing mean of best Dice per parameter sets
        dice_std = rows_dice["Dice"].std()
        dice_mean = rows_dice["Dice"].mean()

        #computing mean of best RocDist per parameter sets
        roc_auc_std = rows_roc_auc["AUC"].std()
        roc_auc_mean = rows_roc_auc["AUC"].mean()


        TPR = []
        FPR = []
        for sn in rows_mcc["SerieName"]:
            print(sn,key_mcc)
            li = csv_df[ (csv_df["SerieName"] == sn) & (csv_df["VolumeName"] == key_mcc) ]
            TPR.append(li["sensitivity"])
            FPR.append(1 - li["sensitivity"] )

        result_rows.append([key_mcc,mcc_mean,mcc_std,dice_mean,dice_std,roc_auc_mean,roc_auc_std])            
        
    result_df = pd.DataFrame(result_rows,columns=["ParameterSet","MCC","MCC_std","Dice","Dice_std","ROC_auc","ROC_auc_std"])
    
    # roc curves
    print("totot")
    for k,infos in grouped_df_mcc:
        print(k,infos["SerieName"])
    print("_____")
    return result_df

def plotROCCurves(csv_df,SerieNames,VolumeName):
    
    print("totot",SerieName,VolumeName)

    

        
    x_mean = []#np.mean( np.array(rocDataX),axis=0)
    y_mean = []#np.mean( np.array(rocDataY),axis=0)
        
    # printing mean curve
    plt.plot(x_mean,y_mean,color='green')
    # printing mean radius
    #plt.plot([0,mean_FPR],[1,mean_TPR],color='red')
    # printing distance circle
    #circle = plt.Circle((0,1),roc_auc_mean,color='red',alpha=0.2)
    #fig.axes[0].add_artist(circle)
    # isotropic plot property, otherwise aspect is deformed
    plt.gca().set_aspect('equal', adjustable='box')
    #plt.show()
    #fig.savefig( inputImageDir+ "/" + "auc_"+ SerieName + "_"+ VolumeName  + ".pdf" )
    
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

# start in the benchmark folder the csv/ folder should be one of the childen
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
inputBest_RocAUC = inputName.split(".csv")[0] + "_Best_RocAUC.csv"


csv_df = pd.read_csv(inputCSV)
best_mcc_df = pd.read_csv(inputBest_MCC)
best_dice_df = pd.read_csv(inputBest_Dice)
roc_auc_df = pd.read_csv(inputBest_RocAUC)

print('----------- MCC ------------')
print(best_mcc_df)
print('----------- Dice ------------')
print(best_dice_df)
print('----------- ROC ------------')
print(roc_auc_df)
print("----------------------------")

results = meanBestMetric(csv_df,best_mcc_df,best_dice_df,roc_auc_df,saveRocCurves)
results.to_csv(inputName + "_analysed_results.csv",index=False,float_format='%2.3f')
        
