PYTHON3 ?= python3
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

check: ${ROOT_DIR}/test_tracer ${ROOT_DIR}/test_error
	$(PYTHON3) ${ROOT_DIR}/test_wrapper.py
	$(PYTHON3) ${ROOT_DIR}/test_asm.py
	$(PYTHON3) ${ROOT_DIR}/test_tracer.py

${ROOT_DIR}/test_tracer: ${ROOT_DIR}/test_tracer.c
	gcc -Og -g -o $@ $<

${ROOT_DIR}/test_error: ${ROOT_DIR}/test_error.c
	gcc -Og -g -o $@ $<
