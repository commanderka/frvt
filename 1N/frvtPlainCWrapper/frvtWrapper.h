#pragma once
#ifdef __cplusplus      
#pragma message "Using CPLUSPLUS"                                                                
extern "C" {
#endif   

    int closeLibrary(void);
    int loadLibrary(char* szLibName, char* szSymbolName, void** pImplPtr);
    int initializeTemplateCreation(void* pImplPtr,char* configDir, int templateRole);
    int createTemplate(void* pImplPtr,void* hMultifaceHandle, int nRole, int* nTemplateDataSize, unsigned char* templateData, int* bIsLeftAssigned, int* bIsRightAssigned, int* leftX, int* leftY, int* rightX, int* rightY );
    int finalizeEnrolment(void* pImplPtr, char* configDir, char* enrollmentDir, char* edbName, char* edbManifestName, int galleryType);
    int initializeIdentification(void* pImplPtr, char* configDir, char* enrollmentDir);
    int identifyTemplate(void* pImplPtr, char* templateData, int nTemplateDataSize, int candidateListLength, void* candidateListHandle, int* nDecision);
    int galleryInsertId(void* pImplPtr, char* templateData, int nTemplateDataSize, char* id);

    int createMultifaceHandle(void** hMultifaceHandle);
    int createImageHandle(void** hImageHandle);
    int loadImage(void* hImageHandle, int width, int height, int depth, unsigned char* data, int description);
    int addImageToMultiface(void* hImageHandle, void* hMultifaceHandle);
    int destroyImageHandle(void** pHandle);
    int destroyCandidateListHandle(void** pHandle);
    int destroyMultifaceHandle(void** pHandle);
    int createCandidateListHandle(void** hCandidateListHandle);
    int getCandidate(void * hCandidateListHandle, int nIndex, int* isAssigned,char* templateId, double* similarityScore);

#ifdef __cplusplus 
}
#endif