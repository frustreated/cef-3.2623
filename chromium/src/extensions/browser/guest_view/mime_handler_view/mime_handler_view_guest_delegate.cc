// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest_delegate.h"

namespace extensions {

bool MimeHandlerViewGuestDelegate::OnGuestAttached(
    content::WebContentsView* guest_view,
    content::WebContentsView* parent_view) {
  return false;
}

bool MimeHandlerViewGuestDelegate::OnGuestDetached(
    content::WebContentsView* guest_view,
    content::WebContentsView* parent_view) {
  return false;
}

bool MimeHandlerViewGuestDelegate::CreateViewForWidget(
    content::WebContentsView* guest_view,
    content::RenderWidgetHost* render_widget_host) {
  return false;
}

bool MimeHandlerViewGuestDelegate::HandleContextMenu(
    content::WebContents* web_contents,
    const content::ContextMenuParams& params) {
  return false;
}

}  // namespace extensions