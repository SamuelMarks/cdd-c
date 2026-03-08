#!/bin/sh

# shellcheck disable=SC2236
if [ ! -z "${BASH_VERSION+x}" ]; then
  # shellcheck disable=SC3028 disable=SC3054
  this_file="${BASH_SOURCE[0]}"
  # shellcheck disable=SC3040
  set -o pipefail
elif [ ! -z "${ZSH_VERSION+x}" ]; then
  # shellcheck disable=SC2296
  this_file="${(%):-%x}"
  # shellcheck disable=SC3040
  set -o pipefail
else
  this_file="${0}"
fi
ROOT="$( cd "$( dirname -- "${this_file}" )" && pwd )"

# TODO: .pre-commit-config.yaml instead of this

COVERAGE="$(cd "${ROOT}"'/cmake-build-debug' && ctest -C 'Debug' -T 'Coverage' 2>/dev/null | awk 'END {print $NF}')"
printf '<svg xmlns="http://www.w3.org/2000/svg" width="114" height="20" role="img" aria-label="coverage: %s"><title>coverage: %s</title><linearGradient id="s" x2="0" y2="%s"><stop offset="0" stop-color="#bbb" stop-opacity=".1"/><stop offset="1" stop-opacity=".1"/></linearGradient><clipPath id="r"><rect width="114" height="20" rx="3" fill="#fff"/></clipPath><g clip-path="url(#r)"><rect width="61" height="20" fill="#555"/><rect x="61" width="53" height="20" fill="#97ca00"/><rect width="114" height="20" fill="url(#s)"/></g><g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" text-rendering="geometricPrecision" font-size="110"><text aria-hidden="true" x="315" y="150" fill="#010101" fill-opacity=".3" transform="scale(.1)" textLength="510">coverage</text><text x="315" y="140" transform="scale(.1)" fill="#fff" textLength="510">coverage</text><text aria-hidden="true" x="865" y="150" fill="#010101" fill-opacity=".3" transform="scale(.1)" textLength="430">%s</text><text x="865" y="140" transform="scale(.1)" fill="#fff" textLength="430">%s</text></g></svg>' "${COVERAGE}" "${COVERAGE}" "${COVERAGE}" "${COVERAGE}" "${COVERAGE}" > "${ROOT}"'/reports/test_coverage.svg'
