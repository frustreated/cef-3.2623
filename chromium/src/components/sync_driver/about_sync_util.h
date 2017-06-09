// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_ABOUT_SYNC_UTIL_H_
#define COMPONENTS_SYNC_DRIVER_ABOUT_SYNC_UTIL_H_

#include "base/memory/scoped_ptr.h"
#include "components/version_info/version_info.h"

class SigninManagerBase;

namespace base {
class DictionaryValue;
}

namespace sync_driver {

class SyncService;

namespace sync_ui_util {

// These strings are used from logs to pull out specific data from sync; we
// don't want these to ever go out of sync between the logs and sync util.
extern const char kIdentityTitle[];
extern const char kDetailsKey[];

// Resource paths.
// Must match the resource file names.
extern const char kAboutJS[];
extern const char kChromeSyncJS[];
extern const char kDataJS[];
extern const char kEventsJS[];
extern const char kSearchJS[];
extern const char kSyncIndexJS[];
extern const char kSyncLogJS[];
extern const char kSyncNodeBrowserJS[];
extern const char kSyncSearchJS[];
extern const char kTypesJS[];

// Message handlers.
// Must match the constants used in the resource files.
extern const char kDispatchEvent[];
extern const char kGetAllNodes[];
extern const char kGetAllNodesCallback[];
extern const char kRegisterForEvents[];
extern const char kRegisterForPerTypeCounters[];
extern const char kRequestListOfTypes[];
extern const char kRequestUpdatedAboutInfo[];

// Other strings.
// Must match the constants used in the resource files.
extern const char kCommit[];
extern const char kCounters[];
extern const char kCounterType[];
extern const char kModelType[];
extern const char kOnAboutInfoUpdated[];
extern const char kOnCountersUpdated[];
extern const char kOnProtocolEvent[];
extern const char kOnReceivedListOfTypes[];
extern const char kStatus[];
extern const char kTypes[];
extern const char kUpdate[];

// This function returns a DictionaryValue which contains all the information
// required to populate the 'About' tab of about:sync.
// Note that |service| may be NULL.
scoped_ptr<base::DictionaryValue> ConstructAboutInformation(
    sync_driver::SyncService* service,
    SigninManagerBase* signin,
    version_info::Channel channel);

}  // namespace sync_ui_util

}  // namespace sync_driver

#endif  // COMPONENTS_SYNC_DRIVER_ABOUT_SYNC_UTIL_H_
