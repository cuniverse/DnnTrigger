// DnnTrigger JNI wrapper for android
// 2016-08 by mckeum
// 2017-11 수정 cuniverse

#include "com_selvasai_stt_selvywakeup_WakeUpSolid.h"

#include <stdio.h>
#include <android/log.h>
#include <Selvy_Trigger_API.h>

#define APPNAME "NdkWakeUpSolid"

// This wrapper is *NOT* thread-safe. Be careful.
static Selvy_DNN_Trigger* dnn_trigger = NULL;

/*
 * Class:     com_selvasai_stt_selvywakeup_WakeUpSolid
 * Method:    solidInit
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_selvasai_stt_selvywakeup_WakeUpSolid_solidInit
  (JNIEnv *env, jobject, jstring jroot_path)
{
    const char* root_path = env->GetStringUTFChars(jroot_path, JNI_FALSE);

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", root_path);
    try {
        dnn_trigger = new Selvy_DNN_Trigger(root_path, "../conf/diotrg_16k.ini");
    } catch (int errCode){
        //int ret = dnn_trigger->getError();
        __android_log_print(ANDROID_LOG_ERROR, APPNAME, "error loading trigger engine: %d\n", errCode);
        return errCode!=0?errCode:-5;
    }

    env->ReleaseStringUTFChars(jroot_path, root_path);

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", root_path);

    return 0;
}

/*
 * Class:     com_selvasai_stt_selvywakeup_WakeUpSolid
 * Method:    solidDetect
 * Signature: (I[S)I
 */
JNIEXPORT jint JNICALL Java_com_selvasai_stt_selvywakeup_WakeUpSolid_solidDetect
  (JNIEnv *env, jobject, jint nBuf, jshortArray jpcm_in, jintArray jInfoArray)
{
//    jsize nBuf = env->GetArrayLength(jpcm_in);
    jshort* jBuf = env->GetShortArrayElements(jpcm_in, 0);
    jint* jInfoBuff = env->GetIntArrayElements(jInfoArray, 0);

    int kw_detected = 0;
    int _frame_info[2];
    auto ret_dec = dnn_trigger->detect(nBuf, jBuf,_frame_info);
    if (0 < ret_dec)
    {
//        printf("KW detected at %d frame \n", ret_dec);
        kw_detected = ret_dec;
    }
    if(jInfoBuff != NULL){
        jInfoBuff[0] = _frame_info[0];
        jInfoBuff[1] = _frame_info[1];
    }
    env->ReleaseIntArrayElements(jInfoArray, jInfoBuff, JNI_OK);
    env->ReleaseShortArrayElements(jpcm_in, jBuf, JNI_ABORT);

    if (0 < kw_detected)    return kw_detected;
    return -1;
}


/*
 * Class:     com_selvasai_stt_selvywakeup_WakeUpSolid
 * Method:    solidDestroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_selvasai_stt_selvywakeup_WakeUpSolid_solidDestroy
  (JNIEnv *, jobject)
{
    delete dnn_trigger;
    dnn_trigger = NULL;
}


/*
 * Class:     com_selvasai_stt_selvywakeup_WakeUpSolid
 * Method:    solidReset
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_selvasai_stt_selvywakeup_WakeUpSolid_solidReset
  (JNIEnv *, jobject)
{
    dnn_trigger->reset();
}
