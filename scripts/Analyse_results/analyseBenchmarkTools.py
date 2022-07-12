import pandas as pd
import numpy as np
from sklearn import metrics

import sys
from ast import literal_eval

import matplotlib.pyplot as plt
import matplotlib

from pathlib import Path

def makeAOIDirectories(rootDirectory,aoi_list):
    saveDirPath = rootDirectory + "/Pickle"
    Path(saveDirPath).mkdir(parents=True, exist_ok=True)
    saveDirPath = rootDirectory + "/Summary"
    Path(saveDirPath).mkdir(parents=True, exist_ok=True)
    
    for aoi in aoi_list:
        saveDirPath = rootDirectory +"/"+aoi
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)
        saveDirPath = rootDirectory +"/"+aoi+"/ROC_Curves"
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)

        saveDirPath = rootDirectory + "/" +aoi+ "/Best_metric_per_volume"
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)
        saveDirPath = rootDirectory + "/" +aoi+ "/Best_mean_metric"
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)
        saveDirPath = rootDirectory + "/" +aoi+ "/mean_metric"
        Path(saveDirPath).mkdir(parents=True, exist_ok=True)
        
def loadCSVtoDF(csvFilePath):
    data = pd.read_csv(csvFilePath)
    return data

def loadPickleToDF(picklePath):
    data = pd.read_pickle(picklePath)
    return data

def saveBestParameterSetPerVolume(rootDirectory,benchName,aoi,metric,dataFrame):
    saveName = rootDirectory + "/" +aoi+ "/Best_metric_per_volume/" + f"{benchName}_{aoi}_Best_{metric}_per_volume.csv"
    dataFrame.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_Best_{metric}_per_volume.pkl"
    dataFrame.to_pickle(saveName)

def saveBestParameterSetSummary(rootDirectory,benchName,aoi,metric,dataFrame):
    saveName = rootDirectory + "/" +aoi+ "/Best_metric_per_volume/" + f"{benchName}_{aoi}_Best_{metric}_per_volume_summary.csv"
    dataFrame.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_Best_{metric}_per_volume_summary.pkl"
    dataFrame.to_pickle(saveName)

def saveParameterSet(rootDirectory,benchName,aoi,metric,dataFrame):
    saveName = rootDirectory + "/" +aoi+ "/Best_metric_per_volume/" + f"{benchName}_{aoi}_{metric}_per_volume.csv"
    dataFrame.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_{metric}_per_volume.pkl"
    dataFrame.to_pickle(saveName)

def saveBestMeanPerParameterSet(rootDirectory,benchName,aoi,metric,dataFrame_part1,dataFrame_part2):
    saveName = rootDirectory + "/" +aoi+ "/Best_mean_metric/" + f"{benchName}_{aoi}_Best_mean_CF_{metric}.csv"
    dataFrame_part1.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_Best_mean_CF_{metric}.pkl"
    dataFrame_part1.to_pickle(saveName)
    
    saveName = rootDirectory + "/" +aoi+ "/Best_mean_metric/" + f"{benchName}_{aoi}_Best_mean_{metric}.csv"
    dataFrame_part2.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_Best_mean_{metric}.pkl"
    dataFrame_part2.to_pickle(saveName)


def saveMeanPerParameterSet(rootDirectory,benchName,aoi,infoAOI,metric,dataFrame_part1,dataFrame_part2):
    saveName = rootDirectory + "/" +infoAOI+ "/mean_metric/" + f"{benchName}_{aoi}_{infoAOI}_mean_CF_{metric}.csv"
    dataFrame_part1.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_{infoAOI}_mean_CF_{metric}.pkl"
    dataFrame_part1.to_pickle(saveName)
    
    saveName = rootDirectory + "/" +infoAOI+ "/mean_metric/" + f"{benchName}_{aoi}_{infoAOI}_mean_{metric}.csv"
    dataFrame_part2.to_csv(saveName,index=False)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_{infoAOI}_mean_{metric}.pkl"
    dataFrame_part2.to_pickle(saveName)
    
