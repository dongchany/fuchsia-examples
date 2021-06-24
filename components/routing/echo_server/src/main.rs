use {
    fidl_fidl_example_routing_echo as fecho, fuchsia_async as fasync,
    fuchsia_component::server::ServiceFs,
};

fn main() {
    let mut executor = fasync::LocalExecutor::new().expect("error creating executor");
    let mut fs = ServiceFs::new_local();
    fs.dir("svc").add_fidl_service(move |stream| {
        fasync::Task::local(async move {
            run_echo_service(stream).await;
        })
        .detach();
    });

}

async fn run_echo_service(mut stream: fecho::EchoRequestStream) {
    while let Some(event) = stream.try_next().await.expect("failed to serve echo service") {
        let fecho::EchoRequest::EchoString { value, response } = event;
        response.send(value.as_ref().map(|s| &**s)).expect("failed to send echo response");
    }
}
