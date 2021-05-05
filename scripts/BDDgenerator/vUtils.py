
import itk
import sys

# reading data

def readDicomSerie(dirName):
    PixelType = itk.ctype('unsigned short')
    Dimension = 3

    ImageType = itk.Image[PixelType, Dimension]

    namesGenerator = itk.GDCMSeriesFileNames.New()
    namesGenerator.SetUseSeriesDetails(True)
    namesGenerator.AddSeriesRestriction("0008|0021")
    namesGenerator.SetGlobalWarningDisplay(False)
    namesGenerator.SetDirectory(dirName)

    seriesUID = namesGenerator.GetSeriesUIDs()

    if len(seriesUID) < 1:
        print('No DICOMs in: ' + dirName)
        sys.exit(1)

    print('The directory: ' + dirName)
    print('Contains the following DICOM Series: ')
    for uid in seriesUID:
        print(uid)

    fileNames = namesGenerator.GetFileNames(seriesUID[0]) # only one serie

    reader = itk.ImageSeriesReader[ImageType].New()
    dicomIO = itk.GDCMImageIO.New()
    reader.SetImageIO(dicomIO)
    reader.SetFileNames(fileNames)
    reader.ForceOrthogonalDirectionOff()
    reader.Update()
    img = reader.GetOutput()
    return  img


def readData(fileName, isDicom):
    if(isDicom):
        return readDicomSerie(fileName)
    return itk.imread(fileName)