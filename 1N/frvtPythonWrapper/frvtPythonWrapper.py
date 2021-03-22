from ctypes import *
import ctypes
import cv2
import numpy as np
import os


class FRVTLibraryLoader:
    def __init__(self):
        self.handle = cdll.LoadLibrary("libfrvtPlainCWrapper.so")
        self.implPtr = c_void_p()

    def loadLibrary(self,libName,libDir=None):
        if libDir is None:
            symbolName = "_ZN7FRVT_1N9Interface17getImplementationEv"
        else:
            fullLibName = os.path.join(libDir,libName)
            stream = os.popen(f'nm -D {fullLibName} |grep getImplementation')
            output = stream.read()
            symbolName = output[output.index("_"):].strip()

        return self.handle.loadLibrary(libName.encode(),symbolName.encode(),ctypes.byref(self.implPtr))
    

class FRVTMultiface:
    def __init__(self,libraryLoader,image):
        self.handle = c_void_p()
        self.libraryLoader = libraryLoader
        self.libraryLoader.handle.createMultifaceHandle(ctypes.byref(self.handle))
        self.libraryLoader.handle.addImageToMultiface(image.handle,self.handle)
    def __del__(self): 
        self.libraryLoader.handle.destroyMultifaceHandle(ctypes.byref(self.handle))

class FRVTCandidate:
    def __init__(self, isAssigned, templateId, similarityScore):
        self.isAssigned = isAssigned
        self.templateId = templateId
        self.similarityScore = similarityScore
    def __str__(self):
        repr = f"FRVTCandidate(isAssigned: {str(self.isAssigned)}, templatedId: {str(self.templateId)}, similarityScore:{str(self.similarityScore)})"
        return repr


class FRVTCandidateList(list):
    def __init__(self,libraryLoader,candidateListLength):
        self.handle = c_void_p()
        self.libraryLoader = libraryLoader
        self.length = candidateListLength
        libraryLoader.handle.createCandidateListHandle(ctypes.byref(self.handle))
    def __getitem__(self,key):
        isAssigned = c_int(0)
        templateId = ctypes.create_string_buffer(1000)
        similarityScore = c_double(0)
        self.libraryLoader.handle.getCandidate(self.handle, c_int(key), ctypes.byref(isAssigned),templateId, ctypes.byref(similarityScore))
        return FRVTCandidate(isAssigned.value,templateId.value.decode(),similarityScore.value)
    def __str__(self):
        repr = []
        for elem in range(self.length):
            repr.append(str(self[elem]))
        return str(repr)
    def toList(self):
        candidates = []
        for elem in range(self.length):
            candidates.append(self[elem])
        return candidates
    def __del__(self):
        self.libraryLoader.handle.destroyCandidateListHandle(ctypes.byref(self.handle))
        self.handle = None



class FRVTImage:
    def __init__(self, libraryLoader, inputType,switchColorChannelsToRGB=True):
        if isinstance(inputType,str):
            imageAsNumpy = cv2.imread(inputType)
        elif isinstance(inputType,np.ndarray):
            imageAsNumpy = inputType
        self.libraryHandle = libraryLoader.handle
        self.handle = c_void_p()
        if switchColorChannelsToRGB:
            imageAsNumpy_rgb = imageAsNumpy[:, :, ::-1]
        else:
            imageAsNumpy_rgb = imageAsNumpy
        self.libraryHandle.createImageHandle(ctypes.byref(self.handle))
        self.libraryHandle.loadImage(self.handle,c_int(imageAsNumpy_rgb.shape[1]),c_int(imageAsNumpy_rgb.shape[0]),c_int(imageAsNumpy_rgb.shape[2]*8),imageAsNumpy_rgb.tobytes(),c_int(0))
    def __del__(self):
        self.libraryHandle.destroyImageHandle(ctypes.byref(self.handle))
        self.handle = None

