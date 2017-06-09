// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/test/ui_controls.h"

#import <Cocoa/Cocoa.h>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "ui/events/keycodes/keyboard_code_conversion_mac.h"
#import "ui/events/test/cocoa_test_event_utils.h"

// Implementation details: We use [NSApplication sendEvent:] instead
// of [NSApplication postEvent:atStart:] so that the event gets sent
// immediately.  This lets us run the post-event task right
// immediately as well.  Unfortunately I cannot subclass NSEvent (it's
// probably a class cluster) to allow other easy answers.  For
// example, if I could subclass NSEvent, I could run the Task in it's
// dealloc routine (which necessarily happens after the event is
// dispatched).  Unlike Linux, Mac does not have message loop
// observer/notification.  Unlike windows, I cannot post non-events
// into the event queue.  (I can post other kinds of tasks but can't
// guarantee their order with regards to events).

// But [NSApplication sendEvent:] causes a problem when sending mouse click
// events. Because in order to handle mouse drag, when processing a mouse
// click event, the application may want to retrieve the next event
// synchronously by calling NSApplication's nextEventMatchingMask method.
// In this case, [NSApplication sendEvent:] causes deadlock.
// So we need to use [NSApplication postEvent:atStart:] for mouse click
// events. In order to notify the caller correctly after all events has been
// processed, we setup a task to watch for the event queue time to time and
// notify the caller as soon as there is no event in the queue.
//
// TODO(suzhe):
// 1. Investigate why using [NSApplication postEvent:atStart:] for keyboard
//    events causes BrowserKeyEventsTest.CommandKeyEvents to fail.
//    See http://crbug.com/49270
// 2. On OSX 10.6, [NSEvent addLocalMonitorForEventsMatchingMask:handler:] may
//    be used, so that we don't need to poll the event queue time to time.

using cocoa_test_event_utils::SynthesizeKeyEvent;
using cocoa_test_event_utils::TimeIntervalSinceSystemStartup;

