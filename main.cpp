#include <pthread.h>
#include <jni.h>
#include <memory.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <bits/sysconf.h>
#include <sys/mman.h>

#include "Includes/Logger.h"
#include "Patching/Patch.h"
#import "Includes/Utils.h"

#if defined(__armv7__)
#include "X64Hook/And64InlineHook.hpp"
#else
#include "Substrate/CydiaSubstrate.h"
#endif
bool Pet = false;
bool Hat = false;
bool Skin = false;
bool NoAds2;
bool Chat = false;
bool CanMove = false;
float speedModifier = 10;
float speedModifier2 = 10;

struct Patches{
    Patch *Pets;
    Patch *Hats;
    Patch *Skins;
    Patch *Chats;
}patch;


//~~~~~~~~~~~~~~~~~~~~~~normal speed~~~~~~~~~~~~~~~~~~~~~~//
float (*old_Speed)(void *instance);
float Speed(void *instance) {
    if (instance != NULL && speedModifier > 1) {
        return (float)speedModifier;
    }
    return old_Speed(instance);
}
//~~~~~~~~~~~~~~~~~~~~~~ghost speed~~~~~~~~~~~~~~~~~~~~~~//
float (*old_Speed2)(void *instance);
float Speed2(void *instance) {
    if (instance != NULL && speedModifier2 > 1) {
        return (float)speedModifier;
    }
    return old_Speed2(instance);
}
//~~~~~~~~~~~~~~~~~~~~~~no ads~~~~~~~~~~~~~~~~~~~~~~//
bool (*old_NoAds)(void *instance);
bool NoAds(void *instance) {
    if (instance != NULL) {
        if (NoAds) {
            return NoAds2;
        }
    }
    return old_NoAds(instance);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// we will run our patches in a new thread so our while loop doesn't block process main thread
void* hack_thread(void*) {
    LOGI("I have been loaded. Mwuahahahaha");
    // loop until our target library is found
    do {
        sleep(1);
    } while (!isLibraryLoaded(libName));
    LOGI("I found the il2cpp lib. Address is: %p", (void*)findLibrary(libName));
    LOGI("Hooking GameManager_LateUpdate");



    //~~~~~~~~~~~~~~~~~~~~~~hooks~~~~~~~~~~~~~~~~~~~~~~//
    octo_hook((void*)getAbsoluteAddress(0xE3D990), (void*)Speed, (void**)&old_Speed); // get_TrueSpeed
    octo_hook((void*)getAbsoluteAddress(0xE42644), (void*)Speed2, (void**)&old_Speed2); // get_TrueGhostSpeed
    octo_hook((void*)getAbsoluteAddress(0xB997E0), (void*)NoAds, (void**)&old_NoAds); // public static bool GNIGMLHMOMB() { }
    /* octo_hook((void*)getAbsoluteAddress(0xD03E20), (void*)EmergencyCooldown, (void**)&old_EmergencyCooldown); //EmergencyCooldown */
    //~~~~~~~~~~~~~~~~~~~~~~hooks~~~~~~~~~~~~~~~~~~~~~~//



    //~~~~~~~~~~~~~~~~~~~~~~patches~~~~~~~~~~~~~~~~~~~~~~//
    patch.Pets = Patch::Setup((void*)getAbsoluteAddress(0xBEECAC), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8); //HatManager*<GetUnlockedPets>b__8_0
    patch.Hats = Patch::Setup((void*)getAbsoluteAddress(0xBEED48), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8); //HatManager*<GetUnlockedHats>b__10_0
    patch.Skins = Patch::Setup((void*)getAbsoluteAddress(0xBEEE3C), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);//HatManager*<GetUnlockedSkins>b__13_0
    patch.Chats = Patch::Setup((void*)getAbsoluteAddress(0x131B970), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);//ChatController*SetVisible
    //patch.Move = Patch::Setup((void*)getAbsoluteAddress(0xAD50B0), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);//PlayerControl*get_CanMove (gay shet nu work)
    //~~~~~~~~~~~~~~~~~~~~~~patches~~~~~~~~~~~~~~~~~~~~~~//
    return NULL;
}


extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_dark_force_NativeLibrary_getListFT(JNIEnv *env, jclass jobj){
    jobjectArray ret;
    int i;
    const char *features[]= {"Unlock Pets", "Unlock Hats", "Unlock Skins", "No Ads", "Chat Visible", "SeekBar_Speed Modifier_0_250", "SeekBar_Ghost Speed Modifier_0_250"};  //toggles
    int Total_Feature = (sizeof features / sizeof features[0]); //Now you dont have to manually update the number everytime
    ret= (jobjectArray)env->NewObjectArray(Total_Feature,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF(""));

    for(i=0;i<Total_Feature;i++) {
        env->SetObjectArrayElement(
                ret,i,env->NewStringUTF(features[i]));
    }
    return(ret);
}
//compile and try oke
//~~~~~~~~~~~~~~~~~~~~~~cases~~~~~~~~~~~~~~~~~~~~~~//
extern "C"
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_changeToggle(JNIEnv *env, jclass thisObj, jint number) {
    int i = (int) number;
    switch (i) {
        case 0:
            Pet = !Pet;
            if (Pet) {
                patch.Pets->Apply();
            } else {
                patch.Pets->Reset();
            }
            break;
        case 1:
            Hat = !Hat;
            if (Hat) {
                patch.Hats->Apply();
            } else {
                patch.Hats->Reset();
            }
            break;
        case 2:
            Skin = !Skin;
            if (Skin) {
                patch.Skins->Apply();
            } else {
                patch.Skins->Reset();
            }
            break;
        case 3:
            NoAds2 = !NoAds2;
            if(NoAds2){
                true;
            } else {
                false;
            }
            break;
        case 4:
            Chat = !Chat;
            if (Chat) {
                patch.Chats->Apply();
            } else {
                patch.Chats->Reset();
            }
            break;
    }
    return;
}
extern "C"
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_init(JNIEnv * env, jclass obj, jobject thiz){
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);

    //Add our toast in here so it wont be easy to change by simply editing the smali and cant
    //be cut out because this method is needed to start the hack (I'm smart)
    jstring jstr = env->NewStringUTF("Mod Menu by FireBlue"); //Edit this text to your desired toast message!
    jclass toast = env->FindClass("android/widget/Toast");
    jmethodID methodMakeText =
            env->GetStaticMethodID(
                    toast,
                    "makeText",
                    "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    if (methodMakeText == NULL) {
        LOGE("toast.makeText not Found");
        return;
    }
    //The last int is the length on how long the toast should be displayed
    //0 = Short, 1 = Long
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText,
                                                   thiz, jstr, 1);

    jmethodID methodShow = env->GetMethodID(toast, "show", "()V");
    if (methodShow == NULL) {
        LOGE("toast.show not Found");
        return;
    }
    env->CallVoidMethod(toastobj, methodShow);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_dark_force_NativeLibrary_changeSeekBar(JNIEnv *env, jclass clazz, jint i, jint seekbarValue) {
    int li = (int) i;
    switch (li) {
        case 5:
            speedModifier = seekbarValue;
            break;
        case 6:
            speedModifier2 = seekbarValue;
            break;
        default:
            break;
    }
    return;
}