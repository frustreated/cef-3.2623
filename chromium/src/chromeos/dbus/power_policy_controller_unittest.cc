// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/power_policy_controller.h"

#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "chromeos/dbus/fake_power_manager_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {

class PowerPolicyControllerTest : public testing::Test {
 public:
  PowerPolicyControllerTest()
      : fake_power_client_(new FakePowerManagerClient) {}

  ~PowerPolicyControllerTest() override {}

  void SetUp() override {
    PowerPolicyController::Initialize(fake_power_client_.get());
    ASSERT_TRUE(PowerPolicyController::IsInitialized());
    policy_controller_ = PowerPolicyController::Get();
  }

  void TearDown() override {
    if (PowerPolicyController::IsInitialized())
      PowerPolicyController::Shutdown();
  }

 protected:
  scoped_ptr<FakePowerManagerClient> fake_power_client_;
  PowerPolicyController* policy_controller_;
  base::MessageLoop message_loop_;
};

TEST_F(PowerPolicyControllerTest, Prefs) {
  PowerPolicyController::PrefValues prefs;
  prefs.ac_screen_dim_delay_ms = 600000;
  prefs.ac_screen_off_delay_ms = 660000;
  prefs.ac_idle_delay_ms = 720000;
  prefs.battery_screen_dim_delay_ms = 300000;
  prefs.battery_screen_off_delay_ms = 360000;
  prefs.battery_idle_delay_ms = 420000;
  prefs.ac_idle_action = PowerPolicyController::ACTION_SUSPEND;
  prefs.battery_idle_action = PowerPolicyController::ACTION_STOP_SESSION;
  prefs.lid_closed_action = PowerPolicyController::ACTION_SHUT_DOWN;
  prefs.use_audio_activity = true;
  prefs.use_video_activity = true;
  prefs.ac_brightness_percent = 87.0;
  prefs.battery_brightness_percent = 43.0;
  prefs.enable_auto_screen_lock = false;
  prefs.presentation_screen_dim_delay_factor = 3.0;
  prefs.user_activity_screen_dim_delay_factor = 2.0;
  prefs.wait_for_initial_user_activity = true;
  prefs.force_nonzero_brightness_for_user_activity = false;
  policy_controller_->ApplyPrefs(prefs);

  power_manager::PowerManagementPolicy expected_policy;
  expected_policy.mutable_ac_delays()->set_screen_dim_ms(600000);
  expected_policy.mutable_ac_delays()->set_screen_off_ms(660000);
  expected_policy.mutable_ac_delays()->set_screen_lock_ms(-1);
  expected_policy.mutable_ac_delays()->set_idle_warning_ms(-1);
  expected_policy.mutable_ac_delays()->set_idle_ms(720000);
  expected_policy.mutable_battery_delays()->set_screen_dim_ms(300000);
  expected_policy.mutable_battery_delays()->set_screen_off_ms(360000);
  expected_policy.mutable_battery_delays()->set_screen_lock_ms(-1);
  expected_policy.mutable_battery_delays()->set_idle_warning_ms(-1);
  expected_policy.mutable_battery_delays()->set_idle_ms(420000);
  expected_policy.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_SUSPEND);
  expected_policy.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_STOP_SESSION);
  expected_policy.set_lid_closed_action(
      power_manager::PowerManagementPolicy_Action_SHUT_DOWN);
  expected_policy.set_use_audio_activity(true);
  expected_policy.set_use_video_activity(true);
  expected_policy.set_ac_brightness_percent(87.0);
  expected_policy.set_battery_brightness_percent(43.0);
  expected_policy.set_presentation_screen_dim_delay_factor(3.0);
  expected_policy.set_user_activity_screen_dim_delay_factor(2.0);
  expected_policy.set_wait_for_initial_user_activity(true);
  expected_policy.set_force_nonzero_brightness_for_user_activity(false);
  expected_policy.set_reason(PowerPolicyController::kPrefsReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Change some prefs and check that an updated policy is sent.
  prefs.ac_idle_warning_delay_ms = 700000;
  prefs.battery_idle_warning_delay_ms = 400000;
  prefs.lid_closed_action = PowerPolicyController::ACTION_SUSPEND;
  prefs.ac_brightness_percent = -1.0;
  prefs.force_nonzero_brightness_for_user_activity = true;
  policy_controller_->ApplyPrefs(prefs);
  expected_policy.mutable_ac_delays()->set_idle_warning_ms(700000);
  expected_policy.mutable_battery_delays()->set_idle_warning_ms(400000);
  expected_policy.set_lid_closed_action(
      power_manager::PowerManagementPolicy_Action_SUSPEND);
  expected_policy.clear_ac_brightness_percent();
  expected_policy.set_force_nonzero_brightness_for_user_activity(true);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // The enable-auto-screen-lock pref should force the screen-lock delays to
  // match the screen-off delays plus a constant value.
  prefs.enable_auto_screen_lock = true;
  policy_controller_->ApplyPrefs(prefs);
  expected_policy.mutable_ac_delays()->set_screen_lock_ms(
      660000 + PowerPolicyController::kScreenLockAfterOffDelayMs);
  expected_policy.mutable_battery_delays()->set_screen_lock_ms(
      360000 + PowerPolicyController::kScreenLockAfterOffDelayMs);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // If the screen-lock-delay prefs are set to lower values than the
  // screen-off delays plus the constant, the lock prefs should take
  // precedence.
  prefs.ac_screen_lock_delay_ms = 70000;
  prefs.battery_screen_lock_delay_ms = 60000;
  policy_controller_->ApplyPrefs(prefs);
  expected_policy.mutable_ac_delays()->set_screen_lock_ms(70000);
  expected_policy.mutable_battery_delays()->set_screen_lock_ms(60000);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // If the artificial screen-lock delays would exceed the idle delay, they
  // shouldn't be set -- the power manager would ignore them since the
  // idle action should lock the screen in this case.
  prefs.ac_screen_off_delay_ms = prefs.ac_idle_delay_ms - 1;
  prefs.battery_screen_off_delay_ms = prefs.battery_idle_delay_ms - 1;
  prefs.ac_screen_lock_delay_ms = -1;
  prefs.battery_screen_lock_delay_ms = -1;
  policy_controller_->ApplyPrefs(prefs);
  expected_policy.mutable_ac_delays()->set_screen_off_ms(
      prefs.ac_screen_off_delay_ms);
  expected_policy.mutable_battery_delays()->set_screen_off_ms(
      prefs.battery_screen_off_delay_ms);
  expected_policy.mutable_ac_delays()->set_screen_lock_ms(-1);
  expected_policy.mutable_battery_delays()->set_screen_lock_ms(-1);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Set the "allow screen wake locks" pref to false.  The system should be
  // prevented from suspending due to user inactivity on AC power but the
  // pref-supplied screen-related delays should be left untouched.
  prefs.allow_screen_wake_locks = false;
  policy_controller_->ApplyPrefs(prefs);
  policy_controller_->AddScreenWakeLock(PowerPolicyController::REASON_OTHER,
                                        "Screen");
  expected_policy.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy.set_reason(std::string(PowerPolicyController::kPrefsReason) +
                             ", Screen");
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));
}