namespace {

// Stores the current mouse location on the screen. So that we can use it
// when firing keyboard and mouse click events.
NSPoint g_mouse_location = { 0, 0 };

bool g_ui_controls_enabled = false;

// Creates the proper sequence of autoreleased key events for a key down + up.
void SynthesizeKeyEventsSequence(NSWindow* window,
                                 ui::KeyboardCode keycode,
                                 bool control,
                                 bool shift,
                                 bool alt,
                                 bool command,
                                 std::vector<NSEvent*>* events) {
  NSEvent* event = nil;
  NSUInteger flags = 0;
  if (control) {
    flags |= NSControlKeyMask;
    event = SynthesizeKeyEvent(window, true, ui::VKEY_CONTROL, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (shift) {
    flags |= NSShiftKeyMask;
    event = SynthesizeKeyEvent(window, true, ui::VKEY_SHIFT, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (alt) {
    flags |= NSAlternateKeyMask;
    event = SynthesizeKeyEvent(window, true, ui::VKEY_MENU, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (command) {
    flags |= NSCommandKeyMask;
    event = SynthesizeKeyEvent(window, true, ui::VKEY_COMMAND, flags);
    DCHECK(event);
    events->push_back(event);
  }

  event = SynthesizeKeyEvent(window, true, keycode, flags);
  DCHECK(event);
  events->push_back(event);
  event = SynthesizeKeyEvent(window, false, keycode, flags);
  DCHECK(event);
  events->push_back(event);

  if (command) {
    flags &= ~NSCommandKeyMask;
    event = SynthesizeKeyEvent(window, false, ui::VKEY_COMMAND, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (alt) {
    flags &= ~NSAlternateKeyMask;
    event = SynthesizeKeyEvent(window, false, ui::VKEY_MENU, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (shift) {
    flags &= ~NSShiftKeyMask;
    event = SynthesizeKeyEvent(window, false, ui::VKEY_SHIFT, flags);
    DCHECK(event);
    events->push_back(event);
  }
  if (control) {
    flags &= ~NSControlKeyMask;
    event = SynthesizeKeyEvent(window, false, ui::VKEY_CONTROL, flags);
    DCHECK(event);
    events->push_back(event);
  }
}

// A helper function to watch for the event queue. The specific task will be
// fired when there is no more event in the queue.
void EventQueueWatcher(const base::Closure& task) {
  NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                      untilDate:nil
                                         inMode:NSDefaultRunLoopMode
                                        dequeue:NO];
  // If there is still event in the queue, then we need to check again.
  if (event) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&EventQueueWatcher, task));
  } else {
    base::MessageLoop::current()->PostTask(FROM_HERE, task);
  }
}

// Returns the NSWindow located at |g_mouse_location|. NULL if there is no
// window there, or if the window located there is not owned by the application.
// On Mac, unless dragging, mouse events are sent to the window under the
// cursor. Note that the OS will ignore transparent windows and windows that
// explicitly ignore mouse events.
NSWindow* WindowAtCurrentMouseLocation() {
  NSInteger window_number = [NSWindow windowNumberAtPoint:g_mouse_location
                              belowWindowWithWindowNumber:0];
  NSWindow* window =
      [[NSApplication sharedApplication] windowWithWindowNumber:window_number];
  if (window)
    return window;

  // It's possible for a window owned by another application to be at that
  // location. Cocoa won't provide an NSWindow* for those. Tests should not care
  // about other applications, and raising windows in a headless application is
  // flaky due to OS restrictions. For tests, hunt through all of this
  // application's windows, top to bottom, looking for a good candidate.
  NSArray* window_list = [[NSApplication sharedApplication] orderedWindows];
  for (window in window_list) {
    // Note this skips the extra checks (e.g. fully-transparent windows), that
    // +[NSWindow windowNumberAtPoint:] performs. Tests that care about that
    // should check separately (the goal here is to minimize flakiness).
    if (NSPointInRect(g_mouse_location, [window frame]))
      return window;
  }

  // Note that -[NSApplication orderedWindows] won't include NSPanels. If a test
  // uses those, it will need to handle that itself.
  return nil;
}

}  // namespace

namespace ui_controls {

void EnableUIControls() {
  g_ui_controls_enabled = true;
}

bool SendKeyPress(gfx::NativeWindow window,
                  ui::KeyboardCode key,
                  bool control,
                  bool shift,
                  bool alt,
                  bool command) {
  CHECK(g_ui_controls_enabled);
  return SendKeyPressNotifyWhenDone(window, key,
                                    control, shift, alt, command,
                                    base::Closure());
}

// Win and Linux implement a SendKeyPress() this as a
// SendKeyPressAndRelease(), so we should as well (despite the name).
bool SendKeyPressNotifyWhenDone(gfx::NativeWindow window,
                                ui::KeyboardCode key,
                                bool control,
                                bool shift,
                                bool alt,
                                bool command,
                                const base::Closure& task) {
  CHECK(g_ui_controls_enabled);
  DCHECK(base::MessageLoopForUI::IsCurrent());

  std::vector<NSEvent*> events;
  SynthesizeKeyEventsSequence(
      window, key, control, shift, alt, command, &events);

  // TODO(suzhe): Using [NSApplication postEvent:atStart:] here causes
  // BrowserKeyEventsTest.CommandKeyEvents to fail. See http://crbug.com/49270
  // But using [NSApplication sendEvent:] should be safe for keyboard events,
  // because until now, no code wants to retrieve the next event when handling
  // a keyboard event.
  for (std::vector<NSEvent*>::iterator iter = events.begin();
       iter != events.end(); ++iter)
    [[NSApplication sharedApplication] sendEvent:*iter];

  if (!task.is_null()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::Bind(&EventQueueWatcher, task));
  }

  return true;
}

bool SendMouseMove(long x, long y) {
  CHECK(g_ui_controls_enabled);
  return SendMouseMoveNotifyWhenDone(x, y, base::Closure());
}

// Input position is in screen coordinates.  However, NSMouseMoved
// events require them window-relative, so we adjust.  We *DO* flip
// the coordinate space, so input events can be the same for all
// platforms.  E.g. (0,0) is upper-left.
bool SendMouseMoveNotifyWhenDone(long x, long y, const base::Closure& task) {
  CHECK(g_ui_controls_enabled);
  CGFloat screenHeight =
    [[[NSScreen screens] firstObject] frame].size.height;
  g_mouse_location = NSMakePoint(x, screenHeight - y);  // flip!

  NSWindow* window = WindowAtCurrentMouseLocation();

  NSPoint pointInWindow = g_mouse_location;
  if (window)
    pointInWindow = [window convertScreenToBase:pointInWindow];
  NSTimeInterval timestamp = TimeIntervalSinceSystemStartup();

  NSEvent* event =
      [NSEvent mouseEventWithType:NSMouseMoved
                         location:pointInWindow
                    modifierFlags:0
                        timestamp:timestamp
                     windowNumber:[window windowNumber]
                          context:nil
                      eventNumber:0
                       clickCount:0
                         pressure:0.0];
  [[NSApplication sharedApplication] postEvent:event atStart:NO];

  if (!task.is_null()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::Bind(&EventQueueWatcher, task));
  }

  return true;
}

bool SendMouseEvents(MouseButton type, int state) {
  CHECK(g_ui_controls_enabled);
  return SendMouseEventsNotifyWhenDone(type, state, base::Closure());
}

bool SendMouseEventsNotifyWhenDone(MouseButton type, int state,
                                   const base::Closure& task) {
  CHECK(g_ui_controls_enabled);
  // On windows it appears state can be (UP|DOWN).  It is unclear if
  // that'll happen here but prepare for it just in case.
  if (state == (UP|DOWN)) {
    return (SendMouseEventsNotifyWhenDone(type, DOWN, base::Closure()) &&
            SendMouseEventsNotifyWhenDone(type, UP, task));
  }
  NSEventType etype = NSLeftMouseDown;
  if (type == LEFT) {
    if (state == UP) {
      etype = NSLeftMouseUp;
    } else {
      etype = NSLeftMouseDown;
    }
  } else if (type == MIDDLE) {
    if (state == UP) {
      etype = NSOtherMouseUp;
    } else {
      etype = NSOtherMouseDown;
    }
  } else if (type == RIGHT) {
    if (state == UP) {
      etype = NSRightMouseUp;
    } else {
      etype = NSRightMouseDown;
    }
  } else {
    return false;
  }
  NSWindow* window = WindowAtCurrentMouseLocation();
  NSPoint pointInWindow = g_mouse_location;
  if (window)
    pointInWindow = [window convertScreenToBase:pointInWindow];

  NSEvent* event =
      [NSEvent mouseEventWithType:etype
                         location:pointInWindow
                    modifierFlags:0
                        timestamp:TimeIntervalSinceSystemStartup()
                     windowNumber:[window windowNumber]
                          context:nil
                      eventNumber:0
                       clickCount:1
                         pressure:(state == DOWN ? 1.0 : 0.0 )];
  [[NSApplication sharedApplication] postEvent:event atStart:NO];

  if (!task.is_null()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::Bind(&EventQueueWatcher, task));
  }

  return true;
}

bool SendMouseClick(MouseButton type) {
  CHECK(g_ui_controls_enabled);
  return SendMouseEventsNotifyWhenDone(type, UP|DOWN, base::Closure());
}

void RunClosureAfterAllPendingUIEvents(const base::Closure& closure) {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&EventQueueWatcher, closure));
}

bool IsFullKeyboardAccessEnabled() {
  return [NSApp isFullKeyboardAccessEnabled];
}

}  // namespace ui_controls
