#include <frvt/1N/src/include/frvt1N.h>

int main(int arc, const char* argv[]){
    std::shared_ptr<FRVT_1N::Interface> oImpl = FRVT_1N::Interface::getImplementation();
    FRVT::ReturnStatus oStatus = oImpl->initializeTemplateCreation("test",FRVT::TemplateRole::Enrollment_1N);
    std::cout<<"Return status:"<<oStatus.code;
}