TEST_F(PowerPolicyControllerTest, WakeLocks) {
  // If our highest lock type is system, we only worry about
  // the idle action.
  const char kSystemWakeLockReason[] = "system";
  power_manager::PowerManagementPolicy expected_policy_system;
  expected_policy_system.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy_system.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);

  // The dim lock type prevents the screen from turning off or
  // locking, and we won't have an idle action.
  const char kDimWakeLockReason[] = "dim";
  power_manager::PowerManagementPolicy expected_policy_dim;
  expected_policy_dim.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy_dim.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy_dim.mutable_ac_delays()->set_screen_off_ms(0);
  expected_policy_dim.mutable_ac_delays()->set_screen_lock_ms(0);
  expected_policy_dim.mutable_battery_delays()->set_screen_off_ms(0);
  expected_policy_dim.mutable_battery_delays()->set_screen_lock_ms(0);

  // The screen lock keeps the screen bright. We won't turn off,
  // lock the screen or do anything for idle.
  const char kScreenWakeLockReason[] = "screen";
  power_manager::PowerManagementPolicy expected_policy_screen;
  expected_policy_screen.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy_screen.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy_screen.mutable_ac_delays()->set_screen_dim_ms(0);
  expected_policy_screen.mutable_ac_delays()->set_screen_off_ms(0);
  expected_policy_screen.mutable_ac_delays()->set_screen_lock_ms(0);
  expected_policy_screen.mutable_battery_delays()->set_screen_dim_ms(0);
  expected_policy_screen.mutable_battery_delays()->set_screen_off_ms(0);
  expected_policy_screen.mutable_battery_delays()->set_screen_lock_ms(0);

  // With no locks our policy should be the default.
  power_manager::PowerManagementPolicy expected_policy_none;

  // There are eight different possibilities for combinations of
  // different locks, as there are three types. We will go through
  // each possibility by adding or removing one lock.

  // System lock only.
  const int system_id = policy_controller_->AddSystemWakeLock(
      PowerPolicyController::REASON_OTHER, kSystemWakeLockReason);
  expected_policy_system.set_reason(kSystemWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_system),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // System and dim locks.
  const int dim_id = policy_controller_->AddDimWakeLock(
      PowerPolicyController::REASON_OTHER, kDimWakeLockReason);
  expected_policy_dim.set_reason(std::string(kSystemWakeLockReason) + ", " +
                                 kDimWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_dim),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Dim lock only.
  policy_controller_->RemoveWakeLock(system_id);
  expected_policy_dim.set_reason(kDimWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_dim),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Dim and screen locks.
  const int screen_id = policy_controller_->AddScreenWakeLock(
      PowerPolicyController::REASON_OTHER, kScreenWakeLockReason);
  expected_policy_screen.set_reason(std::string(kDimWakeLockReason) + ", " +
                                    kScreenWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_screen),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // System, dim and screen locks.
  const int system_id_2 = policy_controller_->AddSystemWakeLock(
      PowerPolicyController::REASON_OTHER, kSystemWakeLockReason);
  expected_policy_screen.set_reason(std::string(kDimWakeLockReason) + ", " +
                                    std::string(kScreenWakeLockReason) + ", " +
                                    kSystemWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_screen),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // System and screen locks.
  policy_controller_->RemoveWakeLock(dim_id);
  expected_policy_screen.set_reason(std::string(kScreenWakeLockReason) + ", " +
                                    kSystemWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_screen),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Screen lock only.
  policy_controller_->RemoveWakeLock(system_id_2);
  expected_policy_screen.set_reason(kScreenWakeLockReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_screen),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // No locks.
  policy_controller_->RemoveWakeLock(screen_id);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy_none),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));
}

