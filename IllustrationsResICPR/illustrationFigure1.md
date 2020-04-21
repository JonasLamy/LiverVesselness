

# Image sample: 3Dircadb1.12


* Generation des mesh

** Mesh du foie
      
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii --inputMin 0 --inputMax 1 -t integer -o liverMaskIrcad1.12.vol 
   volBoundary2obj -i liverMaskIrcad1.12.vol -o liverMaskIrcad1.12.off --customDiffuse 200 200 200 20


** Mesh vérité terrain

   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/vesselsIso.nii --inputMin 0 --inputMax 1 -t integer -o refVesselIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii   
   volBoundary2obj -i  refVesselIrcad1.12.vol  -o  refVesselIrcad1.12.off  --customDiffuse 20 20 200 255    
   volBoundary2obj -i  refVesselIrcad1.12.vol  -o  refVesselTransIrcad1.12.off  --customDiffuse 20 20 200 50 
   
** Mesh dilate:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/dilatedVesselsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o dilateIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii    
   volBoundary2obj -i  dilateIrcad1.12.vol  -o  dilateIrcad1.12.off  --customDiffuse 200 20 20 25    


** Mesh bifu:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/bifurcationsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o bifuIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii
   volBoundary2obj -i  bifuIrcad1.12.vol  -o  bifuIrcad1.12.off  --customDiffuse 200 20 20 100    

** Display comb image (c)
   
   meshViewer -i liverMaskIrcad1.12.off refVesselIrcad1.12.off                                       

** Display comb image (d)

   meshViewer -i refVesselIrcad1.12.off  dilateIrcad1.12.off  



** Display comb image (d)

   meshViewer -i refVesselIrcad1.12.off  bifuIrcad1.12.off  
