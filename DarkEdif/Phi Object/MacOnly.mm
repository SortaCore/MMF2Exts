#import <Foundation/Foundation.h>

#if defined(__cplusplus)
extern "C" {
	char * Phi_Object_getAppFolder()
	{
		/*
		NSError *error;
		NSFileManager *manager = [NSFileManager defaultManager];
		NSURL *applicationSupport = [manager URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:false error:&error];
		NSString *identifier = [[NSBundle mainBundle] bundleIdentifier];
		NSURL *folder = [applicationSupport URLByAppendingPathComponent:identifier];
		[manager createDirectoryAtURL:folder withIntermediateDirectories:true attributes:nil error:&error];
		NSURL *fileURL = [folder URLByAppendingPathComponent:@"TSPlogfile.txt"];
		*/
		
		// OR:
		NSURL *appFolder = [[[NSBundle mainBundle] bundleURL] URLByDeletingLastPathComponent];
		NSString *urlString = [appFolder absoluteString];
		return strdup([urlString UTF8String]);
	}
} // extern "C"
#endif
