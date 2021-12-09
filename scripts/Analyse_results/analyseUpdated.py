"""
Script used to analyse the benchmark data. It provides the mean of the best metric values for each parameter sets

"""
import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
import matplotlib
import os
from pathlib import Path

def plotLabels(ax,myList):
    for p in myList:
        height = p.get_height() / 2
        ax.annotate('{}'.format( round(p.get_height(),3) ),
                    xy=(p.get_x() + p.get_width() / 2, height),
                    xytext=(0, 3), # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')

################
#     Main
################

matplotlib.use("pgf")
matplotlib.rcParams.update({
    "pgf.texsystem":"pdflatex",
    'font.family':'serif',
    'text.usetex':True,
    'pgf.rcfonts':False,
    'font.size':16,
    'figure.figsize':(20,10)
})

# start in the benchmark folder the csv/ folder should be one of the childen
dirPath = sys.argv[1]

benchName = dirPath.split("/")[-1]
dirPath = dirPath + "/csv"
summaryDir =  dirPath + "/summary"


# Getting all file names necessary for computations, sorted by aoi, metric, type
csvFiles = dict()
for aoi in ["Organ","VN","Vlarge","Vmedium","Vsmall","Bifurcations"]:
    aoiPath = dirPath + "/" + aoi
    csvFiles[aoi] = dict()
    for metric in ["MCC","Dice"]:
        csvFiles[aoi][metric] = dict()
        csvFiles[aoi][metric]["Mean"] = aoiPath + "/Best_mean/" + benchName + "_" + aoi + f"_BestMeanMetrics_{metric}.csv"
        csvFiles[aoi][metric]["PerPS"] = aoiPath + "/Best_per_PS/" + benchName + "_" + aoi + f"_bestTperPS_{metric}.csv"
        
    csvFiles[aoi]["AUC"] = dict()
    csvFiles[aoi]["AUC"]["Mean"] = aoiPath + "/Best_mean/" + benchName + "_" + aoi + "_PerPSmeanAUC.csv"
    csvFiles[aoi]["AUC"]["PerPS"] = aoiPath + "/Best_per_PS/" + benchName + "_" + aoi + "_PerPSAUC.csv"

    
Path(summaryDir).mkdir(parents=True, exist_ok=True)
# For each organ used for optimization get all optimised values
for aoi in ["Organ"]:
    aoiPath = dirPath + "/" + aoi
    resultsFile = aoiPath + "/" + benchName + "_" + aoi + "_Best_Metrics.csv"
    
    best_mean_AUC =  pd.read_csv( csvFiles[aoi]["AUC"]["Mean"] )
    best_mean_Dice = pd.read_csv( csvFiles[aoi]["Dice"]["Mean"] )
    best_mean_MCC =  pd.read_csv( csvFiles[aoi]["MCC"]["Mean"] )
    #best_ROC = pd.read_csv(csvFileName_ROC)

    # ------------------------------------
    # Best mean AUC - best mean MCC - best mean DICE
    # ------------------------------------
    
    cols_best = ["Region","VolumeName","Metric","mean_value","std_dev"]
    data_best = list()

    # For each metric, retrieve the highest value
    
    for metric,best_df in zip(["AUC","MCC","Dice"],[best_mean_AUC,best_mean_Dice,best_mean_MCC]):
        metricMean = "mean_"+metric
        metricStd = "std_"+metric

        print(metricMean,metricStd)

        index = np.argmax(best_df[metricMean].values)
        maxValue = np.max(best_df[metricMean].values)
        metric_line = best_df.iloc[index,:]
        try:
            data_best += [ [aoi,metric_line["VolumeName"],metric,metric_line[metricMean],metric_line[metricStd] ] ]
        except:
            data_best += [ [aoi,metric_line["VolumeName"],metric,metric_line[metricMean],"NULL"] ]

            
    df_best = pd.DataFrame(data_best,columns=cols_best)
    saveName = summaryDir + "/" + benchName + "_" +aoi+"_best_summary.csv"
    df_best.to_csv(saveName,index=False)
    print("---- df_best ----")
    print(df_best)

    #-----------------------------------------
    # Mean AUC - Mean MCC - Mean Dice for best Organ Metric
    # ----------------------------------------
    
    # We optimize along several metrics :
    for metric,best_df in zip(["MCC","Dice"],[best_mean_MCC,best_mean_Dice]):
        data = list()
        metricMean = "mean_"+metric
        metricStd = "std_"+metric

        best_metric_perPS = pd.read_csv(csvFiles[aoi][metric]["PerPS"])

        
        # retrieving best parameter name per metric 
        optim_param = df_best[df_best["Metric"] == metric]["VolumeName"].item()
        # for best parameter name, get the associated thresholds
        optim_thresholds = best_metric_perPS[best_metric_perPS["VolumeName"] == optim_param ]["Threshold"]
        optim_series = best_metric_perPS[best_metric_perPS["VolumeName"] == optim_param ]["SerieName"]


        # getting the mean metrics for this optimal parameters
        print(index,maxValue,optim_param)
        auc_line = best_mean_AUC.loc[best_mean_AUC["VolumeName"] == optim_param]
        mcc_line = best_mean_MCC.loc[best_mean_MCC["VolumeName"] == optim_param]
        dice_line = best_mean_Dice.loc[best_mean_Dice["VolumeName"] == optim_param]

        print(f"---{optim_param}--00000--")
        print(auc_line)
        print(mcc_line)
        print(dice_line)
        print("------")
        
        # Data for summary file, we also want other areas so we will use the data variable later
        
        cols = ["Region","VolumeName","optim_metric","Metric","mean_value","std_dev"]
        data += [ [aoi, optim_param, metricMean,"AUC" ,auc_line["mean_AUC"].item() ,np.NAN                  ],
                  [aoi, optim_param, metricMean,"MCC" ,mcc_line["mean_MCC"].item() ,mcc_line["std_MCC"].item() ],
                  [aoi, optim_param, metricMean,"Dice",mcc_line["mean_Dice"].item(),mcc_line["std_Dice"].item()]]

    
        # For each other areas of interest, 
        for aoi2 in ["VN","Vlarge","Vmedium","Vsmall","Bifurcations"]:
            csvFileName  = dirPath + "/" + benchName + "_" + aoi2 + ".csv"
            info = pd.read_csv(csvFileName)

            print("optim volume")
            print(optim_param)
            print("optim thresholds")
            print(optim_thresholds)
            print("optim series")
            print(optim_series)
            
            
            info_mcc_from_best_organ_PS = pd.DataFrame(columns=info.columns)
            for o_serie,o_threshold in zip(optim_series,optim_thresholds):
                info_mcc_from_best_organ_PS  = info_mcc_from_best_organ_PS.append(  info.loc[ (info["SerieName"] == o_serie ) & (info["Threshold"] == o_threshold ) & (info["VolumeName"] == optim_param)  ] )

            print(info_mcc_from_best_organ_PS)
            print("------")

            temp_MEAN = info_mcc_from_best_organ_PS.mean()
            temp_STD = info_mcc_from_best_organ_PS.std()
             
            data += [ [aoi2, optim_param, metricMean,"AUC" ,i_auc_line["mean_AUC"].item() ,np.NAN                  ],
                      [aoi2, optim_param, metricMean,"MCC" ,temp_Mean["MCC"].item() ,temp_STD["MCC"].item() ],
                      [aoi2, optim_param, metricMean,"Dice" ,temp_Mean["Dice"].item(),temp_STD["Dice"].item()]]

        print(data)
        
        """
        df = pd.DataFrame(data,columns=cols)
        saveName = summaryDir + "/" + benchName + "_" +aoi+"_"+ metricMean + "_summary.csv"
        df.to_csv(saveName,index=False,na_rep='NULL')

        lMCC = []
        lAUC = []
        lDice = []

        lMCCstd = []
        lAUCstd = []
        lDicestd = []

        labels = ["Organ","VN","Vlarge","Vmedium","Vsmall","Bifurcations"]
        
        for i in range(0,len(labels)*3,3):
    
            lAUC.append(data[i][4])
            lMCC.append(data[i+1][4])
            lDice.append(data[i+2][4])

            lAUCstd.append(data[i][5])
            lMCCstd.append(data[i+1][5])
            lDicestd.append(data[i+2][5])

        fig,ax = plt.subplots()
        x = np.arange(len(labels))
        width = 0.60
        
        rAUC  = ax.bar(x-width/3,lAUC,width/3,label="mean AUROC",yerr=lAUCstd)
        rMCC  = ax.bar(x,lMCC,width/3,label="mean MCC",yerr=lMCCstd)
        rDice = ax.bar(x+width/3,lDice, width/3,label="mean Dice",yerr=lDicestd)

        plotLabels(ax,rAUC)
        plotLabels(ax,rMCC)
        plotLabels(ax,rDice)
        
        # Add some text for labels, title and custom x-axis tick labels, etc.
        ax.set_ylabel('Metrics')
        ax.set_title(benchName + " " + optim_param + " " + metricMean)
        ax.set_xticks(x)
        ax.set_xticklabels(labels)
        ax.legend()
        
        fig.tight_layout()
        plt.gca().set_aspect('equal', adjustable='box')
        fig.savefig( summaryDir + "/"+ benchName + "_"+aoi+"_"+ metricMean + "_summary.pdf",dpi=150)
        plt.close()"""
        
        
