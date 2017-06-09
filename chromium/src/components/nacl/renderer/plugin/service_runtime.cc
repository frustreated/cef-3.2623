/*
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "components/nacl/renderer/plugin/service_runtime.h"

#include <string.h>
#include <string>
#include <utility>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "components/nacl/renderer/plugin/plugin.h"
#include "components/nacl/renderer/plugin/utility.h"
#include "native_client/src/trusted/service_runtime/nacl_error_code.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/core.h"

namespace plugin {

ServiceRuntime::ServiceRuntime(Plugin* plugin,
                               PP_Instance pp_instance,
                               bool main_service_runtime,
                               bool uses_nonsfi_mode)
    : plugin_(plugin),
      pp_instance_(pp_instance),
      main_service_runtime_(main_service_runtime),
      uses_nonsfi_mode_(uses_nonsfi_mode),
      process_id_(base::kNullProcessId) {
}

void ServiceRuntime::StartSelLdr(const SelLdrStartParams& params,
                                 pp::CompletionCallback callback) {
  GetNaClInterface()->LaunchSelLdr(
      pp_instance_,
      PP_FromBool(main_service_runtime_),
      params.url.c_str(),
      &params.file_info,
      PP_FromBool(uses_nonsfi_mode_),
      params.process_type,
      &translator_channel_,
      &process_id_,
      callback.pp_completion_callback());
}

// TODO(mseaborn): Remove this method, since it is a no-op.
void ServiceRuntime::Shutdown() {
}

ServiceRuntime::~ServiceRuntime() {
}

}  // namespace plugin
