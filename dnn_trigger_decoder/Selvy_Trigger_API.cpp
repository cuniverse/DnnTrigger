#include <stdio.h>
#include <string.h>
#include "Selvy_Trigger_API.h"
#include "dnn_trigger.h"
#include <stdexcept>
#include <string>
#define USE_THROW 1
//#define TIMELOCK 1
//
//static ITriggerAPI* ITriggerAPI::getInst(const char *class_name,const char root_path[], const char config_path[],int *errorCode)
//{
//    ITriggerAPI* _new_inst = NULL;
//    //*errorCode
//    //CLASS_NAME_DNN_Trigger
//    if(errorCode) *errorCode = ERROR_WRONG_CLASS_NAME;
//    if (!strcmp(CLASS_NAME_DNN_Trigger, class_name))
//    {
//        _new_inst = new CDnnTrigger(root_path, config_path);
//    }
//    else
//    {
//        return NULL;
//    }
//    if(_new_inst == NULL)
//    {
//        if(errorCode) *errorCode = ERROR_CREATE_CLASS_FAIL;
//        return NULL;
//    }
//    if(errorCode) *errorCode = NON_ERROR;
//
//
//    return _new_inst;
//}

Selvy_DNN_Trigger::Selvy_DNN_Trigger(const char *root_path, const char *config_path,const char *license_string)
{
    __impl__ = NULL;
#ifdef TIMELOCK
    err = 0;
    struct tm * newtime;
		time_t ltime;
		time(&ltime);
		newtime = localtime(&ltime);
//		if(newtime->tm_year + 1900 == 2018){
//			if( newtime->tm_mon >= 5) {
//                err = -4;
//            }
//		} else
        if(newtime->tm_year + 1900 >= 2019){
            err = -4;
		}// else return -4;
        if(err == -4){
            printf("Selvy_DNN_Trigger() Time Expired!!\n");
            //throw -4;
            return ;
        }
#endif

    if(root_path != NULL) printf("root_path: %s\n",root_path);
    if(config_path != NULL) printf("config_path: %s\n",config_path);
    if(license_string != NULL) printf("license_string: %s\n",license_string);

    //if (!strcmp(CLASS_NAME_DNN_Trigger, class_name))
    {
        CDnnTrigger *_new_inst_ = new CDnnTrigger(root_path, config_path);
        err = _new_inst_->getError();
#ifdef USE_THROW
        if(err){
            throw err;
            return;
        }
#endif
        __impl__ = _new_inst_;
    }
//    else
//    {
//        throw std::invalid_argument("Wrong Class Name!");
//        return;
//    }

}

Selvy_DNN_Trigger::~Selvy_DNN_Trigger() {
    if(__impl__!=NULL) delete __impl__;
}

int Selvy_DNN_Trigger::detect(const int len_sample, const int16_t *pcm_buf,int *p_st_frame_info) {
    if(__impl__==NULL) throw std::runtime_error("Create Class Error: " + err);
    return __impl__->detect(len_sample,pcm_buf,p_st_frame_info);
}

bool Selvy_DNN_Trigger::reset() {
    if(__impl__==NULL) throw std::runtime_error("Create Class Error: " + err);
    return __impl__->reset();
}

//int Selvy_DNN_Trigger::getOutSPFrame() {
//    if(__impl__==NULL) throw std::runtime_error("Create Class Error: " + err);
//    return __impl__->getOutSPFrame();
//}
