analyseScript=PathToFolder/LiverVesselness/scripts/analyse.py

echo "creating image folder"
mkdir images
echo "creating csv fileList"
ls *.csv > csvList.txt
echo "creating latex file"
python3 $analyseScript csvList.txt > results.tex

pdflatex results.tex
pdflatex results.tex
echo "result.pdf generation done"
