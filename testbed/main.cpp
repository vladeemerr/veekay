#include <veekay/veekay.hpp>

namespace {

void initialize() {

}

void shutdown() {

}

void update(double time) {

}

void render() {

}

} // namespace

int main() {
	return veekay::run({
		.init = initialize,
		.shutdown = shutdown,
		.update = update,
		.render = render,
	});
}
