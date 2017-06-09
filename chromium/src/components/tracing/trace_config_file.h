// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TRACING_TRACE_CONFIG_FILE_H_
#define COMPONENTS_TRACING_TRACE_CONFIG_FILE_H_

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/trace_event/trace_config.h"
#include "build/build_config.h"
#include "components/tracing/tracing_export.h"

namespace base {
template <typename Type> struct DefaultSingletonTraits;
}  // namespace base

namespace tracing {

// TraceConfigFile is a singleton that contains the configurations of tracing.
// One can create a trace config file and use it to configure startup and/or
// shutdown tracing.
//
// The trace config file should be JSON formated. One example is:
//   {
//     "trace_config": {
//        "record_mode": "record-until-full",
//        "included_categories": ["cc", "skia"]
//      },
//      "startup_duration": 5,
//      "result_file": "chrometrace.log"
//   }
//
// trace_config:     The configuration of tracing. Please see the details in
//                   base/trace_event/trace_config.h.
//
// startup_duration: The duration for startup tracing in terms of seconds.
//                   Tracing will stop automatically after the duration. If this
//                   value is not specified, the duration is 0 and one needs
//                   to stop tracing by other ways, e.g., by DevTools, or get
//                   the result file after shutting the browser down.
//
// result_file:      The file that contains the trace log. The default result
//                   file path is chrometrace.log. Chrome will dump the trace
//                   log to this file
//                   1) after startup_duration if it is specified;
//                   2) or after browser shutdown if startup duration is 0.
//                   One can also stop tracing and get the result by other ways,
//                   e.g., by DevTools. In that case, the trace log will not be
//                   saved to this file.
//                   Notice: This is not supported on Android. The result file
//                   path will be generated by tracing controller.
//
// The trace config file can be specified by the --trace-config-file flag on
// most platforms except on Android, e.g., --trace-config-file=path/to/file/.
// This flag should not be used with --trace-startup or --trace-shutdown. If
// those two flags are used, --trace-config-file flag will be ignored. If the
// --trace-config-file flag is used without the file path, Chrome will do
// startup tracing with 5 seconds' startup duration.
//
// On Android, Chrome does not read the --trace-config-file flag, because not
// all Chrome based browsers read customized flag, e.g., Android WebView. Chrome
// on Android reads from a fixed file location:
//   /data/local/chrome-trace-config.json
// If this file exists, Chrome will start tracing according to the configuration
// specified in the file, otherwise, Chrome will not start tracing.
class TRACING_EXPORT TraceConfigFile {
 public:
  static TraceConfigFile* GetInstance();

  bool IsEnabled() const;
  base::trace_event::TraceConfig GetTraceConfig() const;
  int GetStartupDuration() const;
#if !defined(OS_ANDROID) || defined(USE_AURA)
  base::FilePath GetResultFile() const;
#endif

 private:
  // This allows constructor and destructor to be private and usable only
  // by the Singleton class.
  friend struct base::DefaultSingletonTraits<TraceConfigFile>;
  TraceConfigFile();
  ~TraceConfigFile();

  bool ParseTraceConfigFileContent(const std::string& content);

  bool is_enabled_;
  base::trace_event::TraceConfig trace_config_;
  int startup_duration_;
  base::FilePath result_file_;

  DISALLOW_COPY_AND_ASSIGN(TraceConfigFile);
};

}  // namespace tracing

#endif  // COMPONENTS_TRACING_TRACE_CONFIG_FILE_H_
