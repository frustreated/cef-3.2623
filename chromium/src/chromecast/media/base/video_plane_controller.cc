// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/media/base/video_plane_controller.h"

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "base/bind.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/single_thread_task_runner.h"
#include "base/time/time.h"
#include "chromecast/media/base/media_message_loop.h"
#include "chromecast/public/cast_media_shlib.h"

namespace chromecast {
namespace media {

namespace {

bool RectFEqual(const RectF& r1, const RectF& r2) {
  return r1.x == r2.x && r1.y == r2.y && r1.width == r2.width &&
         r1.height == r2.height;
}

bool SizeEqual(const Size& s1, const Size& s2) {
  return s1.width == s2.width && s1.height == s2.height;
}

bool DisplayRectFValid(const RectF& r) {
  return r.width >= 0 && r.height >= 0;
}

bool ResolutionSizeValid(const Size& s) {
  return s.width >= 0 && s.height >= 0;
}

}  // namespace

// Helper class for calling VideoPlane::SetGeometry with rate-limiting.
// SetGeometry can take on the order of 100ms to run in some implementations
// and can be called on the order of 20x / second (as fast as graphics frames
// are produced).  This creates an ever-growing backlog of tasks on the media
// thread.
// This class measures the time taken to run SetGeometry to determine a
// reasonable frequency at which to call it.  Excess calls are coalesced
// to just set the most recent geometry.
class VideoPlaneController::RateLimitedSetVideoPlaneGeometry
    : public base::RefCountedThreadSafe<RateLimitedSetVideoPlaneGeometry> {
 public:
  RateLimitedSetVideoPlaneGeometry(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner)
      : pending_display_rect_(0, 0, 0, 0),
        pending_set_geometry_(false),
        min_calling_interval_ms_(0),
        sample_counter_(0),
        task_runner_(task_runner) {}

  void SetGeometry(const chromecast::RectF& display_rect,
                   VideoPlane::Transform transform) {
    DCHECK(task_runner_->BelongsToCurrentThread());
    DCHECK(DisplayRectFValid(display_rect));

    base::TimeTicks now = base::TimeTicks::Now();
    base::TimeDelta elapsed = now - last_set_geometry_time_;

    if (elapsed < base::TimeDelta::FromMilliseconds(min_calling_interval_ms_)) {
      if (!pending_set_geometry_) {
        pending_set_geometry_ = true;

        task_runner_->PostDelayedTask(
            FROM_HERE,
            base::Bind(
                &RateLimitedSetVideoPlaneGeometry::ApplyPendingSetGeometry,
                this),
            base::TimeDelta::FromMilliseconds(2 * min_calling_interval_ms_));
      }

      pending_display_rect_ = display_rect;
      pending_transform_ = transform;
      return;
    }
    last_set_geometry_time_ = now;

    LOG(INFO) << __FUNCTION__ << " rect=" << display_rect.width << "x"
              << display_rect.height << " @" << display_rect.x << ","
              << display_rect.y << " transform " << transform;

    VideoPlane* video_plane = CastMediaShlib::GetVideoPlane();
    CHECK(video_plane);
    base::TimeTicks start = base::TimeTicks::Now();
    video_plane->SetGeometry(display_rect, transform);

    base::TimeDelta set_geometry_time = base::TimeTicks::Now() - start;
    UpdateAverageTime(set_geometry_time.InMilliseconds());
  }

 private:
  friend class base::RefCountedThreadSafe<RateLimitedSetVideoPlaneGeometry>;
  ~RateLimitedSetVideoPlaneGeometry() {}

  void UpdateAverageTime(int64_t sample) {
    const size_t kSampleCount = 5;
    if (samples_.size() < kSampleCount)
      samples_.push_back(sample);
    else
      samples_[sample_counter_++ % kSampleCount] = sample;
    int64_t total = 0;
    for (int64_t s : samples_)
      total += s;
    min_calling_interval_ms_ = 2 * total / samples_.size();
  }

  void ApplyPendingSetGeometry() {
    if (pending_set_geometry_) {
      pending_set_geometry_ = false;
      SetGeometry(pending_display_rect_, pending_transform_);
    }
  }

  RectF pending_display_rect_;
  VideoPlane::Transform pending_transform_;
  bool pending_set_geometry_;
  base::TimeTicks last_set_geometry_time_;

  // Don't call SetGeometry faster than this interval.
  int64_t min_calling_interval_ms_;

  // Min calling interval is computed as double average of last few time samples
  // (i.e. allow at least as much time between calls as the call itself takes).
  std::vector<int64_t> samples_;
  size_t sample_counter_;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  DISALLOW_COPY_AND_ASSIGN(RateLimitedSetVideoPlaneGeometry);
};

// static
VideoPlaneController* VideoPlaneController::GetInstance() {
  return base::Singleton<VideoPlaneController>::get();
}

// TODO(esum): SetGeometry, SetDeviceResolution, and SetGraphicsPlaneResolution
// follow the same pattern (copy/paste). Currently it's not worth modularizing
// since there are only 3 fields. If more fields are needed in the future,
// consider making a generic method to implement this pattern.
void VideoPlaneController::SetGeometry(const RectF& display_rect,
                                       VideoPlane::Transform transform) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(DisplayRectFValid(display_rect));
  if (have_video_plane_geometry_ &&
      RectFEqual(display_rect, video_plane_display_rect_) &&
      transform == video_plane_transform_) {
    VLOG(2) << "No change found in geometry parameters.";
    return;
  }

