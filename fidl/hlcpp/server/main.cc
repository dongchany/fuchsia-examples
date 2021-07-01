// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/examples/cpp/fidl.h>
#include <stdio.h>

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/fidl/cpp/binding.h>

class EchoImpl : public fuchsia::examples::Echo {
 public:
  void EchoString(std::string value, EchoStringCallback callback) override { callback(value); }
  void SendString(std::string value) override {
    if (event_sender_) {
      event_sender_->OnString(value);
    }
  }

  fuchsia::examples::Echo_EventSender* event_sender_;
};

int main(int argc, const char** argv) {
  async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);

  EchoImpl impl;
  fidl::Binding<fuchsia::examples::Echo> binding(&impl);
  impl.event_sender_ = &binding.events();
  fidl::InterfaceRequestHandler<fuchsia::examples::Echo> handler =
      [&](fidl::InterfaceRequest<fuchsia::examples::Echo> request) {
        binding.Bind(std::move(request));
      };
  auto context = sys::ComponentContext::CreateAndServeOutgoingDirectory();
  context->outgoing()->AddPublicService(std::move(handler));

  printf("Running echo server\n");
  return loop.Run();
}
