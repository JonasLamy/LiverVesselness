import sys

scaleMin = int( sys.argv[1] )
factor = float(sys.argv[2] )
nbScales = int( sys.argv[3] )

for i in range(1,nbScales+1):
    print("scale:"+str(i),int(scaleMin * factor**i))
