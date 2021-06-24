use tracing::info;

#[fuchsia::component]
fn main() {
    info!("Hippo: Hello World!");
}
