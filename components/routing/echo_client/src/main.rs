use {
    fidl_fidl_example_routing_echo as fecho,
    fuchsia_component::client::connect_to_protocol, tracing::info,
};

async fn main() {
    let echo = connect_to_protocol::<fecho::EchoMarker>().expect("error connecting to echo");
    let out = echo.echo_string(Some("Hippos rule!")).await.expect("echo_string failed");
    info!("{}", out.as_ref().expect("echo_string got empty result"));
}