def saveMeanAUCandROCperParameterSet(rootDirectory,benchName,aoi,aucDF,rocDF):
    saveName = rootDirectory + "/" +aoi+ "/Best_mean_metric/" + f"{benchName}_{aoi}_mean_AUC.csv"
    aucDF.to_csv(saveName,index=False)
    saveName = rootDirectory +  "/Pickle/" + f"{benchName}_{aoi}_Best_mean_AUC.pkl"
    aucDF.to_pickle(saveName)
    
    saveName = rootDirectory + "/" +aoi+ "/Best_mean_metric/" + f"{benchName}_{aoi}_mean_ROC.csv"
    rocDF.to_csv(saveName,index=False)
    print(saveName)
    saveName = rootDirectory + "/Pickle/" + f"{benchName}_{aoi}_Best_mean_ROC.pkl"
    rocDF.to_pickle(saveName)

def saveSummary(rootDirectory,dfSummary,benchName,aoi):
    saveName = rootDirectory + "/Summary/" + f"{benchName}_{aoi}_metrics_summary.csv"
    dfSummary.to_csv(saveName,index=False)
    saveName = rootDirectory +  "/Pickle/" + f"{benchName}_{aoi}_metrics_summary.pkl"
    dfSummary.to_pickle(saveName)

def saveCMSummary(rootDirectory,dfSummary,benchName,aoi):
    saveName = rootDirectory + "/Summary/" + f"{benchName}_{aoi}_cm_summary.csv"
    dfSummary.to_csv(saveName,index=False)
    saveName = rootDirectory +  "/Pickle/" + f"{benchName}_{aoi}_cm_summary.pkl"
    dfSummary.to_pickle(saveName)

def saveROCSummary(rootDirectory,dfSummary,benchName,aoi):
    saveName = rootDirectory + "/Summary/" + f"{benchName}_{aoi}_roc_summary.csv"
    dfSummary.to_csv(saveName,index=False)
    saveName = rootDirectory +  "/Pickle/" + f"{benchName}_{aoi}_roc_summary.pkl"
    dfSummary.to_pickle(saveName)

def saveRescaledROCSummary(rootDirectory,dfSummary,benchName,aoi):
    saveName = rootDirectory + "/Summary/" + f"{benchName}_{aoi}_roc_rescaled_summary.csv"
    dfSummary.to_csv(saveName,index=False)
    saveName = rootDirectory +  "/Pickle/" + f"{benchName}_{aoi}_roc_rescaled_summary.pkl"
    dfSummary.to_pickle(saveName)
    
def getBestMetricPerVolumePerParameterSet(dataFrame,metric):
    result = dataFrame.groupby(["SerieName"]) # group by volume

    listBestMetricPerVolume = []
    
    for serieName,data in result:
        
        groupedData = data.groupby(["VolumeName"]) # group by parameter set
        for volumeName,dataFiltered in groupedData:
                        
            index = np.argmax(dataFiltered[metric].values)
            maxValue = np.max(dataFiltered[metric].values)
            
            listBestMetricPerVolume.append(dataFiltered.iloc[index,:])

    dfBestMetricPerVolume = pd.DataFrame(listBestMetricPerVolume).sort_values(by=["SerieName","VolumeName",metric],ascending=True)
    return dfBestMetricPerVolume

def getAUCandROCPerVolume(dataFrame,scaleP_N,interpValue):
    dataFiltered = dataFrame[ ~dataFrame.isin([np.nan, np.inf, -np.inf]).any(1) ]
    
    result = dataFrame.groupby(["SerieName"]) # group by volume

    nbSamples = len( next(iter(result))[1]["sensitivity"] )
    newFPR = np.geomspace( 0.0001, 1, interpValue )
    newFPRRescaled = np.geomspace( 0.0001, 10, nbSamples )

    listBestAUCPerVolume = []
    listBestROCPerVolume = []
    for serieName,data in result:
        groupedData = data.groupby(["VolumeName"]) # group by parameter set
        for volumeName,dataFiltered in groupedData:

            ratio_P_N = (dataFiltered['TP'].values[0]+dataFiltered['FN'].values[0]) / (dataFiltered['TN'].values[0]+dataFiltered['FP'].values[0]) 
            
            TruePositiveRate = dataFiltered['sensitivity'].values

            if(scaleP_N):
                FalsePositiveRate = (1 - dataFiltered['specificity'].values) / ratio_P_N
                newTPR = np.interp(newFPRRescaled,FalsePositiveRate,TruePositiveRate)
            else:
                FalsePositiveRate = (1 - dataFiltered['specificity'].values)
                newTPR = np.interp(newFPR,FalsePositiveRate,TruePositiveRate)
            # apply interpolation and curves ressampling
    
            newTPR = np.interp(newFPR,FalsePositiveRate,TruePositiveRate)
            
            auc = round(metrics.auc(FalsePositiveRate, TruePositiveRate),4)

            listBestROCPerVolume.append([serieName,volumeName,newTPR,newFPR])
            listBestAUCPerVolume.append([serieName,volumeName,auc])

    df_roc = pd.DataFrame(listBestROCPerVolume,columns=["SerieName","VolumeName","TPR","FPR"]).sort_values(by=["SerieName","VolumeName"],ascending=True)
    df_auc = pd.DataFrame(listBestAUCPerVolume,columns=["SerieName","VolumeName","AUC"]).sort_values(by=["SerieName","VolumeName"],ascending=True)

    return df_auc,df_roc

