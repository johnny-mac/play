#import <Cocoa/Cocoa.h>
#import "OutputWindowController.h"

@interface ApplicationDelegate : NSObject <OutputWindowDelegate>
{
	OutputWindowController* outputWindowController;
	IBOutlet NSMenuItem* pauseResumeMenuItem;
	IBOutlet NSMenuItem* loadStateMenuItem;
	IBOutlet NSMenuItem* saveStateMenuItem;
	IBOutlet NSMenuItem* screenCaptureMenuItem;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification;
- (IBAction)showPreferences:(id)sender;
- (IBAction)screenCaptureMenuSelected:(id)sender;
- (IBAction)bootElfMenuSelected:(id)sender;
- (IBAction)bootCdrom0MenuSelected:(id)sender;
- (IBAction)bootDiskImageMenuSelected:(id)sender;
- (IBAction)fitToScreenMenuSelected:(id)sender;
- (IBAction)fillScreenMenuSelected:(id)sender;
- (IBAction)actualSizeMenuSelected:(id)sender;
- (IBAction)fullScreenMenuSelected:(id)sender;
- (IBAction)pauseResumeMenuSelected:(id)sender;
- (IBAction)saveStateMenuSelected:(id)sender;
- (IBAction)loadStateMenuSelected:(id)sender;
- (void)bootFromElf:(NSString*)fileName;
- (void)bootFromCdrom0;
- (void)updateTitle;

@end
