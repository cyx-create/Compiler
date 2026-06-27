RM = rm -rf
MAKEFLAGS = --no-print-directory

BUILD_DIR = $(CURDIR)/build
TEST_DIR = $(CURDIR)/test
FDMJC = $(BUILD_DIR)/tools/main/fdmjc
LIBSYSY = $(CURDIR)/vendor/libsysy/libsysy32.s
GCC = arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -static -Wl,-z,noexecstack -lm
QEMU = qemu-arm

.PHONY: all build sync compile run run-full run-const run-loop1 run-loop2 run-allloop run-allopt clean rebuild verify verify-quads verify-ir verify-e2e verify-e2e-quick verify-e2e-full

all: build

sync:
	@bash scripts/sync_sources.sh

setup-test:
	@bash scripts/setup_test.sh

build: sync
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@ninja -C $(BUILD_DIR)

compile: build setup-test
	@for f in $(TEST_DIR)/*.fmj; do \
		base=$${f%.fmj}; \
		name=$$(basename $$base); \
		echo "Compiling $$name ..."; \
		$(FDMJC) $(TEST_DIR) $$name all || exit 1; \
	done

run: compile
	@python3 scripts/run_tests.py base --tier all

run-full: compile
	@python3 scripts/run_tests.py base --tier all

run-const: compile
	@python3 scripts/run_tests.py const

run-loop1: compile
	@python3 scripts/run_tests.py loop1

run-loop2: compile
	@python3 scripts/run_tests.py loop2

run-allloop: compile
	@python3 scripts/run_tests.py allloop

run-allopt: compile
	@python3 scripts/run_tests.py allopt

clean:
	@$(RM) $(BUILD_DIR)
	@find $(TEST_DIR) -type f ! -name '*.fmj' ! -name '*.2-semant.ast' ! -name '*.4-xml.quad' ! -name '*.4.quad' ! -name '*.5-xml.asm' ! -name '*.4-ssa-withflow-xml.quad' ! -name '*.4-ssa.quad' -delete 2>/dev/null || true

rebuild: clean build

verify-quads: build setup-test
	@bash scripts/verify_quads.sh

verify-ir: build setup-test
	@bash scripts/verify_ir.sh

verify: verify-quads verify-ir verify-e2e-quick

verify-e2e-quick: build setup-test
	@bash scripts/verify_e2e.sh --quick

verify-e2e: build setup-test
	@bash scripts/verify_e2e.sh

verify-e2e-full: build setup-test
	@bash scripts/verify_e2e.sh --full