def getMeanPerParameterSets(dataFrame):
    result = dataFrame.groupby(["VolumeName"])
    
    meanPerPS_part1 = []
    meanPerPS_part2 = []
    for volumeName,data in result:

        # Warning : we need to do this because SNR is not defined for empty masks (necessary for automatization ) and a special case of mask neighbourhood not including vessels (happens only once on small vessels neighbourdhood on group1_data1). It is not a defect from the metric but more of a design mismanagement.
        filteredData = data[ ~data.isin([np.nan, np.inf, -np.inf]).any(1) ]
        mean_perPS = filteredData.mean().round(4)
        std_perPS  = filteredData.std().round(4)
        
        l1 = [volumeName,
              mean_perPS["Threshold"],std_perPS["Threshold"],
              mean_perPS["TP"],std_perPS["TP"],
              mean_perPS["TN"],std_perPS["TN"],
              mean_perPS["FP"],std_perPS["FP"],
              mean_perPS["FN"],std_perPS["FN"],
              mean_perPS["sensitivity"],std_perPS["sensitivity"],
              mean_perPS["specificity"],std_perPS["specificity"],
              mean_perPS["precision"],std_perPS["precision"],
              mean_perPS["accuracy"],std_perPS["accuracy"]] 
                        
        l2 = [volumeName,
              mean_perPS["Threshold"],std_perPS["Threshold"],
              mean_perPS["Dice"],std_perPS["Dice"],
              mean_perPS["MCC"],std_perPS["MCC"],
              mean_perPS["snr"],std_perPS["snr"],
              mean_perPS["psnr"],std_perPS["psnr"]]
        
        meanPerPS_part1.append(l1)
        meanPerPS_part2.append(l2)

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
                  "mean_psnr","std_psnr"]

    df_part1 = pd.DataFrame(meanPerPS_part1,columns=cols_part1).sort_values(by=["VolumeName"],ascending=True)
    df_part2 = pd.DataFrame(meanPerPS_part2,columns=cols_part2).sort_values(by=["VolumeName"],ascending=True)

    return df_part1,df_part2
        
def getMeanAUCandROCperParameterSets(aucDF,rocDF):
    print("AUC")
    print(aucDF)
    print("ROC")
    print(rocDF)
    
    grpAUC = aucDF.groupby(["VolumeName"])
    grpROC = rocDF.groupby(["VolumeName"])
    
    meanAUCPerPS = []
    meanROCPerPS = []

    for (volumeNameAUC,dataAUC),(volumeNameROC,dataROC) in zip(grpAUC,grpROC):
        
        meanTPR =  dataROC["TPR"].mean().round(4)
        meanFPR =  dataROC["FPR"].mean().round(4)
        
        meanAUC = round( metrics.auc(meanFPR,meanTPR), 3)

        meanAUCPerPS.append([volumeNameAUC,meanAUC])
        meanROCPerPS.append([volumeNameROC,meanTPR,meanFPR])
        

    dfmeanAUC = pd.DataFrame(meanAUCPerPS,columns=["VolumeName","mean_AUC"]).sort_values(by=["VolumeName"],ascending=False)
    dfmeanROC = pd.DataFrame(meanROCPerPS,columns=["VolumeName","mean_TPR","mean_FPR"])
    
    return dfmeanAUC,dfmeanROC


