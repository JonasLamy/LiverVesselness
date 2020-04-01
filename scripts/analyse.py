# data management
import pandas as pd
import numpy as np
# system arguments
import sys
# plot display
import matplotlib
import matplotlib.pyplot as plt
import mplcursors
# results sorting
import heapq
from collections import deque
# input file sorting
import re

class CurvesData:
    def __init__(self,FPR,TPR,MCC,Dice):
        self.FPR = FPR
        self.TPR = TPR
        self.MCC = MCC
        self.Dice = Dice

class AnalyseResults2:
    def __init__(self,csvFileList):
        self.csvData = []
        self.csvFileList = csvFileList
        # reading csv files
        for csvFile in csvFileList:
            data = pd.read_csv(csvFile)
            self.csvData.append( data )

        # adding once and for all ROC distance

        for csvFile in self.csvData:
            TruePositiveRate = csvFile['sensitivity']
            FalsePositiveRate = 1 - csvFile['specificity']
        # finding closest roc curve from ideal discriminator
            dist = np.sqrt( np.square(FalsePositiveRate) + np.square(1 - TruePositiveRate) )
        
            csvFile['ROCDist'] = dist
        
    def setTopListLength(self,length):
        self.topLength = length

    def BestMetricsPerVolume(self,dataIndex,rankingMethod):
        if 'Name' in self.csvData[dataIndex].columns:
            attribute = 'Name'
        else:
            attribute = 'AlgoID'

        heap = []
        topLength = self.topLength
        orderedDisplayList = []

        id = 0
        grp = self.csvData[dataIndex].groupby(['SerieName'])


        orderedDisplayList = []
        for serieName,dataGroup in grp:
            volumeName = serieName.split("/")[1].replace("_","\_")
            # computing ROC curve
            if(rankingMethod == "ROCDist"):
                dataFiltered = dataGroup.loc[dataGroup[rankingMethod].idxmin()].copy()
            else:
                dataFiltered = dataGroup.loc[dataGroup[rankingMethod].idxmax()].copy()
            dataFiltered['Volume'] = volumeName
            # computing ROC curve
            orderedDisplayList.append(dataFiltered)
        
        return orderedDisplayList



    def avgMetric(self,dataIndex,rankingMethod):
        if 'Name' in self.csvData[dataIndex].columns:
            attribute = 'Name'
        else:
            attribute = 'AlgoID'

        heap = []
        topLength = self.topLength

        id = 0
        grp = self.csvData[dataIndex].groupby([attribute])

        for name,dataGroup in grp:
            dataFiltered = dataGroup.groupby(["Threshold"],as_index=False).mean()

            TPR = dataFiltered['sensitivity']
            FPR = 1 - dataFiltered['specificity']

            curvesData = CurvesData(FPR,TPR,dataFiltered['MCC'].iloc[::-1],dataFiltered['Dice'].iloc[::-1])

            dataFiltered.insert(0,"Name",name)
            
            stdDev = dataGroup.groupby(["Threshold"],as_index=False).std()

            dataFiltered.insert(0,"stdDevMCC",stdDev["MCC"])
            dataFiltered.insert(0,"stdDevDice",stdDev["Dice"])

            indexROC = np.argmin( np.array( dataFiltered["ROCDist"]) )
            minRocDist = np.min(dataFiltered["ROCDist"])
            
            if( rankingMethod == "ROCDist"): # ROC is min distance from the top left corner of the graph (0,1)
                # using heap instead of queue for top
                heapq.heappush(heap, (minRocDist,id,dataFiltered.iloc[indexROC,:],curvesData) )
                
            elif( rankingMethod == "FP" or rankingMethod == "FN" or rankingMethod =="Hausdorff") : # interested in minimizing these results
                index = np.argmin(dataFiltered[rankingMethod].values)
                minValue = np.min(dataFiltered[rankingMethod].values)
                
                heapq.heappush(heap, (minValue,id,dataFiltered.iloc[index,:],curvesData) )
                # printing resultats in terminal and preparing# printing resultats in terminal and preparing
            else: # interested in maximizing those values (MCC, dice, sensitivity, specificity, precision, accuracy)
                index = np.argmax(dataFiltered[rankingMethod].values)
                maxValue = np.max(dataFiltered[rankingMethod].values)
                # Python heap is a min heap, using minus values to emulate max heap
                heapq.heappush(heap, (-maxValue,id,dataFiltered.iloc[index,:],curvesData) )
            id += 1

            

        orderedDisplayList = []

        #################

        # poping results from the stack

        ################
        c = 0
        while( heap ):
            (d,i,e,curvesData) = heapq.heappop(heap)
            c += 1
            if( rankingMethod =="ROCDist" or rankingMethod == "FP" or rankingMethod == "FN" or rankingMethod == "Hausdorff" ):
                orderedDisplayList.append((d,i,e,curvesData))
            else: # max queue was used
                orderedDisplayList.append((-d,i,e,curvesData))
            if(c>=topLength):
                break

        return orderedDisplayList

    def computeMeanMetrics(self,rankingMethod):

        for i in range(0,len(self.csvData) ):
            result = self.avgMetric(i,rankingMethod)

            fig = plt.figure(dpi=100 )
            fig.set_size_inches(w=10,h=6)
            axes = fig.subplots(1,3)
            fig.suptitle('Mean metrics : ' + rankingMethod)

            templateHeader = "{0:50} & {1:20} & {2:10} & {3:20} & {4:20} & {5:10} \\\\"
            template = "{0:50} & {1:20} & {2:10} & {3:20} & {4:20} & {5:10} \\\\"

            print("\\begin{center}")
            print("\\begin{tabular}{l l l l l l}")
            print("\\hline")
            print( templateHeader.format("BenchmarkName","Parameters","Threshold","MCC","Dice","ROCDist") )
            #print("\\hline")

            BenchmarkName = self.csvFileList[i].replace("_","\_")
            for d,j,infos,curvesData in result:
                Name = infos["Name"].replace("_","\_")
                Threshold = infos["Threshold"]
                
                MCC =  "{0:5.4f} $\pm$ {1:2.5f}".format( infos["MCC"],infos["stdDevMCC"] )
                Dice = "{0:5.4f} $\pm$ {1:2.5f}".format( infos["Dice"],infos["stdDevDice"] )
                ROC =  "{:5.4f}".format( infos["ROCDist"] ) 


                #########################
                #   printing the results
                #########################
                # making plot of the thing and saving it
                ax0 = axes[0] # ROC
                #ax0.set_position([0, 0, 0.5, 1])
                ax1 = axes[1] # MCC
                #ax0.set_position([0.5, 0.5, 1, 1])
                ax2 = axes[2] # Dice
                #ax2.set_position([0.66, 0, 0.33, 1])

                # ROC plot
                ax0.plot(curvesData.FPR,curvesData.TPR,marker="x",label=f"${infos['Name']}$")
                ax0.set_xlabel('False Positive Rate')
                ax0.set_ylabel('True Postive Rate')
                ax0.set_ylim(0,1)
                ax0.set_xlim(0,1)
                ax0.title.set_text=("ROC curve")

                # MCC plot
                ax1.plot(np.linspace(1,0,curvesData.MCC.size),curvesData.MCC.values,label="_nolegend_")
                ax1.set_xlabel('threshold')
                ax1.set_ylabel('MCC')
                ax1.set_ylim(-1,1)
                ax1.set_xlim(1,0)
                ax1.title.set_text=("MCC")
                # Dice plot
                ax2.plot(np.linspace(1,0,curvesData.Dice.size),curvesData.Dice.values,label="_nolegend_")
                ax2.set_xlabel('threshold')
                ax2.set_ylabel('dice')
                ax2.set_ylim(0,1)
                ax2.set_xlim(1,0)
                ax2.title.set_text=("Dice")

                print(template.format(BenchmarkName,Name,Threshold,MCC,Dice,ROC))
            print("\\end{tabular}")
            print("\\end{center}")

            dirName = "images/"
            imgName = self.csvFileList[i].split('.')[0] + "_first_"+rankingMethod + ".pdf"
        
            print("\\begin{figure}[ht]")
            print("\\includegraphics[width=\linewidth,]{"+dirName+imgName+"}")
            print("\\caption{" + BenchmarkName.split('.')[0] + " "+rankingMethod + "}")
            print("\\end{figure}")
            print("\n")
            print("\\FloatBarrier")

            fig.legend(loc='lower center',ncol=4)        
            plt.savefig(dirName+imgName,bbox_inches="tight")
            plt.close()

    def computeTopMeanMetrics(self,rankingMethod):
        templateHeader = "{0:50} & {1:20}  & {2:10} & {3:20} & {4:10} & {5:10} \\\\"
        template = "{0:50} & {1:20} & {2:10} & {3:20} & {4:10} & {5:10} \\\\"

        fig = plt.figure(dpi=100 )
        fig.set_size_inches(w=10,h=6)
        axes = fig.subplots(1,3)
        fig.suptitle('Top Mean metrics : ' + rankingMethod)

        print("\\begin{center}")
        print("\\begin{tabular}{l l l l l l}")
        print("\\hline")
        print( templateHeader.format("BenchmarkName","Parameters","Threshold","MCC","Dice","ROCDist") )
        #print("\\hline")

        for i in range(0,len(self.csvData) ):
            
            result = self.avgMetric(i,rankingMethod)

            if(len(result) == 0):
                raise Exception("error file empty:" + self.csvFileList[i])
            d,id,infos,curvesData = result[0]
            BenchmarkName = self.csvFileList[i].replace("_","\_")
            curvesName = re.search( r'[a-z,A-Z]+ScalesSearch',self.csvFileList[i]).group(0).split("Scales")[0]
            
            Name = infos["Name"].replace("_","\_")
            Threshold = infos["Threshold"]
            MCC =  "{0:5.4f} $\pm$ {1:2.5f}".format( infos["MCC"],infos["stdDevMCC"] )
            Dice = "{0:5.4f} $\pm$ {1:2.5f}".format( infos["Dice"],infos["stdDevDice"] )
            ROC =  "{:5.4f}".format( infos["ROCDist"] ) 

            print(template.format(BenchmarkName,Name,Threshold,MCC,Dice,ROC))


            #########################
            #   printing the results
            #########################
            # making plot of the thing and saving it
            ax0 = axes[0] # ROC
            #ax0.set_position([0, 0, 0.5, 1])
            ax1 = axes[1] # MCC
            #ax0.set_position([0.5, 0.5, 1, 1])
            ax2 = axes[2] # Dice
            #ax2.set_position([0.66, 0, 0.33, 1])

            # ROC plot
            ax0.plot(curvesData.FPR,curvesData.TPR,marker="x",label=f"${curvesName}$")
            ax0.set_xlabel('False Positive Rate')
            ax0.set_ylabel('True Postive Rate')
            ax0.set_ylim(0,1)
            ax0.set_xlim(0,1)
            ax0.title.set_text=("ROC curve")

            # MCC plot
            ax1.plot(np.linspace(1,0,curvesData.MCC.size),curvesData.MCC.values,label="_nolegend_")
            ax1.set_xlabel('threshold')
            ax1.set_ylabel('MCC')
            ax1.set_ylim(-1,1)
            ax1.set_xlim(1,0)
            ax1.title.set_text=("MCC")

            # Dice plot
            ax2.plot(np.linspace(1,0,curvesData.Dice.size),curvesData.Dice.values,label="_nolegend_")
            ax2.set_xlabel('threshold')
            ax2.set_ylabel('dice')
            ax2.set_ylim(0,1)
            ax2.set_xlim(1,0)
            ax2.title.set_text=("Dice")

        print("\\end{tabular}")
        print("\\end{center}")
        print("\n")

        dirName = "images/"
        imgName = self.csvFileList[i].split('.')[0] + "_"+rankingMethod + ".pdf"
    
        print("\\begin{figure}[ht]")
        print("\\includegraphics[width=\linewidth,]{"+dirName+imgName+"}")
        print("\\caption{" + BenchmarkName.split('.')[0] + " "+rankingMethod + "}")
        print("\\end{figure}")
        print("\n")
        print("\\FloatBarrier")

        fig.legend(loc='lower center',ncol=4)        
        plt.savefig(dirName+imgName,bbox_inches="tight")
        plt.close()
        

    def computeBestMetricsPerVolume(self,rankingMethod):
        templateHeader = "{0:50} & {1:20} & {2:20} & {3:10} & {4:10} & {5:10} & {6:10} \\\\"
        template = "{0:50} & {1:20} & {2:20} & {3:10} & {4:10} & {5:10} & {6:10} \\\\"

        for i in range(0,len(self.csvData) ):
            
            result = self.BestMetricsPerVolume(i,rankingMethod)

            if(len(result) == 0):
                raise Exception("error file empty:" + self.csvFileList[i])

            print("\\begin{center}")
            print("\\begin{tabular}{l l l l l l l}")
            print("\\hline")
            print( templateHeader.format("BenchmarkName","Volume","Name","Threshold","MCC","Dice","ROCDist") )

            BenchmarkName = self.csvFileList[i].replace("_","\_")
            for infos in result:
                Name = infos["Name"].replace("_","\_")
                Threshold = infos["Threshold"]
                Volume = infos["Volume"]
                MCC =  "{0:5.4f}".format( infos["MCC"] )
                Dice = "{:5.4f}".format( infos["Dice"] )
                ROC =  "{:5.4f} ".format( infos["ROCDist"] ) 

                print(template.format(BenchmarkName,Volume,Name,Threshold,MCC,Dice,ROC))
            print("\n") 
            print("\\end{tabular}")
            print("\\end{center}")