  VLOG(1) << "New geometry parameters "
          << " rect=" << display_rect.width << "x" << display_rect.height
          << " @" << display_rect.x << "," << display_rect.y << " transform "
          << transform;

  have_video_plane_geometry_ = true;
  video_plane_display_rect_ = display_rect;
  video_plane_transform_ = transform;

  MaybeRunSetGeometry();
}

void VideoPlaneController::SetDeviceResolution(const Size& resolution) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(ResolutionSizeValid(resolution));
  if (have_output_res_ && SizeEqual(resolution, output_res_)) {
    VLOG(2) << "No change found in output resolution.";
    return;
  }

  VLOG(1) << "New output resolution " << resolution.width << "x"
          << resolution.height;

  have_output_res_ = true;
  output_res_ = resolution;

  MaybeRunSetGeometry();
}

void VideoPlaneController::SetGraphicsPlaneResolution(const Size& resolution) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(ResolutionSizeValid(resolution));
  if (have_graphics_res_ && SizeEqual(resolution, graphics_res_)) {
    VLOG(2) << "No change found in graphics resolution.";
    return;
  }

  VLOG(1) << "New graphics resolution " << resolution.width << "x"
          << resolution.height;

  have_graphics_res_ = true;
  graphics_res_ = resolution;

  MaybeRunSetGeometry();
}

VideoPlaneController::VideoPlaneController()
    : have_output_res_(false),
      have_graphics_res_(false),
      output_res_(0, 0),
      graphics_res_(0, 0),
      have_video_plane_geometry_(false),
      video_plane_display_rect_(0, 0),
      video_plane_transform_(VideoPlane::TRANSFORM_NONE),
      media_task_runner_(MediaMessageLoop::GetTaskRunner()),
      video_plane_wrapper_(
          new RateLimitedSetVideoPlaneGeometry(media_task_runner_)) {}

VideoPlaneController::~VideoPlaneController() {}

void VideoPlaneController::MaybeRunSetGeometry() {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!HaveDataForSetGeometry())
    return;

  DCHECK(graphics_res_.width != 0 && graphics_res_.height != 0);

  RectF scaled_rect = video_plane_display_rect_;
  if (graphics_res_.width != output_res_.width ||
      graphics_res_.height != output_res_.height) {
    float sx = static_cast<float>(output_res_.width) / graphics_res_.width;
    float sy = static_cast<float>(output_res_.height) / graphics_res_.height;
    scaled_rect.x *= sx;
    scaled_rect.y *= sy;
    scaled_rect.width *= sx;
    scaled_rect.height *= sy;
  }

  media_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&RateLimitedSetVideoPlaneGeometry::SetGeometry,
                 video_plane_wrapper_, scaled_rect, video_plane_transform_));
}

bool VideoPlaneController::HaveDataForSetGeometry() const {
  DCHECK(thread_checker_.CalledOnValidThread());
  return have_output_res_ && have_graphics_res_ && have_video_plane_geometry_;
}

}  // namespace media
}  // namespace chromecast
