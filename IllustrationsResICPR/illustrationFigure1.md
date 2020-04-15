

* Image sample: 3Dircadb1.12

** Generation des mesh resolution 0.5

*** Mesh du foie
      
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii --inputMin 0 --inputMax 1 -t integer -o liverMaskIrcad1.12.vol 
   volBoundary2obj -i liverMaskIrcad1.12.vol -o liverMaskIrcad1.12.off --customDiffuse 230 20 20 10


*** Mesh vérité terrain

   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/vesselsIso.nii --inputMin 0 --inputMax 1 -t integer -o refVesselIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii   
   volBoundary2obj -i  refVesselIrcad1.12.vol  -o  refVesselIrcad1.12.off  --customDiffuse 20 20 200 255    
   volBoundary2obj -i  refVesselIrcad1.12.vol  -o  refVesselTransIrcad1.12.off  --customDiffuse 20 20 200 50 
   
*** Mesh dilate:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/dilatedVesselsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o dilateIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii    
   volBoundary2obj -i  dilateIrcad1.12.vol  -o  dilateIrcad1.12.off  --customDiffuse 200 20 20 25    


*** Mesh bifu:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/bifurcationsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o bifuIrcad1.12.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii
   volBoundary2obj -i  bifuIrcad1.12.vol  -o  bifuIrcad1.12.off  --customDiffuse 200 20 20 100    

*** Display comb image (c)
   
   meshViewer -i liverMaskIrcad1.12.off refVesselIrcad1.12.off                                       

*** Display comb image (d)

   meshViewer -i refVesselIrcad1.12.off  dilateIrcad1.12.off  



*** Display comb image (d)

   meshViewer -i refVesselIrcad1.12.off  bifuIrcad1.12.off  




*** vb rician 5 10 20 

Prender les dernières données DATA11 ou DATA12 avec beaucoup de bifurcations.


** Generation des mesh resolution 1x1x1

*** Mesh du foie
      
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii --inputMin 0 --inputMax 1 -t integer -o liverMaskIrcad1.12_1x1x1.vol 
   volBoundary2obj -i liverMaskIrcad1.12_1x1x1.vol -o liverMaskIrcad1.12_1x1x1.off --customDiffuse 20 20 230 255



*** Mesh vérité terrain

   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/vesselsIso.nii --inputMin 0 --inputMax 1 -t integer -o refVesselIrcad1.12_1x1x1.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii   
   volBoundary2obj -i  refVesselIrcad1.12_1x1x1.vol  -o  refVesselIrcad1.12_1x1x1.off  --customDiffuse 20 20 200 255    
   volBoundary2obj -i  refVesselIrcad1.12_1x1x1.vol  -o  refVesselTransIrcad1.12_1x1x1.off  --customDiffuse 20 20 200 50 
   
*** Mesh dilate:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/dilatedVesselsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o dilateIrcad1.12_1x1x1.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii    
   volBoundary2obj -i  dilateIrcad1.12_1x1x1.vol  -o  dilateIrcad1.12_1x1x1.off  --customDiffuse 200 20 20 25    


*** Mesh bifu:
   itk2vol -i /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/bifurcationsMaskIso.nii --inputMin 0 --inputMax 1 -t integer -o bifuIrcad1.12_1x1x1.vol --maskImage  /Users/kerautre/EnCours/LiverVesselness/build/bin/ircad_iso/3Dircadb1.12/maskedLiverIso.nii
   volBoundary2obj -i  bifuIrcad1.12_1x1x1.vol  -o  bifuIrcad1.12_1x1x1.off  --customDiffuse 200 20 20 100    

*** Display comb image (c)
   
   meshViewer -i liverMaskIrcad1.12_1x1x1.off refVesselIrcad1.12_1x1x1.off                                       

*** Display comb image (d)

   meshViewer -i refVesselIrcad1.12_1x1x1.off  dilateIrcad1.12_1x1x1.off  



*** Display comb image (d)

   meshViewer -i refVesselIrcad1.12_1x1x1.off  bifuIrcad1.12_1x1x1.off  

   meshViewer -i refVesselTransIrcad1.12_1x1x1.off  bifuOpaqueIrcad1.12_1x1x1.off










* Image sample VascuSynth 

** Generation des mesh resolution 1x1x1:


*** Mesh References
    itk2vol -i /Users/kerautre/Downloads/vascu_2013/Group10/data12/gt.nii  --inputMin 0 --inputMax 1 -t integer -o vascuRef_1x1x1.vol    
    volBoundary2obj -i liverMaskIrcad1.12_1x1x1.vol -o  vascuRef_1x1x1.off --customDiffuse 20 20 230 255
    volBoundary2obj -i  vascuRef_1x1x1.vol -o vascuRefTransp_1x1x1.off --customDiffuse 20 20 200 50
  


*** Mesh dilate
    itk2vol -i /Users/kerautre/Downloads/vascu_2013/Group10/data12/gtDilated.nii  --inputMin 0 --inputMax 1 -t integer -o dilate_1x1x1.vol        
    volBoundary2obj -i  dilate_1x1x1.vol  -o  dilate_1x1x1.off  --customDiffuse 200 20 20 25    


*** Mesh bifucations
    itk2vol -i /Users/kerautre/Downloads/vascu_2013/Group10/data12/bifurcationGT.nii  --inputMin 0 --inputMax 1 -t integer -o bifu_1x1x1.vol        
    volBoundary2obj -i  bifu_1x1x1.vol  -o  bifu_1x1x1.off  --customDiffuse 200 20 20 255    



*** Visu

 meshViewer -i vascuRef_1x1x1.off
 meshViewer -i vascuRef_1x1x1.off dilate_1x1x1.off
 meshViewer -i vascuRefTransp_1x1x1.off bifu_1x1x1.off
