from pathlib import Path
import os
import glob
import sys



pathDirectory = "patched_ircad_ps_mean_results/agregatingFrangi"
pathResults = "patched_ircad_ps_mean_results/ircad_PS_Frangi"

aoi_of_interest = ["Organ","VN","Vsmall","Vmedium","Vlarge","Bifurcations"]
# List
pathList = sorted(glob.glob(f"{pathDirectory}/ircad_PS_*"))

csvParts = dict()
for path in pathList:
    benchName = path.split("/")[-1] # fileName
    splitedBenchName = benchName.split("_")
    dataBaseName = splitedBenchName[0]
    benchTypeName = splitedBenchName[1]
    methodName = splitedBenchName[2]
    volumeName = splitedBenchName[3]
    if(methodName not in csvParts.keys()):
        csvParts[methodName] = []
        csvParts[methodName].append(path)
    else:
        csvParts[methodName].append(path)

print("----")
print("dict")
for key in csvParts:
    print(csvParts[key])
print("-----")
        
# for each key, make a csv file
for aoi in aoi_of_interest:
    for key in csvParts:
        pathForSavingFiles = f"{pathResults}/{dataBaseName}_{benchTypeName}_{key}/csv" 
        Path(pathForSavingFiles).mkdir(parents=True, exist_ok=True)
        
        benchName = csvParts[key][0].split("/")[-1] # fileName
        splitedBenchName = benchName.split("_")
        dataBaseName = splitedBenchName[0]
        benchTypeName = splitedBenchName[1]
        methodName = splitedBenchName[2]
        volumeName = splitedBenchName[3]
        
        file = pathForSavingFiles + f"/{dataBaseName}_{benchTypeName}_{key}_{aoi}.csv"

        firstPart = csvParts[key][0]
        step1 = f"cat {firstPart}/csv/{dataBaseName}_{benchTypeName}_{key}_{volumeName}_{aoi}.csv > {file}"

        os.system(step1)

        print(f"file: {file}")
        print(f"part 0:{step1}")
        
        for i in range(1,len(csvParts[key])):
            benchName = csvParts[key][i].split("/")[-1] # fileName
            splitedBenchName = benchName.split("_")
            dataBaseName = splitedBenchName[0]
            benchTypeName = splitedBenchName[1]
            methodName = splitedBenchName[2]
            volumeName = splitedBenchName[3]

            otherParts = csvParts[key][i] + f"/csv/{dataBaseName}_{benchTypeName}_{key}_{volumeName}_{aoi}.csv"

            step2 = f"tail -n +2 {otherParts} >> {file}"
            print(f"part {i}:{step2}")
            os.system(step2)
        print("----")
    
    
