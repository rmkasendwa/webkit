/*
 * Copyright (C) 2018-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>
#import <WebKit/WKFoundation.h>

@class WKWebView;
@class _WKFrameHandle;
@protocol _WKInspectorDelegate;

NS_ASSUME_NONNULL_BEGIN

@protocol _WKInspectorExtensionHost
- (void)close;
@end

WK_CLASS_AVAILABLE(macos(10.14.4), ios(12.2))
@interface _WKInspector : NSObject <_WKInspectorExtensionHost>

- (instancetype)init NS_UNAVAILABLE;

@property (nonatomic, weak) id <_WKInspectorDelegate> delegate WK_API_AVAILABLE(macos(WK_MAC_TBA), ios(WK_IOS_TBA));

@property (nonatomic, readonly) WKWebView *webView;
@property (nonatomic, readonly) BOOL isConnected;
@property (nonatomic, readonly) BOOL isVisible;
@property (nonatomic, readonly) BOOL isFront;
@property (nonatomic, readonly) BOOL isProfilingPage;
@property (nonatomic, readonly) BOOL isElementSelectionActive;

- (void)connect;
- (void)show;
- (void)hide;
- (void)showConsole;
- (void)showResources;
- (void)showMainResourceForFrame:(_WKFrameHandle *)frame;
- (void)attach;
- (void)detach;
- (void)togglePageProfiling;
- (void)toggleElementSelection;
- (void)printErrorToConsole:(NSString *)error;

@end

NS_ASSUME_NONNULL_END