TEST_F(PowerPolicyControllerTest, IgnoreMediaWakeLocksWhenRequested) {
  PowerPolicyController::PrefValues prefs;
  policy_controller_->ApplyPrefs(prefs);
  const power_manager::PowerManagementPolicy kDefaultPolicy =
      fake_power_client_->policy();

  // Wake locks created for audio or video playback should be ignored when the
  // |use_audio_activity| or |use_video_activity| prefs are unset.
  prefs.use_audio_activity = false;
  prefs.use_video_activity = false;
  policy_controller_->ApplyPrefs(prefs);

  const int audio_id = policy_controller_->AddSystemWakeLock(
      PowerPolicyController::REASON_AUDIO_PLAYBACK, "audio");
  const int video_id = policy_controller_->AddScreenWakeLock(
      PowerPolicyController::REASON_VIDEO_PLAYBACK, "video");

  power_manager::PowerManagementPolicy expected_policy = kDefaultPolicy;
  expected_policy.set_use_audio_activity(false);
  expected_policy.set_use_video_activity(false);
  expected_policy.set_reason(PowerPolicyController::kPrefsReason);
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Non-media screen wake locks should still be honored.
  const int other_id = policy_controller_->AddScreenWakeLock(
      PowerPolicyController::REASON_OTHER, "other");

  expected_policy.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy.mutable_ac_delays()->set_screen_dim_ms(0);
  expected_policy.mutable_ac_delays()->set_screen_off_ms(0);
  expected_policy.mutable_ac_delays()->set_screen_lock_ms(0);
  expected_policy.mutable_battery_delays()->set_screen_dim_ms(0);
  expected_policy.mutable_battery_delays()->set_screen_off_ms(0);
  expected_policy.mutable_battery_delays()->set_screen_lock_ms(0);
  expected_policy.set_reason(std::string(PowerPolicyController::kPrefsReason) +
                             ", other");
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  // Start honoring audio activity and check that the audio wake lock is used.
  policy_controller_->RemoveWakeLock(other_id);
  prefs.use_audio_activity = true;
  policy_controller_->ApplyPrefs(prefs);

  expected_policy = kDefaultPolicy;
  expected_policy.set_use_video_activity(false);
  expected_policy.set_ac_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy.set_battery_idle_action(
      power_manager::PowerManagementPolicy_Action_DO_NOTHING);
  expected_policy.set_reason(std::string(PowerPolicyController::kPrefsReason) +
                             ", audio");
  EXPECT_EQ(PowerPolicyController::GetPolicyDebugString(expected_policy),
            PowerPolicyController::GetPolicyDebugString(
                fake_power_client_->policy()));

  policy_controller_->RemoveWakeLock(audio_id);
  policy_controller_->RemoveWakeLock(video_id);
}

TEST_F(PowerPolicyControllerTest, AvoidSendingEmptyPolicies) {
  // Check that empty policies aren't sent when PowerPolicyController is created
  // or destroyed.
  EXPECT_EQ(0, fake_power_client_->num_set_policy_calls());
  PowerPolicyController::Shutdown();
  EXPECT_EQ(0, fake_power_client_->num_set_policy_calls());
}

}  // namespace chromeos