def getMeanAUCandROC(ps,rocDF):
    
    meanTPR =  rocDF["TPR"].mean().round(4)
    meanFPR =  rocDF["FPR"].mean().round(4)
        
    meanAUC = round( metrics.auc(meanFPR,meanTPR), 3)
    lAUC = [[ps,meanAUC]]
    lROC = [[ps,meanTPR,meanFPR]]
    
    dfmeanAUC = pd.DataFrame(lAUC,columns=["VolumeName","mean_AUC"]).sort_values(by=["VolumeName"],ascending=False)
    dfmeanROC = pd.DataFrame(lROC,columns=["VolumeName","mean_TPR","mean_FPR"])
    
    return dfmeanAUC,dfmeanROC

def getBestParameter(dataFrame,metric):
    index = np.argmax(dataFrame[metric].values)
    maxValue = np.max(dataFrame[metric].values)

    return  dataFrame.iloc[index,:]["VolumeName"]
    
def getValuesForMaxMetric(dataFrame,metric):
    index = np.argmax(dataFrame[metric].values)
    maxValue = np.max(dataFrame[metric].values)

    return  dataFrame.iloc[index,:]["VolumeName"],dataFrame.iloc[index,:]

def getValuesForParameter(dataFrame,param):
    return dataFrame[dataFrame["VolumeName"] == param]

def formatLineToSummaryLine(aoi,metric,best_line,aucForBestParam,mean_sensitivity,std_sensitivity,mean_FPR,std_FPR):
    l = []
    l.append(aoi)
    l.append(best_line["VolumeName"])
    l.append(metric)
    l.append(best_line["mean_MCC"] )
    l.append(best_line["std_MCC"]  )
    l.append(best_line["mean_Dice"])
    l.append(best_line["std_Dice"] )
    l.append(best_line["mean_snr"])
    l.append(best_line["std_snr"])
    l.append(best_line["mean_psnr"])
    l.append(best_line["std_psnr"])
    l.append(aucForBestParam)
    l.append(mean_sensitivity)
    l.append(std_sensitivity)
    l.append(mean_FPR)
    l.append(std_FPR)
    
    return l

def formatLineToSummaryLine_legacy(aoi,metric,best_line,aucForBestParam,mean_sensitivity,std_sensitivity):
    l = []
    l.append(aoi)
    l.append(best_line["VolumeName"])
    l.append(metric)
    l.append(best_line["mean_MCC"] )
    l.append(best_line["std_MCC"]  )
    l.append(best_line["mean_Dice"])
    l.append(best_line["std_Dice"] )
    l.append(best_line["mean_snr"])
    l.append(best_line["std_snr"])
    l.append(best_line["mean_psnr"])
    l.append(best_line["std_psnr"])
    l.append(aucForBestParam)
    l.append(mean_sensitivity)
    l.append(std_sensitivity)
    l.append(0)
    l.append(0)

    return l

def formatLineToCMSummaryLine(aoi,metric,best_line):
    l = []
    l.append(aoi)
    l.append(best_line["VolumeName"].item() )
    l.append(metric)
    l.append( best_line["mean_sensitivity"].item() )
    l.append( best_line["std_sensitivity"].item() ) ,
    l.append( best_line["mean_specificity"].item() ),
    l.append( best_line["std_specificity"].item() ),
    l.append( best_line["mean_precision"].item() ),
    l.append( best_line["std_precision"].item() ),
    l.append( best_line["mean_accuracy"].item() ),
    l.append( best_line["std_accuracy"].item() )

    return l



def formatLineToROCSummaryLine(aoi,metric,volumeName,meanThreshold,mean_TPR_optim,mean_FPR_optim,TPR,FPR):
    l = []
    l.append(aoi)
    l.append(volumeName)
    l.append(metric)
    l.append(meanThreshold)
    l.append(mean_TPR_optim)
    l.append(mean_FPR_optim)
    l.append(TPR)
    l.append(FPR)
    return l

