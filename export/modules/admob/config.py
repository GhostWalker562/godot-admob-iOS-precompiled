# Godot 2.1 only passes platform. Godot 3+ build passes env, platform
def can_build(*argv):
	platform = argv[1] if len(argv) == 2 else argv[0]
	return platform=="android" or platform=="iphone"

def configure(env):
	if (env['platform'] == 'android'):
		env.android_add_dependency("compile ('com.google.android.gms:play-services-ads:16.0.0') { exclude group: 'com.android.support' }")
		env.android_add_java_dir("android")
		env.android_add_to_manifest("android/AndroidManifestChunk.xml")
		env.disable_module()
            
	if env['platform'] == "iphone":
		xcframework_arch_directory = ''
		if env['arch'] == 'x86_64':
			xcframework_arch_directory = 'ios-x86_64-simulator'
		else:
			xcframework_arch_directory = 'ios-armv7_arm64'

		env.Append(FRAMEWORKPATH=[
			'#modules/admob/ios/lib',
			'#modules/admob/ios/lib/GoogleUtilities.xcframework/' + xcframework_arch_directory, 
			'#modules/admob/ios/lib/nanopb.xcframework/' + xcframework_arch_directory, 
			'#modules/admob/ios/lib/PromisesObjC.xcframework/' + xcframework_arch_directory])

		env.Append(CPPPATH=['#core'])
		env.Append(LINKFLAGS=[
			'-ObjC', 
			'-framework', 'AdSupport', 
			'-framework', 'CoreTelephony', 
			'-framework', 'EventKit', 
			'-framework', 'EventKitUI', 
			'-framework', 'MessageUI', 
			'-framework', 'StoreKit', 
			'-framework', 'SafariServices', 
			'-framework', 'CoreBluetooth', 
			'-framework', 'AssetsLibrary', 
			'-framework', 'CoreData', 
			'-framework', 'CoreLocation', 
			'-framework', 'CoreText', 
			'-framework', 'ImageIO', 
			'-framework', 'GLKit', 
			'-framework', 'CoreVideo', 
			'-framework', 'CFNetwork', 
			'-framework', 'MobileCoreServices', 
			'-framework', 'nanopb', 
			'-framework', 'PromisesObjC', 
			'-framework', 'GoogleUtilities', 
			'-framework', 'GoogleMobileAds', 
			'-framework', 'GoogleAppMeasurement'])
