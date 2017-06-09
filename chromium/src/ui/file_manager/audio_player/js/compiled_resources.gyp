# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'background',
      'variables': {
        'depends': [
          '../../../../third_party/jstemplate/compiled_resources.gyp:jstemplate',
          '../../../webui/resources/js/cr.js',
          '../../../webui/resources/js/cr/event_target.js',
          '../../../webui/resources/js/cr/ui.js',
          '../../../webui/resources/js/cr/ui/array_data_model.js',
          # Referenced in common/js/util.js.
          '../../../webui/resources/js/cr/ui/dialogs.js',
          '../../../webui/resources/js/load_time_data.js',
          '../../../webui/resources/js/util.js',
          '../../file_manager/common/js/util.js',
          '../../file_manager/common/js/async_util.js',
          '../../file_manager/common/js/file_type.js',
          '../../file_manager/common/js/metrics_base.js',
          '../../file_manager/common/js/metrics_events.js',
          '../../file_manager/common/js/metrics.js',
          '../../file_manager/common/js/volume_manager_common.js',
          '../../file_manager/background/js/app_window_wrapper.js',
          '../../file_manager/background/js/background_base.js',
          '../../file_manager/background/js/test_util_base.js',
          '../../file_manager/background/js/volume_manager.js',
          'error_util.js',
          'test_util.js',
        ],
        'externs': [
          '<(EXTERNS_DIR)/chrome_send.js',
          '<(EXTERNS_DIR)/chrome_extensions.js',
          '<(EXTERNS_DIR)/file_manager_private.js',
          '<(EXTERNS_DIR)/metrics_private.js',
          '../../../../third_party/analytics/externs.js',
          '../../externs/chrome_test.js',
          '../../externs/es6_workaround.js',
          '../../externs/platform.js',
        ],
      },
      'includes': [
        '../../compile_js.gypi',
      ],
    },
    {
      'target_name': 'audio_player',
      'variables': {
        'depends': [
          '../../../../third_party/jstemplate/compiled_resources.gyp:jstemplate',
          '../../../../third_party/polymer/v1_0/components-chromium/iron-a11y-keys-behavior/iron-a11y-keys-behavior-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/iron-behaviors/iron-button-state-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/iron-behaviors/iron-control-state-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/iron-form-element-behavior/iron-form-element-behavior-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/iron-range-behavior/iron-range-behavior-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/paper-behaviors/paper-inky-focus-behavior-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/paper-behaviors/paper-ripple-behavior-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/paper-progress/paper-progress-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/paper-ripple/paper-ripple-extracted.js',
          '../../../../third_party/polymer/v1_0/components-chromium/paper-slider/paper-slider-extracted.js',
          '../../../webui/resources/js/cr.js',
          '../../../webui/resources/js/cr/event_target.js',
          '../../../webui/resources/js/cr/ui.js',
          '../../../webui/resources/js/cr/ui/array_data_model.js',
          '../../../webui/resources/js/cr/ui/dialogs.js',
          '../../../webui/resources/js/load_time_data.js',
          '../../../webui/resources/js/util.js',
          '../../file_manager/common/js/async_util.js',
          '../../file_manager/common/js/file_type.js',
          '../../file_manager/common/js/lru_cache.js',
          '../../file_manager/common/js/metrics_base.js',
          '../../file_manager/common/js/metrics_events.js',
          '../../file_manager/common/js/metrics.js',
          '../../file_manager/common/js/util.js',
          '../../file_manager/common/js/volume_manager_common.js',
          '../../file_manager/foreground/js/metadata/content_metadata_provider.js',
          '../../file_manager/foreground/js/metadata/external_metadata_provider.js',
          '../../file_manager/foreground/js/metadata/file_system_metadata_provider.js',
          '../../file_manager/foreground/js/metadata/metadata_cache_item.js',
          '../../file_manager/foreground/js/metadata/metadata_cache_set.js',
          '../../file_manager/foreground/js/metadata/metadata_item.js',
          '../../file_manager/foreground/js/metadata/metadata_model.js',
          '../../file_manager/foreground/js/metadata/multi_metadata_provider.js',
          '../../file_manager/foreground/js/metadata/new_metadata_provider.js',
          '../../file_manager/foreground/js/metadata/thumbnail_model.js',
          '../../file_manager/background/js/volume_manager.js',
          '../../file_manager/foreground/js/volume_manager_wrapper.js',
          '../elements/control_panel.js',
          '../elements/track_list.js',
          '../elements/audio_player.js',
        ],
        'externs': [
          '<(EXTERNS_DIR)/chrome_extensions.js',
          '<(EXTERNS_DIR)/chrome_send.js',
          '<(EXTERNS_DIR)/file_manager_private.js',
          '<(EXTERNS_DIR)/metrics_private.js',
          '../../../../third_party/analytics/externs.js',
          '../../externs/audio_player_foreground.js',
          '../../externs/chrome_test.js',
          '../../externs/es6_workaround.js',
          '../../externs/platform.js',
        ],
      },
      'includes': [
        '../../compile_js.gypi',
      ],
    }
  ],
}

