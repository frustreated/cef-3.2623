// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/policy/core/common/cloud/component_cloud_policy_updater.h"

#include <stddef.h>
#include <stdint.h>
#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "components/policy/core/common/cloud/component_cloud_policy_store.h"
#include "components/policy/core/common/cloud/external_policy_data_fetcher.h"
#include "policy/proto/chrome_extension_policy.pb.h"
#include "policy/proto/device_management_backend.pb.h"

namespace em = enterprise_management;

namespace policy {

namespace {

// The maximum size of the serialized policy protobuf.
const size_t kPolicyProtoMaxSize = 16 * 1024;

// The maximum size of the downloaded policy data.
const int64_t kPolicyDataMaxSize = 5 * 1024 * 1024;

// Tha maximum number of policy data fetches to run in parallel.
const int64_t kMaxParallelPolicyDataFetches = 2;

}  // namespace

ComponentCloudPolicyUpdater::ComponentCloudPolicyUpdater(
    scoped_refptr<base::SequencedTaskRunner> task_runner,
    scoped_ptr<ExternalPolicyDataFetcher> external_policy_data_fetcher,
    ComponentCloudPolicyStore* store)
    : store_(store),
      external_policy_data_updater_(task_runner,
                                    std::move(external_policy_data_fetcher),
                                    kMaxParallelPolicyDataFetches) {}

ComponentCloudPolicyUpdater::~ComponentCloudPolicyUpdater() {
}

void ComponentCloudPolicyUpdater::UpdateExternalPolicy(
    scoped_ptr<em::PolicyFetchResponse> response) {
  // Keep a serialized copy of |response|, to cache it later.
  // The policy is also rejected if it exceeds the maximum size.
  std::string serialized_response;
  if (!response->SerializeToString(&serialized_response) ||
      serialized_response.size() > kPolicyProtoMaxSize) {
    return;
  }

  // Validate the policy before doing anything else.
  PolicyNamespace ns;
  em::ExternalPolicyData data;
  if (!store_->ValidatePolicy(std::move(response), &ns, &data)) {
    LOG(ERROR) << "Failed to validate component policy fetched from DMServer";
    return;
  }

  // Maybe the data for this hash has already been downloaded and cached.
  const std::string& cached_hash = store_->GetCachedHash(ns);
  if (!cached_hash.empty() && data.secure_hash() == cached_hash)
    return;

  // TODO(joaodasilva): implement the other two auth methods.
  if (data.download_auth_method() != em::ExternalPolicyData::NONE)
    return;

  const std::string key = NamespaceToKey(ns);

  if (data.download_url().empty() || !data.has_secure_hash()) {
    // If there is no policy for this component or the policy has been removed,
    // cancel any existing request to fetch policy for this component.
    external_policy_data_updater_.CancelExternalDataFetch(key);

    // Delete any existing policy for this component.
    store_->Delete(ns);
  } else {
    // Make a request to fetch policy for this component. If another fetch
    // request is already pending for the component, it will be canceled.
    external_policy_data_updater_.FetchExternalData(
        key,
        ExternalPolicyDataUpdater::Request(data.download_url(),
                                           data.secure_hash(),
                                           kPolicyDataMaxSize),
        base::Bind(&ComponentCloudPolicyStore::Store, base::Unretained(store_),
                   ns,
                   serialized_response,
                   data.secure_hash()));
  }
}

void ComponentCloudPolicyUpdater::CancelUpdate(const PolicyNamespace& ns) {
  external_policy_data_updater_.CancelExternalDataFetch(NamespaceToKey(ns));
}

std::string ComponentCloudPolicyUpdater::NamespaceToKey(
    const PolicyNamespace& ns) {
  const std::string domain = base::IntToString(ns.domain);
  const std::string size = base::SizeTToString(domain.size());
  return size + ":" + domain + ":" + ns.component_id;
}

}  // namespace policy
