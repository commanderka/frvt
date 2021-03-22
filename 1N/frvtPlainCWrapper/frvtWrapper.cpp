#include "frvtWrapper.h"
#include <frvt/1N/src/include/frvt1N.h>
#include <frvt/common/src/include/frvt_structs.h>
#include <cstring>
#include <dlfcn.h>

using  namespace FRVT_1N;

 static std::shared_ptr<FRVT_1N::Interface> oInterface  = NULL;

int loadLibrary(char* szLibName, char* szSymbolName, void** pImplPtr){
    void* handle = dlopen(szLibName, RTLD_LAZY);
    void* pSym = dlsym(handle, szSymbolName);
    using Func =  std::shared_ptr<FRVT_1N::Interface> (*)(void);
    Func func = (Func) pSym;
    oInterface  = func();

    *pImplPtr = oInterface.get();
    return 0;
}


int initializeTemplateCreation(void* pImplPtr,char* configDir, int templateRole){
    
    FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;
    FRVT::ReturnStatus oStatus = oInterface->initializeTemplateCreation(configDir,(FRVT::TemplateRole) templateRole);
    
    return (int) oStatus.code;
    
}

int closeLibrary(void){
}


int createTemplate(void* pImplPtr,void* hMultifaceHandle, int nRole, int* pTemplateDataSize, unsigned char* templateData, int* bIsLeftAssigned, int* bIsRightAssigned, int* leftX, int* leftY, int* rightX, int* rightY ){
    
    FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;
    FRVT::Multiface* pMultiface =  (FRVT::Multiface*) hMultifaceHandle;
    std::vector<uint8_t> abTemplateData;
    std::vector<FRVT::EyePair> aoEyeCoordinates((*pMultiface).size());
    oInterface->createTemplate(*pMultiface,(FRVT::TemplateRole) nRole,abTemplateData,aoEyeCoordinates);


    if (*pTemplateDataSize != abTemplateData.size()){
        *pTemplateDataSize = abTemplateData.size();
        return -1;
    }
    else{
        std::memcpy(templateData,&abTemplateData[0],abTemplateData.size());
        *bIsLeftAssigned = aoEyeCoordinates[0].isLeftAssigned;
        *bIsRightAssigned = aoEyeCoordinates[0].isRightAssigned;
        *leftX = aoEyeCoordinates[0].xleft;
        *leftY = aoEyeCoordinates[0].yleft;
        *rightX = aoEyeCoordinates[0].xright;
        *rightY = aoEyeCoordinates[0].yright;
        return 0;
    }

   return 0;

}

int finalizeEnrolment(void* pImplPtr,char* configDir, char* enrollmentDir, char* edbName, char* edbManifestName, int galleryType){
    FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;

    FRVT::ReturnStatus oRetStatus = pImpl->finalizeEnrollment(configDir,enrollmentDir,edbName,edbManifestName,(FRVT::GalleryType) galleryType);
    return (int) oRetStatus.code;

}

int initializeIdentification(void* pImplPtr,char* configDir, char* enrollmentDir){
    FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;
    FRVT::ReturnStatus oRetStatus = pImpl->initializeIdentification(configDir,enrollmentDir);
    return (int) oRetStatus.code;

}

int identifyTemplate(void* pImplPtr,char* templateData, int nTemplateDataSize, int candidateListLength, void* candidateListHandle, int* nDecision){
      FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;

      std::vector<FRVT::Candidate>* pCandidateListHandle = (std::vector<FRVT::Candidate>*) candidateListHandle;
      std::vector<uint8_t> anTemplateData(nTemplateDataSize);
      std::memcpy(&anTemplateData[0],templateData,nTemplateDataSize);
      bool bDecision = false;
      FRVT::ReturnStatus oRetStatus = pImpl->identifyTemplate(anTemplateData,candidateListLength,*pCandidateListHandle,bDecision);
      *nDecision = bDecision;
      return (int) oRetStatus.code;
}



int galleryInsertId(void* pImplPtr,char* templateData, int nTemplateDataSize, char* id){
     FRVT_1N::Interface* pImpl =  (FRVT_1N::Interface*) pImplPtr;
 
     std::vector<uint8_t> anTemplateData(nTemplateDataSize);
     std::memcpy(&anTemplateData[0],templateData,nTemplateDataSize);
     FRVT::ReturnStatus oRetStatus = pImpl->galleryInsertID(anTemplateData,id);
     return (int) oRetStatus.code;
}

int createMultifaceHandle(void** hMultifaceHandle){
    FRVT::Multiface* pMultiFace = new FRVT::Multiface();
    *hMultifaceHandle =  pMultiFace;
    return 0;
}

int createImageHandle(void** hImageHandle){
    FRVT::Image* pImage = new FRVT::Image();
    *hImageHandle = pImage;
    return 0;
}

int createCandidateListHandle(void** hCandidateListHandle){
    std::vector<FRVT::Candidate>* aoCandidateList = new  std::vector<FRVT::Candidate>();
    *hCandidateListHandle = aoCandidateList;
    return 0;
}

int getCandidate(void * hCandidateListHandle, int nIndex, int* isAssigned,char* templateId, double* similarityScore){
    std::vector<FRVT::Candidate>* aoCandidateList = (std::vector<FRVT::Candidate>*) hCandidateListHandle;
    if (nIndex >= aoCandidateList->size()){
        return -1;
    }
    else{
        FRVT::Candidate oCurrentCandidate = (*aoCandidateList)[nIndex];
        *isAssigned = oCurrentCandidate.isAssigned;
        strcpy(templateId,oCurrentCandidate.templateId.c_str());
        *similarityScore = oCurrentCandidate.similarityScore;
        return 0;
    }

}

int loadImage(void* hImageHandle, int width, int height, int depth, unsigned char* data, int description){
    FRVT::Image* pImage = (FRVT::Image*) hImageHandle;
    pImage->depth = depth;
    pImage->description = (FRVT::Image::Label) description;
    pImage->height = height;
    pImage->width = width;

    uint8_t *myData = new uint8_t[pImage->size()];

    std::memcpy(myData,data,pImage->size());
    pImage->data.reset(myData, std::default_delete<uint8_t[]>());
   
    return 0;
}

int destroyImageHandle(void** pHandle){
    delete (FRVT::Image*) *pHandle;
    *pHandle = nullptr;
    return 0;
}

int destroyCandidateListHandle(void** pHandle){
    delete (std::vector<FRVT::Candidate>*) *pHandle;
    *pHandle = nullptr;
    return 0;
}

int destroyMultifaceHandle(void** pHandle){
    delete (FRVT::Multiface*) *pHandle;
    *pHandle = nullptr;
    return 0;
}

int addImageToMultiface(void* hImageHandle, void* hMultifaceHandle){
    FRVT::Image* pImage = (FRVT::Image*) hImageHandle;
    FRVT::Multiface* pMultiFace = (FRVT::Multiface*) hMultifaceHandle;
    pMultiFace->push_back(*pImage);
    return 0;
}