def analyseMask(file_list,topListLength):
    for rankingMethod in ["MCC","Dice","ROCDist"]:
        print("\\subsection{RANKING METHOD :" + rankingMethod + "}")
        
        analyser2 = AnalyseResults2( file_list )
        analyser2.setTopListLength(topListLength)
        print()
        print("\\subsubsection{Compute first mean metric}")
        
        analyser2.computeTopMeanMetrics(rankingMethod)

        print("\\subsubsection{Top "+ str(topListLength) +" mean Metric}")
        analyser2 = AnalyseResults2( file_list )
        analyser2.setTopListLength(topListLength)
        analyser2.computeMeanMetrics(rankingMethod)

def analyseMask_annexe(file_List,topListLength):
    for rankingMethod in ["MCC","Dice","ROCDist"]:
        analyser2 = AnalyseResults2( file_List )
        analyser2.setTopListLength(topListLength)
        print("\\subsubsection{In-depth metric, Ranking Method :"+ rankingMethod +"}")
        analyser2.computeBestMetricsPerVolume(rankingMethod)


csvFileListFile = sys.argv[1]

fileList = []
with open(csvFileListFile) as f:
    fileList = f.read().splitlines()

WholeFileList=[]
BifurcationFileList=[]
DilatedVesselsFileList=[]