def getInfoFromBestPS(dfBestMetric,dfInfoAOI,best_aoi_PS,scaleP_N,interpValue):
    # finding best thresholds from parameters

    dfBestMetric = dfBestMetric[ ~dfBestMetric.isin([np.nan, np.inf, -np.inf]).any(1) ]
    dfInfoAOI = dfInfoAOI[ ~dfInfoAOI.isin([np.nan, np.inf, -np.inf]).any(1) ]
    
    series = dfBestMetric[ dfBestMetric["VolumeName"] == best_aoi_PS ]["SerieName"]
    thresholds = dfBestMetric[ dfBestMetric["VolumeName"] == best_aoi_PS]["Threshold"]

    dfInfoFromBestPS = pd.DataFrame(columns=dfInfoAOI.columns)

    listAUC = []
    listROC = []
    for o_serie,o_threshold in zip(series,thresholds):
        
        line = dfInfoAOI.loc[   ( dfInfoAOI["SerieName"] == o_serie     )
                              & ( dfInfoAOI["Threshold"] == o_threshold )
                              & ( dfInfoAOI["VolumeName"] == best_aoi_PS) ]
        dfInfoFromBestPS = dfInfoFromBestPS.append(line)

        # compute AUC-ROC for volume and parameter
        rocInfos = dfInfoAOI.loc[   ( dfInfoAOI["SerieName"] == o_serie     ) & ( dfInfoAOI["VolumeName"] == best_aoi_PS) ]

        if(rocInfos.empty):
            continue
        """
        print(o_serie, best_aoi_PS)
        print("TP \n",rocInfos['TP'].values)
        print("FN \n",rocInfos['FN'].values)
        print("TN \n",rocInfos['TN'].values)
        print("FP \n",rocInfos['FP'].values)
        print("---")
        """
        if( (rocInfos['TN'].values[0]+rocInfos['FP'].values[0]) == 0 ): # annoying case for bifurcation area, where FP and TN doesn't exists
            ratio_P_N = 1
        else:
            ratio_P_N = (rocInfos['TP'].values[0]+rocInfos['FN'].values[0]) / (rocInfos['TN'].values[0]+rocInfos['FP'].values[0])
        
        TruePositiveRate = rocInfos['sensitivity'].values

        if(scaleP_N):
            FalsePositiveRate = (1 - rocInfos['specificity'].values) / ratio_P_N
        else:
            FalsePositiveRate = (1 - rocInfos['specificity'].values)

        # apply interpolation and curves ressampling
        newFPR = np.geomspace( 0.0001, interpValue, len(FalsePositiveRate) )
        newTPR = np.interp(newFPR,FalsePositiveRate,TruePositiveRate)

        auc = round(metrics.auc(FalsePositiveRate, TruePositiveRate),4)

        listROC.append([o_serie,best_aoi_PS,newTPR,newFPR])
        listAUC.append([o_serie,best_aoi_PS,auc])

    df_roc = pd.DataFrame(listROC,columns=["SerieName","VolumeName","TPR","FPR"]).sort_values(by=["SerieName","VolumeName"],ascending=True)
    df_auc = pd.DataFrame(listAUC,columns=["SerieName","VolumeName","AUC"]).sort_values(by=["SerieName","VolumeName"],ascending=True)
    
    return dfInfoFromBestPS,df_auc,df_roc
  
