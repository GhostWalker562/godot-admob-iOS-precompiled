#include "godotAdmob.h"
#import "app_delegate.h"

#if VERSION_MAJOR == 3
#define CLASS_DB ClassDB
#else
#define CLASS_DB ObjectTypeDB
#endif

GodotAdmob *GodotAdmob::instance = NULL; //<< INIT NULL VAR ON COMPILE TIME!

GodotAdmob::GodotAdmob() {
    initialized = false;
    banner = NULL; 
    interstitial  = NULL;
    rewarded = NULL;
    //
    ERR_FAIL_COND(instance != NULL); //<< SUCCESS!!! FIRST 

    instance = this;
}

GodotAdmob::~GodotAdmob() {
   if (initialized) {
       [banner release]; //free banner
       [interstitial release]; //free
       [rewarded release]; //free
   }

   if (instance == this) {
       instance = NULL;
   }
}

void GodotAdmob::init(bool isReal, int instanceId) {
    if (instance != this) {
        NSLog(@"GodotAdmob Module dublicate singleton");
        return;
    }
    if (initialized) {
        NSLog(@"GodotAdmob Module already initialized");
        return;
    }
    
    initialized = true;
    
    banner = [[AdmobBanner alloc] init];
    [banner initialize :isReal :instanceId];
    
    interstitial = [[AdmobInterstitial alloc] init];
    [interstitial initialize :isReal :instanceId :banner];
    
    rewarded = [[AdmobRewarded alloc] init];
    [rewarded initialize :isReal :instanceId :banner];
}

void GodotAdmob::initWithContentRating(bool isReal, int instanceId, bool child_directed, bool is_personalized, const String &max_ad_content_rate) {
    if (instance != this) {
        NSLog(@"GodotAdmob Module dublicate singleton");
        return;
    }
    if (initialized) {
        NSLog(@"GodotAdmob Module already initialized");
        return;
    }
    
    initialized = true;
    
    banner = [[AdmobBanner alloc] init];
    [banner initialize :isReal :instanceId];
    
    interstitial = [[AdmobInterstitial alloc] init];
    [interstitial initialize :isReal :instanceId :banner];
    
    rewarded = [[AdmobRewarded alloc] init];
    [rewarded initialize :isReal :instanceId :banner];
}

void GodotAdmob::loadBanner(const String &bannerId, bool isOnTop) {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    NSString *idStr = [NSString stringWithCString:bannerId.utf8().get_data() encoding: NSUTF8StringEncoding];
    [banner loadBanner:idStr :isOnTop];

}

void GodotAdmob::showBanner() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    [banner showBanner];
}

void GodotAdmob::hideBanner() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    [banner hideBanner];
}


void GodotAdmob::resize() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    [banner resize];
}

int GodotAdmob::getBannerWidth() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return 0;
    }
    return (uintptr_t)[banner getBannerWidth];
}

int GodotAdmob::getBannerHeight() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return 0;
    }
    return (uintptr_t)[banner getBannerHeight];
}

void GodotAdmob::loadInterstitial(const String &interstitialId) {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    NSString *idStr = [NSString stringWithCString:interstitialId.utf8().get_data() encoding: NSUTF8StringEncoding];
    [interstitial loadInterstitial:idStr];

}

void GodotAdmob::showInterstitial() {
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    [interstitial showInterstitial];
    
}

void GodotAdmob::loadRewardedVideo(const String &rewardedId) {
    //init
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    NSString *idStr = [NSString stringWithCString:rewardedId.utf8().get_data() encoding: NSUTF8StringEncoding];
    [rewarded loadRewardedVideo: idStr];
    
}

void GodotAdmob::showRewardedVideo() {
    //show
    if (!initialized) {
        NSLog(@"GodotAdmob Module not initialized");
        return;
    }
    
    [rewarded showRewardedVideo];
}



void GodotAdmob::_bind_methods() {
    CLASS_DB::bind_method("init",&GodotAdmob::init);
    CLASS_DB::bind_method("initWithContentRating",&GodotAdmob::initWithContentRating);
    CLASS_DB::bind_method("loadBanner",&GodotAdmob::loadBanner);
    CLASS_DB::bind_method("showBanner",&GodotAdmob::showBanner);
    CLASS_DB::bind_method("hideBanner",&GodotAdmob::hideBanner);
    CLASS_DB::bind_method("loadInterstitial",&GodotAdmob::loadInterstitial);
    CLASS_DB::bind_method("showInterstitial",&GodotAdmob::showInterstitial);
    CLASS_DB::bind_method("loadRewardedVideo",&GodotAdmob::loadRewardedVideo);
    CLASS_DB::bind_method("showRewardedVideo",&GodotAdmob::showRewardedVideo);
    CLASS_DB::bind_method("resize",&GodotAdmob::resize);
    CLASS_DB::bind_method("getBannerWidth",&GodotAdmob::getBannerWidth);
    CLASS_DB::bind_method("getBannerHeight",&GodotAdmob::getBannerHeight);
}
