# Implement a FIDL server
This tutorial shows you how to implement a FIDL protocol and run it on Fuchsia.

##  What do you see when you run the server?
Run the server:
```shell
fx shell run fuchsia-pkg://fuchsia.com/echo-hlcpp-server#meta/echo-server.cmx 
```

You should see the `printf("Running echo server")` output from the `main()` function following by the server hanging.

## How to build the server?
1. Create and run a component
2. Implement the server
3. Serve the protocol

### 1. Create and run a component
#### Create the component
1. Add a `main()` function to `examples/fidl/hlcpp/server/main.cc`

```c++
include <stdio.h>

int main(int argc, const char** argv) {
  printf("Running echo server\n");
  return 0;
}
```
2. Declare a target for the server in `examples/fidl/hlcpp/server/BUILD.gn`
```gn
import("//build/components.gni")

executable("bin") {
    output_name = "fidl_echo_hlcpp_server"
    sources = [ "main.cc" ]
}

fuchsia_component("echo-server") {
    manifest = "meta/server.cmx"
    deps = [ ":bin" ]
}

fuchsia_package("server") {
    package_name = "echo-hlcpp-server"
    deps = [ ":echo-server" ]
}
```
3. Add a component manifest in `examples/fidl/hlcpp/server/meta/server.cmx`
```text
{
  "include": [
    "sdk/lib/diagonstics/syslog/client.shard.cmx"
  ],
  "program": {
    "binary": "bin/fidl_echo_hlcpp_server"
  }
}
```
#### Run the component
1. Add the server to configuration and build it:
```shell
fx set core.x64 --with //examples/fidl/hlcpp/server && fx build
```
2. Ensure `fx serve` is running in a separate tab and connected to an instance of Fuchsia, then run the server:
```shell
fx shell run fuchsia-pkg://fuchsia.com/echo-hlcpp-server#meta/server.cmx
```

### 2. Implement the server
#### Add a dependency on the FIDL library
1. Add `//examples/fidl/fuchsia.examples` to the `deps` of the executables
2. Include the bindings into the main file with `#include <fuchsia/examples/cpp/fidl.h>`
The full `bin` target declaration should look like this:
```gn
executable("bin") {
    output_name = "fidl_echo_hlcpp_server"
    sources = [ "main.cc" ]
    deps = [ "//examples/fidl/fuchsia.examples" ]
}
```

#### Add an implementation for the protocol
Add the following to `main.cc`, above the `main()` function:
```c++
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
```

### 3. Serve the protocol
#### Initialize the event loop
The first aspect is the use of an async loop:
```c++
async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);

return loop.Run();
```

#### Initialize the binding
Then, the code initializes the `fidl::Binding` as mentioned above:
```c++
EchoImpl impl;
fidl::Binding<fuchsia::examples::Echo> binding(&impl);
impl.event_sender_ = &binding.events();
```

#### Define a protocol request handler
Next, the code defines a handler for incoming requests from a client:
```c++
fidl::InterfaceRequestHandler<fuchsia::examples::Echo> handler = 
	[&](fidl::InterfaceRequest<fuchsia::examples::Echo> request) {
      binding.Bind(std::move(request));
    };
```

#### Register the protocol request handler
Finally, the code registers the handler with the component manager:
```c++
auto context = sys::ComponentContext::CreateAndServeOutgoingDirectory();
context->outgoing()->AddPublicService(std::move(handler));
```

#### Add new dependencies
Add include to `main.cc` file:
```c++
#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/fidl/cpp/binding.h>
```
The bin target is like this:
```gn
executable("bin") {
  output_name = "fidl_echo_hlcpp_server"
  sources = [ "main.cc" ]
  deps = [
    "//examples/fidl/fuchsia.examples",
    "//sdk/lib/fidl/cpp",
    "//sdk/lib/sys/cpp",
    "//zircon/system/ulib/async-loop:async-loop-cpp",
    "//zircon/system/ulib/async-loop:async-loop-default",
  ]
}
```
