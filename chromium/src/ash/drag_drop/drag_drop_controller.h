// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_DRAG_DROP_DRAG_DROP_CONTROLLER_H_
#define ASH_DRAG_DROP_DRAG_DROP_CONTROLLER_H_

#include "ash/ash_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ui/aura/window_observer.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/events/event_constants.h"
#include "ui/events/event_handler.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/wm/public/drag_drop_client.h"

namespace gfx {
class LinearAnimation;
}

namespace ash {
class DragDropTracker;
class DragDropTrackerDelegate;
class DragImageView;

namespace test {
class DragDropControllerTest;
}

class ASH_EXPORT DragDropController
    : public aura::client::DragDropClient,
      public ui::EventHandler,
      public gfx::AnimationDelegate,
      public aura::WindowObserver {
 public:
  DragDropController();
  ~DragDropController() override;

  void set_should_block_during_drag_drop(bool should_block_during_drag_drop) {
    should_block_during_drag_drop_ = should_block_during_drag_drop;
  }

  // Overridden from aura::client::DragDropClient:
  int StartDragAndDrop(const ui::OSExchangeData& data,
                       aura::Window* root_window,
                       aura::Window* source_window,
                       const gfx::Point& screen_location,
                       int operation,
                       ui::DragDropTypes::DragEventSource source) override;
  void DragUpdate(aura::Window* target, const ui::LocatedEvent& event) override;
  void Drop(aura::Window* target, const ui::LocatedEvent& event) override;
  void DragCancel() override;
  bool IsDragDropInProgress() override;

  // Overridden from ui::EventHandler:
  void OnKeyEvent(ui::KeyEvent* event) override;
  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnTouchEvent(ui::TouchEvent* event) override;
  void OnGestureEvent(ui::GestureEvent* event) override;

  // Overridden from aura::WindowObserver.
  void OnWindowDestroyed(aura::Window* window) override;

 protected:
  // Helper method to create a LinearAnimation object that will run the drag
  // cancel animation. Caller take ownership of the returned object. Protected
  // for testing.
  virtual gfx::LinearAnimation* CreateCancelAnimation(
      int duration,
      int frame_rate,
      gfx::AnimationDelegate* delegate);

  // Actual implementation of |DragCancel()|. protected for testing.
  virtual void DoDragCancel(int drag_cancel_animation_duration_ms);

 private:
  friend class ash::test::DragDropControllerTest;

  // Overridden from gfx::AnimationDelegate:
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

  // Helper method to start drag widget flying back animation.
  void StartCanceledAnimation(int animation_duration_ms);

  // Helper method to forward |pending_log_tap_| event to |drag_source_window_|.
  void ForwardPendingLongTap();

  // Helper method to reset everything.
  void Cleanup();

  scoped_ptr<DragImageView> drag_image_;
  gfx::Vector2d drag_image_offset_;
  const ui::OSExchangeData* drag_data_;
  int drag_operation_;

  // Window that is currently under the drag cursor.
  aura::Window* drag_window_;

  // Starting and final bounds for the drag image for the drag cancel animation.
  gfx::Rect drag_image_initial_bounds_for_cancel_animation_;
  gfx::Rect drag_image_final_bounds_for_cancel_animation_;

  scoped_ptr<gfx::LinearAnimation> cancel_animation_;

  // Window that started the drag.
  aura::Window* drag_source_window_;

  // Indicates whether the caller should be blocked on a drag/drop session.
  // Only be used for tests.
  bool should_block_during_drag_drop_;

  // Closure for quitting nested message loop.
  base::Closure quit_closure_;

  scoped_ptr<ash::DragDropTracker> drag_drop_tracker_;
  scoped_ptr<DragDropTrackerDelegate> drag_drop_window_delegate_;

  ui::DragDropTypes::DragEventSource current_drag_event_source_;

  // Holds a synthetic long tap event to be sent to the |drag_source_window_|.
  // See comment in OnGestureEvent() on why we need this.
  scoped_ptr<ui::GestureEvent> pending_long_tap_;

  base::WeakPtrFactory<DragDropController> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DragDropController);
};

}  // namespace ash

#endif  // ASH_DRAG_DROP_DRAG_DROP_CONTROLLER_H_
