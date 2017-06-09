// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/testing_pref_service.h"
#include "base/test/histogram_tester.h"
#include "components/signin/core/browser/signin_investigator.h"
#include "components/signin/core/browser/signin_metrics.h"
#include "components/signin/core/common/signin_pref_names.h"
#include "testing/gtest/include/gtest/gtest.h"

using signin_metrics::AccountEquality;

namespace {
const char kSameEmail[] = "user1@domain.com";
const char kDifferentEmail[] = "user2@domain.com";
const char kSameId[] = "1";
const char kDifferentId[] = "2";
const char kEmptyId[] = "";

class FakeProvider : public SigninInvestigator::DependencyProvider {
 public:
  FakeProvider(const std::string& last_email, const std::string& last_id) {
    prefs_.registry()->RegisterStringPref(prefs::kGoogleServicesLastUsername,
                                          "");
    prefs_.registry()->RegisterStringPref(prefs::kGoogleServicesLastAccountId,
                                          "");
    prefs_.SetString(prefs::kGoogleServicesLastUsername, last_email);
    prefs_.SetString(prefs::kGoogleServicesLastAccountId, last_id);
  }
  PrefService* GetPrefs() override { return &prefs_; }
  TestingPrefServiceSimple prefs_;
};
}  // namespace

class SigninInvestigatorTest : public testing::Test {
 protected:
  void AssertAccountEquality(const std::string& email,
                             const std::string& id,
                             bool equals_expectated,
                             AccountEquality histogram_expected) {
    base::HistogramTester histogram_tester;
    FakeProvider provider(email, id);
    SigninInvestigator investigator(kSameEmail, kSameId, &provider);
    bool equals_actual = investigator.AreAccountsEqualWithFallback();
    ASSERT_EQ(equals_expectated, equals_actual);
    histogram_tester.ExpectUniqueSample(
        "Signin.AccountEquality", static_cast<int>(histogram_expected), 1);
  }

  void AssertInvestigatedScenario(const std::string& email,
                                  const std::string& id,
                                  InvestigatedScenario expected) {
    base::HistogramTester histogram_tester;
    FakeProvider provider(email, id);
    SigninInvestigator investigator(kSameEmail, kSameId, &provider);
    InvestigatedScenario actual = investigator.Investigate();
    ASSERT_EQ(expected, actual);
    histogram_tester.ExpectUniqueSample("Signin.InvestigatedScenario",
                                        static_cast<int>(expected), 1);
  }
};

TEST_F(SigninInvestigatorTest, EqualitySameEmailsSameIds) {
  AssertAccountEquality(kSameEmail, kSameId, true, AccountEquality::BOTH_EQUAL);
}

TEST_F(SigninInvestigatorTest, EqualitySameEmailsDifferentIds) {
  AssertAccountEquality(kSameEmail, kDifferentId, false,
                        AccountEquality::ONLY_SAME_EMAIL);
}

TEST_F(SigninInvestigatorTest, EqualityDifferentEmailsSameIds) {
  AssertAccountEquality(kDifferentEmail, kSameId, true,
                        AccountEquality::ONLY_SAME_ID);
}

TEST_F(SigninInvestigatorTest, EqualityDifferentEmailsDifferentIds) {
  AssertAccountEquality(kDifferentEmail, kDifferentId, false,
                        AccountEquality::BOTH_DIFFERENT);
}

TEST_F(SigninInvestigatorTest, EqualitySameEmailFallback) {
  AssertAccountEquality(kSameEmail, kEmptyId, true,
                        AccountEquality::EMAIL_FALLBACK);
}

TEST_F(SigninInvestigatorTest, EqualityDifferentEmailFallback) {
  AssertAccountEquality(kDifferentEmail, kEmptyId, false,
                        AccountEquality::EMAIL_FALLBACK);
}

TEST_F(SigninInvestigatorTest, InvestigateSameAccount) {
  AssertInvestigatedScenario(kSameEmail, kSameId,
                             InvestigatedScenario::SAME_ACCOUNT);
}

TEST_F(SigninInvestigatorTest, InvestigateUpgrade) {
  AssertInvestigatedScenario("", "", InvestigatedScenario::UPGRADE_LOW_RISK);
}

TEST_F(SigninInvestigatorTest, InvestigateDifferentAccount) {
  AssertInvestigatedScenario(kDifferentEmail, kDifferentId,
                             InvestigatedScenario::DIFFERENT_ACCOUNT);
}
