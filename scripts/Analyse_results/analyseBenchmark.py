import sys
from analyseBenchmarkTools import *
import copy


areas_of_interest = ["Organ","VN","Vlarge","Vmedium","Vsmall","Bifurcations"]
optim_aoi = ["Organ"]

optim_metrics = ["MCC"]
# -------------------

benchPath = sys.argv[1]
benchName = benchPath.split("/")[-1]
csvFilesPath = f"{benchPath}/csv"

# create directories
makeAOIDirectories(csvFilesPath,areas_of_interest)

print(optim_aoi)
optim_aoi[0]
for aoi in optim_aoi:
    print("aoi",aoi)
    # ------------------
    # Get best metrics per volumes
    # ------------------
    informative_aoi = copy.deepcopy(areas_of_interest)
    informative_aoi.remove(aoi)
    print(informative_aoi)

    bestParameter = dict()    
    l_temp = dict()
    l_roc = dict()
    # Dice - MCC
    for metric in optim_metrics:
        aoiCSVPath = f"{csvFilesPath}/{benchName}_{aoi}.csv" 
        aoiDF = loadCSVtoDF( aoiCSVPath )
        
        dfBestMetricPerVolume = getBestMetricPerVolume(aoiDF,metric)
        saveBestParameterSet(csvFilesPath,benchName,aoi,metric,dfBestMetricPerVolume) 

        BMPV = dfBestMetricPerVolume.groupby("SerieName")
        bestMPV = []
        for serieName,data in BMPV:
            index = np.argmax(data[metric].values)
            maxValue = np.max(data[metric].values)
            
            bestMPV.append(data.iloc[index,:])
        d = pd.DataFrame(bestMPV,columns=dfBestMetricPerVolume.columns)
        saveBestParameterSetSummary(csvFilesPath,benchName,aoi,metric,d)
        # -------------------
        # Get best mean metric
        # -------------------

        # get best mean metric per {volume - parameter set}
        aoiPicklePath = f"{csvFilesPath}/Pickle/{benchName}_{aoi}_Best_{metric}_per_volume.pkl" 
        aoiBestDF = loadPickleToDF( aoiPicklePath )
        
        dfMeanPerPS_part1,dfMeanPerPS_part2 = getMeanPerParameterSets(aoiBestDF)
        saveBestMeanPerParameterSet(csvFilesPath,benchName,aoi,metric,dfMeanPerPS_part1,dfMeanPerPS_part2)
        bestParameter[metric],values  = getValuesForMaxMetric(dfMeanPerPS_part2,f"mean_{metric}")
        # get mean CF metrics for best parameter
        meanCFvalues = getValuesForParameter(dfMeanPerPS_part1,bestParameter[metric])
 
        if( (meanCFvalues['mean_TN'].values[0]+meanCFvalues['mean_FP'].values[0]) == 0 ): # annoying case for bifurcation area, where FP and TN doesn't exists
            ratio_P_N = 1
        else:
            ratio_P_N = (meanCFvalues['mean_TP'].values[0]+meanCFvalues['mean_FN'].values[0]) / (meanCFvalues['mean_TN'].values[0]+meanCFvalues['mean_FP'].values[0])
       
        print("----")
        print(values)
        print(meanCFvalues)
        mean_TPR_optim = meanCFvalues["mean_sensitivity"].item()
        mean_FPR_optim = ( 1 - meanCFvalues["mean_specificity"].item() ) / ratio_P_N
        print(mean_TPR_optim,mean_FPR_optim)
        print("bobobobo")
        
        # AUC - ROC
        dfAUCPerVolume,dfROCPerVolume = getAUCandROCPerVolume(aoiDF)
        saveParameterSet(csvFilesPath,benchName,aoi,"AUC",dfAUCPerVolume)
        saveParameterSet(csvFilesPath,benchName,aoi,"ROC",dfROCPerVolume)
        # get mean AUC for best parameter 
        dfMeanAUC,dfMeanROC = getMeanAUCandROCperParameterSets(dfAUCPerVolume,dfROCPerVolume)
        meanAUCForBestPS = getValuesForParameter(dfMeanAUC,bestParameter[metric])["mean_AUC"].item()
        # get mean ROC for best parameter
        meanROCForBestPS = getValuesForParameter(dfMeanROC,bestParameter[metric])
        l2 =  formatLineToROCSummaryLine(aoi,metric,meanROCForBestPS["VolumeName"].item(),values["mean_Threshold"].item(),mean_TPR_optim,mean_FPR_optim,meanROCForBestPS["mean_TPR"].item(),meanROCForBestPS["mean_FPR"].item())

        print("l2",l2)
        l_roc[metric] = l2
        
        l = formatLineToSummaryLine(aoi,metric,values,meanAUCForBestPS,meanCFvalues["mean_sensitivity"].item(),meanCFvalues["std_sensitivity"].item())
        l_temp[metric] = l
        
    # -----------------------------------------------------------
    # compute mean Metric using best parameter set in optim aoi
    # -----------------------------------------------------------
    l_best_metrics = [] # for summary display
    l_best_roc_metrics = [] # for summary display
    for metric in optim_metrics:        
        # trick for getting best metric in the good place
        l_best_metrics.append( l_temp[metric] )
        l_best_roc_metrics.append( l_roc[metric] )
        # load best metrics per volume
        aoiPicklePath = f"{csvFilesPath}/Pickle/{benchName}_{aoi}_Best_{metric}_per_volume.pkl" 
        aoiBestMetricPerVolume = loadPickleToDF( aoiPicklePath )

        # TODO from best parameters, retrieve mean of metric in the other volumes
        
        for infoAOI in informative_aoi:
            print("info aoi",infoAOI)
            infoAOICSVPath = f"{csvFilesPath}/{benchName}_{infoAOI}.csv"
            dfInfoAOI = loadCSVtoDF(infoAOICSVPath)

            dfInfoMetric,dfInfoAUC,dfInfoROC = getInfoFromBestPS(aoiBestMetricPerVolume, dfInfoAOI,metric, bestParameter[metric] )
            
            dfInfoMeanMetric_part1,dfInfoMeanMetric_part2 = getMeanPerParameterSets(dfInfoMetric)

            saveMeanPerParameterSet(csvFilesPath,benchName,aoi,infoAOI,metric,dfInfoMeanMetric_part1,dfInfoMeanMetric_part2)
            meanCFvalues = getValuesForParameter(dfInfoMeanMetric_part1,bestParameter[metric])
            
            dfInfoMeanAUC,dfInfoMeanROC = getMeanAUCandROC(bestParameter[metric],dfInfoAUC,dfInfoROC)
            
            ps_name,values = getValuesForMaxMetric(dfInfoMeanMetric_part2,f"mean_{metric}")
            valuesROC = getValuesForParameter(dfInfoMeanROC,ps_name)

            if( (meanCFvalues['mean_TN'].values[0]+meanCFvalues['mean_FP'].values[0]) == 0 ): # annoying case for bifurcation area, where FP and TN doesn't exists
                ratio_P_N = 1
            else:
                ratio_P_N = (meanCFvalues['mean_TP'].values[0]+meanCFvalues['mean_FN'].values[0]) / (meanCFvalues['mean_TN'].values[0]+meanCFvalues['mean_FP'].values[0])


            mean_TPR_optim = meanCFvalues["mean_sensitivity"].item()
            mean_FPR_optim = (1 - meanCFvalues["mean_specificity"].item() ) / ratio_P_N

            l2 = formatLineToROCSummaryLine(infoAOI,metric,valuesROC["VolumeName"].item(),values["mean_Threshold"].item(),mean_TPR_optim,mean_FPR_optim,valuesROC["mean_TPR"].item(),valuesROC["mean_FPR"].item())
            l_best_roc_metrics.append(l2)
            
            l = formatLineToSummaryLine(infoAOI,metric,values,dfInfoMeanAUC["mean_AUC"].item(),meanCFvalues["mean_sensitivity"].item(),meanCFvalues["std_sensitivity"].item())
            l_best_metrics.append(l)
            
    # -----------------
    # Get Summary 
    # -----------------
    dfSummary = pd.DataFrame(l_best_metrics,columns = ["Region","ParameterSet","optimMetric","mean_MCC","std_MCC","mean_Dice","std_Dice","mean_snr","std_snr","mean_psnr","std_psnr","mean_AUC","mean_TPR","std_TPR"])
    saveSummary(csvFilesPath,dfSummary,benchName,aoi)

    dfROCSummary = pd.DataFrame(l_best_roc_metrics, columns = ["Region","ParameterSet","optimMetric","mean_Threshold","mean_TPR_optim","mean_FPR_optim","TPR","FPR"])
    #print(dfROCSummary)
    saveROCSummary(csvFilesPath,dfROCSummary,benchName,aoi)

    # ----------------
    # Make graphics
    # ----------------

    for metric in optim_metrics:
        plotGraphic(csvFilesPath,metric,dfSummary,benchName,aoi,informative_aoi)
        plotROCCurve(csvFilesPath,metric,dfROCSummary,benchName,aoi,informative_aoi) 
        
