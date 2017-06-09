// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/mus/public/cpp/window_tree_host_factory.h"

#include "components/mus/public/cpp/window_tree_connection.h"
#include "components/mus/public/cpp/window_tree_delegate.h"
#include "mojo/shell/public/cpp/application_impl.h"

namespace mus {

void CreateWindowTreeHost(mojom::WindowTreeHostFactory* factory,
                          mojom::WindowTreeHostClientPtr host_client,
                          WindowTreeDelegate* delegate,
                          mojom::WindowTreeHostPtr* host,
                          mojom::WindowManagerPtr window_manager,
                          WindowManagerDelegate* window_manager_delegate) {
  mojom::WindowTreeClientPtr tree_client;
  WindowTreeConnection::CreateForWindowManager(
      delegate, GetProxy(&tree_client),
      WindowTreeConnection::CreateType::DONT_WAIT_FOR_EMBED,
      window_manager_delegate);
  factory->CreateWindowTreeHost(GetProxy(host), std::move(host_client),
                                std::move(tree_client),
                                std::move(window_manager));
}

void CreateSingleWindowTreeHost(
    mojo::ApplicationImpl* app,
    mojom::WindowTreeHostClientPtr host_client,
    WindowTreeDelegate* delegate,
    mojom::WindowTreeHostPtr* host,
    mojom::WindowManagerPtr window_manager,
    WindowManagerDelegate* window_manager_delegate) {
  mojom::WindowTreeHostFactoryPtr factory;
  app->ConnectToService("mojo:mus", &factory);
  CreateWindowTreeHost(factory.get(), std::move(host_client), delegate, host,
                       std::move(window_manager), window_manager_delegate);
}

}  // namespace mus