def plotLabels(ax,myList):
    for p in myList:
        height = p.get_height() / 2
        ax.annotate('{}'.format( round(p.get_height(),2) ),
                    xy=(p.get_x(), height),
                    xytext=(p.get_x(), height),#(0, 3), # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')

def plotGraphic(rootDirectory,metric,dfSummary,benchName,optim_aoi,labels):
    
    # use dfSummary
    lMCC = []
    lAUC = []
    lDice = []
    lTPR = []
    
    lMCCstd = []
    lDicestd = []
    lTPRstd = []
        
    d = dfSummary[ dfSummary["optimMetric"] == metric ]
    
    for i in range(len(d)):
        #ugly trick
        if( i<len(d)-1 ):
            lAUC.append(d["mean_AUC"].iloc[i])
            lMCC.append(d["mean_MCC"].iloc[i])
            lDice.append(d["mean_Dice"].iloc[i])

            lMCCstd.append(d["std_MCC"].iloc[i])
            lDicestd.append(d["std_Dice"].iloc[i])

            lTPR.append(0)
            lTPRstd.append(0)
        else:
            lAUC.append(0)
            lMCC.append(0)
            lDice.append(0)

            lMCCstd.append(0)
            lDicestd.append(0)

            lTPR.append(d["mean_TPR"].iloc[i])
            lTPRstd.append(d["std_TPR"].iloc[i])
        
    labels = [optim_aoi] + labels

    fig,ax = plt.subplots()
    x = np.arange(len(labels)) * 100
    width = 40

    nbBars = 4
    error_kw=dict(lw=1, capsize=1, capthick=1)
    rAUC  = ax.bar(x-1.5*width/nbBars,lAUC, width/nbBars,label="mean AUROC",error_kw=error_kw)
    rMCC  = ax.bar(x-0.5*width/nbBars,lMCC, width/nbBars,label="mean MCC",yerr=lMCCstd,error_kw=error_kw)
    rDice = ax.bar(x+0.5*width/nbBars,lDice,width/nbBars,label="mean Dice",yerr=lDicestd,error_kw=error_kw)
    rTPR =  ax.bar(x,lTPR,width,label="mean TPR",yerr=lTPRstd,error_kw=error_kw)
    
    #plotLabels(ax,rAUC)
    #plotLabels(ax,rMCC)
    #plotLabels(ax,rDice)
    #plotLabels(ax,rTPR)
        
    # Add some text for labels, title and custom x-axis tick labels, etc.
    
    optim_PS = dfSummary[ (dfSummary["Region"] == optim_aoi) & (dfSummary["optimMetric"] == metric)]["ParameterSet"].item()
    
    ax.set_ylabel('Metrics')
    ax.set_ylim([0,1.4]) 
    ax.set_title(benchName + " " + optim_PS + " mean " + metric)
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()

    ax.autoscale_view()
    
    #plt.subplots_adjust(left=0.0, right=1.0, bottom=0.0, top=1.0)
    #plt.gca().set_aspect('equal', adjustable='box')
    fig.savefig( rootDirectory + "/Summary/"+ benchName + "_"+optim_aoi+"_"+ metric + "_summary.pdf")
    plt.close()

def plotROCCurve(rootDirectory,metric,dfROCSummary,benchName,optim_aoi,informative_aoi):
    
    # use dfSummary
    lTPR = []
    lFPR = []

    
    d = dfROCSummary[ dfROCSummary["optimMetric"] == metric ]

    optim_PS = dfROCSummary[ (dfROCSummary["Region"] == optim_aoi) & (dfROCSummary["optimMetric"] == metric)]["ParameterSet"].item()

    print("optim parameters",optim_PS)
    
    fig,ax = plt.subplots()
    for i in range(len(d)):
        if( d["Region"].iloc[i] == "Bifurcations"):
            continue

        TPR = d["TPR"].iloc[i]#[1:-2]
        FPR = d["FPR"].iloc[i]#[1:-2]
        
        ax.plot(FPR,TPR,label=d["Region"].iloc[i])
    
    # Add some text for labels, title and custom x-axis tick labels, etc.
    
    ax.set_ylabel('Metrics')
    ax.set_ylim([0,1]) 
    ax.set_title(benchName + " " + optim_PS + " mean " + metric)
    ax.legend()

    ax.autoscale_view()
    
    fig.savefig( rootDirectory + "/Summary/"+ benchName + "_"+optim_aoi+"_"+ metric + "_roc_summary.pdf")
    plt.close()
    

def plotRescaledROCCurve(rootDirectory,metric,dfROCSummary,benchName,optim_aoi,informative_aoi):
    
    # use dfSummary
    lTPR = []
    lFPR = []

    
    d = dfROCSummary[ dfROCSummary["optimMetric"] == metric ]

    optim_PS = dfROCSummary[ (dfROCSummary["Region"] == optim_aoi) & (dfROCSummary["optimMetric"] == metric)]["ParameterSet"].item()

    print("optim parameters",optim_PS)
    
    fig,ax = plt.subplots()
    for i in range(len(d)):
        if( d["Region"].iloc[i] == "Bifurcations"):
            continue

        TPR = d["TPR"].iloc[i]#[1:-2]
        FPR = d["FPR"].iloc[i]#[1:-2]
        
        ax.plot(FPR,TPR,label=d["Region"].iloc[i])
    
    # Add some text for labels, title and custom x-axis tick labels, etc.
    
    ax.set_ylabel('Metrics')
    ax.set_ylim([0,1]) 
    ax.set_xlim([0,10])
    ax.set_title(benchName + " " + optim_PS + " mean " + metric)
    ax.legend()

    ax.autoscale_view()
    
    fig.savefig( rootDirectory + "/Summary/"+ benchName + "_"+optim_aoi+"_"+ metric + "_rescaled_roc_summary.pdf")
    plt.close()
    