patternBifurcations = re.compile("bifurcations.csv")
patternDilatedVessels = re.compile("dilatedVessels.csv") 
for f in fileList:
    if( patternBifurcations.search(f) ):
        BifurcationFileList.append(f)
        continue
    if( patternDilatedVessels.search(f) ):
        DilatedVesselsFileList.append(f)
        continue
    WholeFileList.append(f)
# for each algorithm 
# get mean metrics value for each threshold + std deviation on whole set
# get best metric value for each volume

matplotlib.use("pgf")
matplotlib.rcParams.update({
    "pgf.texsystem":"pdflatex",
    'font.family':'serif',
    'text.usetex':True,
    'pgf.rcfonts':False
})

print("\\documentclass{article}")
print("\\usepackage{pdflscape}")
print("\\usepackage{graphicx}")
print("\\usepackage{placeins}")
print("\\usepackage{hyperref}")
print("\\hypersetup{colorlinks=true,linktoc=all,linkcolor=blue}")
print("\\begin{document}")
print("\\tableofcontents")
print("\\begin{landscape}")
listLength = 5

print("\\section{Whole Liver metrics}")
analyseMask(WholeFileList,listLength)

print("\\section{Vessels neighbourhood metrics}")
analyseMask(DilatedVesselsFileList,listLength)
print("\\section{Bifurcations metrics}")
analyseMask(BifurcationFileList,listLength)

print("\\section{Annexe}")
print("\\section{Whole Liver metrics}")
analyseMask_annexe(WholeFileList,listLength)
print("\\section{Vessels neighbourhood metrics}")
analyseMask_annexe(DilatedVesselsFileList,listLength)
print("\\section{Bifurcations metrics}")
analyseMask_annexe(BifurcationFileList,listLength)

print("\\end{landscape}")
print("\\end{document}")


