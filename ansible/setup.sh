#!/usr/bin/env bash
set -euxo pipefail

PYTHON_VENV_DIR=.local/python/venv

python -m venv ${PYTHON_VENV_DIR}
source ${PYTHON_VENV_DIR}/bin/activate