class FRVTWrapper:
    def __init__(self,libraryLoader):
        self.libraryLoader = libraryLoader
    
    def initializeTemplateCreation(self):
        configDir = "/mnt/d/coding/frvtBinaries/config"
        return self.libraryLoader.handle.initializeTemplateCreation(self.libraryLoader.implPtr,configDir.encode(),c_int(2))
    def encodeTemplate(self,multiFace):
        templateDataSize = 512
        templateDataSize_cint = c_int(templateDataSize)
        isLeftAssigned = c_bool(False)
        isRightAssigned = c_bool(False)
        leftX = c_int(0)
        rightX = c_int(0)
        leftY = c_int(0)
        rightY = c_int(0)
        templateData = np.empty(templateDataSize,dtype=np.byte)
        isRightAssigned = c_bool(False)
        retCode =  self.libraryLoader.handle.createTemplate(self.libraryLoader.implPtr,multiFace.handle,c_int(2), ctypes.byref(templateDataSize_cint), templateData.ctypes.data_as(c_char_p), ctypes.byref(isLeftAssigned), ctypes.byref(isRightAssigned), ctypes.byref(leftX), ctypes.byref(leftY), ctypes.byref(rightX), ctypes.byref(rightY))
        return (retCode,templateData,isLeftAssigned.value,isRightAssigned.value,leftX.value,rightX.value,leftY.value,rightY.value)
    
    def finalizeEnrolment(self,configDir,enrollmentDir,edbName,edbManifestName, galleryType):
        retCode = self.libraryLoader.handle.finalizeEnrolment(self.libraryLoader.implPtr,configDir.encode(), enrollmentDir.encode(), edbName.encode(), edbManifestName.encode(), c_int(galleryType))
        return retCode


    def initializeIdentification(self,configDir, enrollmentDir):
         retCode = self.libraryLoader.handle.initializeIdentification(self.libraryLoader.implPtr,configDir.encode(), enrollmentDir.encode())
         return retCode
    
    def identifyTemplate(self,template,candidateListLength):
        decision = c_int(0)
        candidateList = FRVTCandidateList(self.libraryLoader,candidateListLength)
        retCode = self.libraryLoader.handle.identifyTemplate(self.libraryLoader.implPtr, template.ctypes.data_as(c_char_p),  template.size, c_int(candidateListLength),candidateList.handle,ctypes.byref(decision))
        return (candidateList,decision.value)

    def insertTemplate(self,template,idString):
        retCode = self.libraryLoader.handle.galleryInsertId(self.libraryLoader.implPtr, template.ctypes.data_as(c_char_p),template.size,idString.encode())
        return retCode
    
    def writeEdbAndManifestFromTemplateList(self,templateList,edbName,manifestName):
        edbHandle = open(edbName,"wb")
        manifestHandle = open(manifestName,"w")
        offset = 0
        for templateElement in templateList:
            templateId = templateElement[0]
            templateData = templateElement[1]
            edbHandle.write(templateData.tobytes())
            manifestHandle.write(f"{templateId}  {templateData.size} {str(offset)}\n")
            offset += templateData.size
        edbHandle.close()
        manifestHandle.close()


if __name__ == '__main__':
    myLibraryLoader = FRVTLibraryLoader()
    myWrapper = FRVTWrapper(myLibraryLoader)
    returnCode = myLibraryLoader.loadLibrary("libfrvt_1N_dermalog_008.so",libDir="/mnt/d/coding/frvtBinaries/lib")
    returnCode = myWrapper.initializeTemplateCreation()
    myImage = FRVTImage(myLibraryLoader,"/mnt/d/coding/frvtPythonWrapper/face.jpg")
    myImage2 = FRVTImage(myLibraryLoader,"/mnt/d/coding/frvtPythonWrapper/face2.jpg")
    trump1 = FRVTImage(myLibraryLoader,"/mnt/d/coding/frvtPythonWrapper/trump1.jpg")
    trump2 = FRVTImage(myLibraryLoader,"/mnt/d/coding/frvtPythonWrapper/trump2.jpg")
    myMultiface = FRVTMultiface(myLibraryLoader,myImage)
    myMultiface2 = FRVTMultiface(myLibraryLoader,myImage2)
    myMultifaceTrump1 = FRVTMultiface(myLibraryLoader,trump1)
    myMultifaceTrump2 = FRVTMultiface(myLibraryLoader,trump2)
    templates = []
    for i in range(10):
        encodingResult = myWrapper.encodeTemplate(myMultiface)
        encodingResult_trump = myWrapper.encodeTemplate(myMultifaceTrump1)
        templates.append((str(i),encodingResult[0]))
        templates.append(("trump_"+str(i),encodingResult_trump[0]))
    encodingResult2 = myWrapper.encodeTemplate(myMultiface2)
    encodingResult_trump2 = myWrapper.encodeTemplate(myMultifaceTrump2)
    templates.append((str(11),encodingResult2[0]))
    edbName = "edb.bin"
    manifestName = "manifest.txt"
    myWrapper.writeEdbAndManifestFromTemplateList(templates,edbName,manifestName)
    retCode = myWrapper.finalizeEnrolment("/mnt/d/coding/frvtBinaries/config","/mnt/d/coding/frvtBinaries/enroll", edbName,manifestName,0)
    retCode = myWrapper.initializeIdentification("/mnt/d/coding/frvtBinaries/config","/mnt/d/coding/frvtBinaries/enroll")
    candidateList,decision = myWrapper.identifyTemplate(encodingResult_trump2[0],5)
    candidateList_converted = candidateList.toList()
    print(candidateList_converted)


    pass
    

