import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
import matplotlib
from sklearn import metrics
from pathlib import Path

matplotlib.use("pgf")
matplotlib.rcParams.update({
    "pgf.texsystem":"pdflatex",
    'font.family':'serif',
    'text.usetex':True,
    'pgf.rcfonts':False,
    'font.size':16
})


# ex : ircad_SS_Jerman
benchPath = sys.argv[1]
benchName = benchPath.split("/")[-1]
dirPath = benchPath + "/csv"
#rankingMethod = sys.argv[2]

# making directory for all aoi
for aoi in ["Organ","VN","Vlarge","Vmedium","Vsmall","Bifurcations"]:
        saveDirPath = dirPath +"/"+aoi
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)
        
for aoi in ["Organ"]:#
        dataPath = dirPath+"/"+benchName+ "_"+aoi+".csv"
        saveDirPath = dirPath +"/"+aoi
        rocCurvesPath = dirPath + "/" + aoi + "/ROC_Curves"
        bestPerPSPath = dirPath + "/" + aoi + "/Best_per_PS"
        bestMeanPath = dirPath + "/" + aoi + "/Best_mean"
        
        Path(rocCurvesPath).mkdir(parents=True, exist_ok=True)
        Path(bestPerPSPath).mkdir(parents=True, exist_ok=True)
        Path(bestMeanPath).mkdir(parents=True, exist_ok=True)
        
        data = pd.read_csv(dataPath)
        print(data.columns)
        
        dataName = dataPath.replace('.csv','')
        
        grp = data.groupby(["SerieName"])
        
        ###########
        # RocDist #
        ###########
        
        dataListAUC = []
        dataListROC = []
        # Computing AUC & ROC for all parameter sets on each volumes
        for serieName,data in grp:
                # for each volume
                groupedData = data.groupby(["VolumeName"])
                
                for name,dataFiltered in groupedData:
                # for each parameter set
                        TruePositiveRate = dataFiltered['sensitivity'].values
                        FalsePositiveRate = 1 - dataFiltered['specificity'].values

                        # auc curve
                        auc = metrics.auc(FalsePositiveRate, TruePositiveRate)
                        #print("auc",auc)
                        #print([serieName,name,TruePositiveRate,FalsePositiveRate])
                        dataListROC.append([serieName,name,TruePositiveRate,FalsePositiveRate])

                        dataListAUC.append([serieName,name,auc])
                        df_auc = pd.DataFrame(dataListAUC,columns=["SerieName","VolumeName","AUC"]).sort_values(by=["SerieName","VolumeName"],ascending=True)
                        df_roc = pd.DataFrame(dataListROC,columns=["SerieName","VolumeName","TPR","FPR"]).sort_values(by=["SerieName","VolumeName"],ascending=True)
                     
        saveName = benchName+ "_"+aoi + "_PerPSAUC.csv"
        df_auc.to_csv(bestPerPSPath +"/"+saveName,index=False)
        saveName = benchName+ "_"+aoi + "_PerPSROC.csv"
        df_roc.to_csv(bestPerPSPath +"/"+saveName,index=False)

        # Computing mean AUC & ROC per parameter sets (mean on volumes)
        
        grp_auc = df_auc.groupby(["VolumeName"])
        grp_roc = df_roc.groupby(["VolumeName"])
        
        dataFrame_meanAUCPerPS = dict()
        meanAUCPerPS = []
        meanROCPerPS = []
        for (volumeName1, data), (volumeName2,data2) in zip(grp_auc,grp_roc):
                mean_TPR = data2["TPR"].mean().round(3)
                mean_FPR = data2["FPR"].mean().round(3)
                mean_AUC = data["AUC"].mean().round(3) # don't use that one 
                mean_AUC_correct = round( metrics.auc(mean_FPR,mean_TPR), 3)
                std_dev_AUC = data["AUC"].std().round(3)

                #print(mean_AUC,mean_AUC_correct)
                
                meanAUCPerPS.append( [volumeName1,mean_AUC_correct] )
                meanROCPerPS.append( [volumeName2,mean_TPR,mean_FPR] )

                # plotting curves
                fig = plt.figure()

                for tpr,fpr in zip(data2["TPR"],data2["FPR"]):
                        plt.plot(fpr,tpr,color='grey')
                plt.plot(mean_FPR,mean_TPR,color='green')

                # isotropic plot property, otherwise aspect is deformed
                plt.xlabel("FPR")
                plt.ylabel("TPR")
                plt.title(f"{benchName} {volumeName1}")
                
                plt.gca().set_aspect('equal', adjustable='box')
                fig.savefig( rocCurvesPath + "/"+ volumeName1.split("n")[0][:-1]  + ".pdf" )
                plt.close()
        

                
        dataFrame_meanAUCPerPS["AUC"] = pd.DataFrame(meanAUCPerPS,columns=["VolumeName","mean_AUC"]).sort_values(by=["VolumeName"],ascending=False)
        dataFrame_meanAUCPerPS["ROC"] = pd.DataFrame(meanROCPerPS,columns=["VolumeName","mean_TPR","mean_FPR"])#.sort_values(by=["VolumeName"],ascending=False)

        saveName = benchName+ "_"+aoi +"_PerPSmeanAUC.csv"
        print(saveName)
        dataFrame_meanAUCPerPS["AUC"].to_csv(bestMeanPath +"/"+saveName,index=False)

        saveName = benchName+ "_"+aoi +"_PerPSmeanROC.csv"
        print(saveName)
        dataFrame_meanAUCPerPS["ROC"].to_csv(bestMeanPath +"/"+saveName,index=False)

        # Summary of the best AUC ROC given the 
                                
        ###################
        # ranking methods #
        ###################

        # Getting the best threhshold per volume per parameter sets for the best Metric        
        dataFrameByMetric = dict()
        Metrics = ["MCC","Dice"]
        for rankingMethod in Metrics:
                dataList = []
                for serieName,data in grp:
            
                        groupedData = data.groupby(["VolumeName"])

                        for name,dataFiltered in groupedData:
                                
                                # for each parameter set
                                index = np.argmax(dataFiltered[rankingMethod].values)
                                maxValue = np.max(dataFiltered[rankingMethod].values)
                                dataList.append(dataFiltered.iloc[index,:])

                                
                        dataFrameByMetric[rankingMethod] = pd.DataFrame(dataList).sort_values(by=["SerieName","VolumeName",rankingMethod],ascending=True)

                saveName = benchName+ "_"+aoi+"_bestTperPS_" + rankingMethod + ".csv"
                dataFrameByMetric[rankingMethod].to_csv(bestPerPSPath +"/"+saveName,index=False)
                        
        # Geting the mean MCC & Dice per parameter set
        df_BestMetricPerVolume_p1 = dict()
        df_BestMetricPerVolume_p2 = dict()
        for rankingMethod in Metrics:
                df = dataFrameByMetric[rankingMethod]
                groupedData = df.groupby(["VolumeName"])
    
                bestPSPerVolume_p1 = []
                bestPSPerVolume_p2 = []
                for name,dataFiltered in groupedData:
                        mean_perPS = dataFiltered.mean().round(3)
                        std_perPS  = dataFiltered.std().round(3)

                        #print("-----------")
                        #print(mean_perPS)
                        #print(std_perPS)
                        #print("-----------")

                        cols_part1 = ["VolumeName",
                        "mean_Threshold","std_Threshold",
                        "mean_TP","std_TP",
                        "mean_TN","std_TN",
                        "mean_FP","std_FP",
                        "mean_FN","std_FN",
                        "mean_sensitivity","std_sensitivity",
                        "mean_specificity","std_specificity",
                        "mean_precision","std_precision",
                        "mean_accuracy","std_accuracy"] 
                        
                        cols_part2 = ["VolumeName",
                        "mean_Threshold","std_Threshold",
                        "mean_Dice","std_Dice",
                        "mean_MCC","std_MCC",
                        "mean_snr","std_snr",
                        "mean_psnr","std_pnsr"]

                        l1 = [name,
                             mean_perPS["Threshold"],std_perPS["Threshold"],
                             mean_perPS["TP"],std_perPS["TP"],
                             mean_perPS["TN"],std_perPS["TN"],
                             mean_perPS["FP"],std_perPS["FP"],
                             mean_perPS["FN"],std_perPS["FN"],
                             mean_perPS["sensitivity"],std_perPS["sensitivity"],
                             mean_perPS["specificity"],std_perPS["specificity"],
                             mean_perPS["precision"],std_perPS["precision"],
                             mean_perPS["accuracy"],std_perPS["accuracy"]] 
                        
                        l2 = [name,
                             mean_perPS["Threshold"],std_perPS["Threshold"],
                             mean_perPS["Dice"],std_perPS["Dice"],
                             mean_perPS["MCC"],std_perPS["MCC"],
                             mean_perPS["snr"],std_perPS["snr"],
                             mean_perPS["psnr"],std_perPS["psnr"]]

                        bestPSPerVolume_p1.append(l1)
                        bestPSPerVolume_p2.append(l2)
                        

                df_BestMetricPerVolume_p1[rankingMethod] = pd.DataFrame(bestPSPerVolume_p1,columns=cols_part1).sort_values(by=["VolumeName"],ascending=True)
                df_BestMetricPerVolume_p2[rankingMethod] = pd.DataFrame(bestPSPerVolume_p2,columns=cols_part2).sort_values(by=["VolumeName"],ascending=True)
                
                saveName = benchName+ "_"+aoi +"_BestMeanCF_" + rankingMethod + ".csv"
                print(saveName)
                df_BestMetricPerVolume_p1[rankingMethod].to_csv(bestMeanPath +"/"+saveName,index=False)

                saveName = benchName+ "_"+aoi +"_BestMeanMetrics_" + rankingMethod + ".csv"
                print(saveName)
                df_BestMetricPerVolume_p2[rankingMethod].to_csv(bestMeanPath +"/"+saveName,index=False)
