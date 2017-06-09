// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_CMA_TEST_MOCK_FRAME_PROVIDER_H_
#define CHROMECAST_MEDIA_CMA_TEST_MOCK_FRAME_PROVIDER_H_

#include <stddef.h>

#include <vector>

#include "base/macros.h"
#include "chromecast/media/cma/base/coded_frame_provider.h"

namespace chromecast {
namespace media {
class FrameGeneratorForTest;

class MockFrameProvider : public CodedFrameProvider {
 public:
  MockFrameProvider();
  ~MockFrameProvider() override;

  void Configure(
      const std::vector<bool>& delayed_task_pattern,
      scoped_ptr<FrameGeneratorForTest> frame_generator);
  void SetDelayFlush(bool delay_flush);

  // CodedFrameProvider implementation.
  void Read(const ReadCB& read_cb) override;
  void Flush(const base::Closure& flush_cb) override;

 private:
  void DoRead(const ReadCB& read_cb);

  // Parameterization of the frame provider.
  // |delayed_task_pattern_| indicates the pattern for delivering frames,
  // i.e. after receiving a Read request, either delivers a frame right away
  // or wait some time before delivering the frame.
  // |pattern_idx_| is the current index in the pattern.
  // |delay_flush_| indicates whether to delay flush cb in Flush. Default is
  // false.
  std::vector<bool> delayed_task_pattern_;
  size_t pattern_idx_;
  bool delay_flush_;

  scoped_ptr<FrameGeneratorForTest> frame_generator_;

  DISALLOW_COPY_AND_ASSIGN(MockFrameProvider);
};

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_CMA_TEST_MOCK_FRAME_PROVIDER_H_
