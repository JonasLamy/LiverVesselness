import pandas as pd
import numpy as np
import sys
import matplotlib.pyplot as plt
import mplcursors
import heapq
from collections import deque


rankingMethod = sys.argv[2]
data = pd.read_csv(sys.argv[1])
print(data.columns)

noPlot=False
if( len(sys.argv)>3):
    noPlot=True

if 'Name' in data.columns:
    attribute ='Name'
else:
    attribute = 'AlgoID'

grp = data.groupby([attribute])


count = 0

topMetric = deque()
topMinRocDist = deque()
heap = []


######################

#Finding top parameter's sets depending

######################

topLength = 20
globMinDist = 10000
globMaxValue = 0
globMinValue = 100000000000

id = 0
for name,dataFiltered in grp:
    print(name)
    # computing ROC curve
    TruePositiveRate = dataFiltered['sensitivity'].values
    FalsePositiveRate = 1 - dataFiltered['specificity'].values
    # finding closest roc curve from ideal discriminator
    dist = np.sqrt( np.square(FalsePositiveRate) + np.square(1 - TruePositiveRate) )
    indexROC = np.argmin( dist)
    minRocDist = np.min(dist)

    #TODO avoid copy paste
    if( rankingMethod == "ROC"): # ROC is min distance from the top left corner of the graph (0,1)
        # using heap instead of queue for top
        if( len(heap) < topLength ):
            heapq.heappush(heap, (minRocDist,id,dataFiltered.iloc[indexROC,:]) )
        else:
            heapq.heappushpop(heap, (minRocDist,id,dataFiltered.iloc[indexROC,:]) )
        
    elif( rankingMethod == "FP" or rankingMethod == "FN") : # interested in minimizing these results
        index = np.argmin(dataFiltered[rankingMethod].values)
        minValue = np.min(dataFiltered[rankingMethod].values)
        
        if( len(heap) < topLength ):
            heapq.heappush(heap, (minValue,id,dataFiltered.iloc[index,:]) )
        else:
            heapq.heappushpop(heap, (minValue,id,dataFiltered.iloc[index,:]) )
        # printing resultats in terminal and preparing# printing resultats in terminal and preparing
    else: # interested in maximizing those values (MCC, dice, sensitivity, specificity, precision, accuracy)
        index = np.argmax(dataFiltered[rankingMethod].values)
        maxValue = np.max(dataFiltered[rankingMethod].values)
        # Python heap is a min heap, using minus values to emulate max heap
        if( len(heap) < topLength ):
            heapq.heappush(heap, (-maxValue,id,dataFiltered.iloc[index,:]) )
        else:
            heapq.heappushpop(heap, (-maxValue,id,dataFiltered.iloc[index,:]) )
    id += 1
        
print("--------------")
topList = []
orderedDisplayList = []

#################

# poping results from the stack

################

while( heap ):
    (d,i,e) = heapq.heappop(heap)

    if( rankingMethod =="ROC" or rankingMethod == "FP" or rankingMethod == "FN" ):
        topList.append( e )
        orderedDisplayList.append((d,i,e))
    else: # max queue was used
        topList.append( e )
        orderedDisplayList.append((-d,i,e))

############

# display results on terminal

###########
        
rank=len(orderedDisplayList)
for d,i,e in reversed(orderedDisplayList):
    print("-----------")
    print("rank:",rank)
    if( rankingMethod =="ROC" or rankingMethod == "FP" or rankingMethod == "FN" ):
        print("distance from closest point to top point(0,1) for ROC:",d)
    else:
        print("metric score:",d)
    
    print(e)
    rank-=1

##########

# display results on plot

##########
    
if(not noPlot):
    fig,axes = plt.subplots(2,3)
    ax = axes[0,0]
    ax1 = axes[0,1]
    ax2 = axes[0,2]
    ax3 = axes[1,0]
    ax4 = axes[1,1]
    ax5 = axes[1,2]

    ax.set_title("click to see lines label")

    for top in topList:
        name = top[attribute]
    
        dataFiltered = grp.get_group(name)
        
        # computing ROC curve
        TruePositiveRate = dataFiltered['sensitivity'].values
        FalsePositiveRate = 1 - dataFiltered['specificity'].values
        
        ax.plot(FalsePositiveRate,TruePositiveRate,marker="x",label=f"${name}$")
        ax.set_xlabel('False Positive Rate')
        ax.set_ylabel('True Postive Rate')
        ax.set_ylim(0,1)
        ax.set_xlim(0,1)

        # MCC plot
        ax1.plot(np.linspace(1,0,101),dataFiltered['MCC'].values,label=f"${name}$")
        ax1.set_xlabel('threshold')
        ax1.set_ylabel('MCC')
        ax1.set_ylim(-1,1)
        ax1.set_xlim(1,0)
    
        # Dice plot
        ax2.plot(np.linspace(1,0,101),dataFiltered['Dice'].values,label=f"${name}$")
        ax2.set_xlabel('threshold')
        ax2.set_ylabel('dice')
        ax2.set_ylim(0,1)
        ax2.set_xlim(1,0)
        
        # Precision recall plot
        ax3.plot(dataFiltered['sensitivity'].values,dataFiltered['precision'].values,label=f"${name}$")
        ax3.set_xlabel('recall')
        ax3.set_ylabel('precision')
        ax3.set_xlim(0,1)
        ax3.set_ylim(0,1)
        
        # sensitivity plot
        ax4.plot(np.linspace(1,0,101),dataFiltered['sensitivity'].values,label=f"${name}$")
        ax4.set_xlabel('threshold')
        ax4.set_ylabel('sensitivity')
        ax4.set_xlim(1,0)
    
        # specificity plot
        ax5.plot(np.linspace(1,0,101),dataFiltered['specificity'].values,label=f"${name}$")
        ax5.set_xlabel('threshold')
        ax5.set_ylabel('specificity')
        ax5.set_xlim(1,0)

    plt.legend(loc='best',ncol=4)
    mplcursors.cursor().connect(
        "add", lambda sel: sel.annotation.set_text(sel.artist.get_label()+"\n x="+str(sel.target[0]) +"\n y=" + str(sel.target[1])))
    plt.show()
