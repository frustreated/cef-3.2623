// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIGNIN_SIGNIN_PROMO_H_
#define CHROME_BROWSER_SIGNIN_SIGNIN_PROMO_H_

#include <string>

#include "chrome/browser/ui/profile_chooser_constants.h"
#include "components/signin/core/browser/signin_metrics.h"

class GURL;
class Profile;

namespace user_prefs {
class PrefRegistrySyncable;
}

// Utility functions for sign in promos.
namespace signin {

const char kSignInPromoQueryKeyAccessPoint[] = "access_point";
const char kSignInPromoQueryKeyAutoClose[] = "auto_close";
const char kSignInPromoQueryKeyContinue[] = "continue";
const char kSignInPromoQueryKeyReason[] = "reason";
const char kSignInPromoQueryKeySource[] = "source";
const char kSignInPromoQueryKeyConstrained[] = "constrained";
const char kSignInPromoQueryKeyShowAccountManagement[] =
    "showAccountManagement";

// Returns true if the sign in promo should be visible.
// |profile| is the profile of the tab the promo would be shown on.
bool ShouldShowPromo(Profile* profile);

// Returns true if we should show the sign in promo at startup.
bool ShouldShowPromoAtStartup(Profile* profile, bool is_new_profile);

// Called when the sign in promo has been shown so that we can keep track
// of the number of times we've displayed it.
void DidShowPromoAtStartup(Profile* profile);

// Registers the fact that the user has skipped the sign in promo.
void SetUserSkippedPromo(Profile* profile);

// Gets the sign in landing page URL.
GURL GetLandingURL(signin_metrics::AccessPoint access_point);

// Returns the sign in promo URL wth the given arguments in the query.
// |access_point| indicates where the sign in is being initiated.
// |reason| indicates the purpose of using this URL.
// |auto_close| whether to close the sign in promo automatically when done.
// |is_constrained} whether to load the URL in a constrained window, false
// by default.
GURL GetPromoURL(signin_metrics::AccessPoint access_point,
                 signin_metrics::Reason reason,
                 bool auto_close);
GURL GetPromoURL(signin_metrics::AccessPoint access_point,
                 signin_metrics::Reason reason,
                 bool auto_close,
                 bool is_constrained);

// Returns a sign in promo URL specifically for reauthenticating |account_id|.
GURL GetReauthURL(signin_metrics::AccessPoint access_point,
                  signin_metrics::Reason reason,
                  Profile* profile,
                  const std::string& account_id);

// Returns a sign in promo URL specifically for reauthenticating |email|.
GURL GetReauthURLWithEmail(signin_metrics::AccessPoint access_point,
                           signin_metrics::Reason reason,
                           const std::string& email);

// Gets the next page URL from the query portion of the sign in promo URL.
GURL GetNextPageURLForPromoURL(const GURL& url);

// Gets the partition URL for the embedded sign in frame/webview.
GURL GetSigninPartitionURL();

// Gets the signin URL to be used to display the sign in flow for |mode| in
// |profile|.
GURL GetSigninURLFromBubbleViewMode(Profile* profile,
                                    profiles::BubbleViewMode mode,
                                    signin_metrics::AccessPoint access_point);

// Gets the access point from the query portion of the sign in promo URL.
signin_metrics::AccessPoint GetAccessPointForPromoURL(const GURL& url);

// Gets the sign in reason from the query portion of the sign in promo URL.
signin_metrics::Reason GetSigninReasonForPromoURL(const GURL& url);

// Returns true if the auto_close parameter in the given URL is set to true.
bool IsAutoCloseEnabledInURL(const GURL& url);

// Returns true if the showAccountManagement parameter in the given url is set
// to true.
bool ShouldShowAccountManagement(const GURL& url);

// Forces UseWebBasedSigninFlow() to return true when set; used in tests only.
void ForceWebBasedSigninFlowForTesting(bool force);

// Registers the preferences the Sign In Promo needs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace signin

#endif  // CHROME_BROWSER_SIGNIN_SIGNIN_PROMO_H_