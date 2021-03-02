//
//  MetalHelper.mm
//  AlephOne
//
//  Created by Dustin Wenz on 2/19/21.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <MGLContext.h>
#import <MGLKView.h>

#include "OGL_Headers.h"

#include "SDL_syswm.h"

MGLContext *context;

void* getLayerFromSDLWindow(SDL_Window *main_screen)
{
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  SDL_GetWindowWMInfo(main_screen, &wmi);

  NSWindow *theWindow = (NSWindow*)(wmi.info.cocoa.window);

    
    //Maybe create a MetalANGLE context here and attach it to the SDL window?

    
    /*auto device = MTLCreateSystemDefaultDevice();

    auto swap_chain = [CAMetalLayer layer];
    swap_chain.device = device;
    swap_chain.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    swap_chain.framebufferOnly = YES;
    swap_chain.frame = theWindow.frame;

    theWindow.contentView.layer = swap_chain;
    theWindow.opaque = YES;
    theWindow.backgroundColor = nil;

    auto command_queue = [device newCommandQueue];

    auto render_pass_descriptor = [MTLRenderPassDescriptor new];

    auto render_pass_color_attachment_descriptor = render_pass_descriptor.colorAttachments[0];
    render_pass_color_attachment_descriptor.texture = nil;
    render_pass_color_attachment_descriptor.loadAction = MTLLoadActionClear;
    render_pass_color_attachment_descriptor.clearColor = MTLClearColorMake(0.2, 0.58, 0.7, 1.0);
    render_pass_color_attachment_descriptor.storeAction = MTLStoreActionStore;
*/

 
    
    
  NSViewController *theViewController = theWindow.contentViewController;
  NSView *theView = theWindow.contentView; //[theViewController view];

  [theView setWantsLayer:YES];
    
    MGLLayer *mglLayer = [[MGLLayer alloc] init];

    mglLayer.opaque = YES;
    // In this application, we want to retain the EAGLDrawable contents after a call to present.
    mglLayer.retainedBacking = YES;
    mglLayer.drawableColorFormat = MGLDrawableColorFormatRGBA8888;

    // Set the layer's scale factor as you wish
    mglLayer.contentsScale = 2.0;// [[NSScreen mainScreen] scale];

    // Initialize OpenGL context
    context = [[MGLContext alloc] initWithAPI:kMGLRenderingAPIOpenGLES2];

    // Set context current without any active layer for now. It is perfectly fine to create
    // textures, buffers without any active layer. But before calling any GL draw commands,
    // you must call [MGLContext setCurrentContext: forLayer:], see renderFunc code below.
    if (!context || ![MGLContext setCurrentContext:context]) {
        return nil;
    }

    [theView setLayer:mglLayer];
    
    // Retrieve renderbuffer size.
    // NOTES:
    // - Unlike CAEAGLLayer, you don't need to manually create default framebuffer and
    //   renderbuffer. MGLLayer already creates them internally.
    // - The size could be changed at any time, for example when user resizes the view or
    //   rotates it on iOS devices. So it's better not to cache it.
    GLuint backingWidth, backingHeight;
    backingWidth = mglLayer.drawableSize.width;
    backingHeight = mglLayer.drawableSize.height;
    

    
  NSLog(@"SDL NSView type: %@", NSStringFromClass([theView class]));
  NSLog(@"SDL NSView Layer type: %@", NSStringFromClass([theView.layer class]));
  NSLog(@"SDL NSView Layer size (h,w): %f, %f", theView.layer.bounds.size.height, theView.layer.bounds.size.width);

    if( [theView isKindOfClass:[MTKView class]] ) {
      NSLog(@"Our view is MTKView");
      CAMetalLayer *mLayer = (CAMetalLayer*)theView.layer;
      NSLog(@"CAMetalLayer drawable size (h,w): %f, %f", [mLayer drawableSize].height, [mLayer drawableSize].width);

    }
    
        //Test it!
    [MGLContext setCurrentContext:context forLayer:mglLayer];

    // Clear the buffer. The following glBindFramebuffer() call is optionally. Only needed if you
    // have custom framebuffers aside from the default one.
    glBindFramebuffer(GL_FRAMEBUFFER, mglLayer.defaultOpenGLFrameBufferID);
    glClearColor(1.0, 0.0, 0.0, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    // Display the buffer
    [context present:mglLayer];
    
    
  if( [theView.layer isKindOfClass:[CAMetalLayer class]] ) {
    NSLog(@"Setting CAMetalLayer size?");
    CAMetalLayer *mLayer = (CAMetalLayer*)theView.layer;
    NSLog(@"CAMetalLayer drawable size (h,w): %f, %f", [mLayer drawableSize].height, [mLayer drawableSize].width);

  }
   
  return theView.layer;
}

void swapWindow(SDL_Window *main_screen){
    
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(main_screen, &wmi);

    NSWindow *theWindow = (NSWindow*)(wmi.info.cocoa.window);
    
    MGLLayer *mglLayer = (MGLLayer*)[theWindow.contentView layer];
    [MGLContext setCurrentContext:context forLayer:mglLayer];

    // Clear the buffer. The following glBindFramebuffer() call is optionally. Only needed if you
    // have custom framebuffers aside from the default one.
    glBindFramebuffer(GL_FRAMEBUFFER, mglLayer.defaultOpenGLFrameBufferID);
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);

    // Display the buffer
    [context present:mglLayer];
}
