#!/bin/bash
# Check GPL v2 license headers in C source files
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# GPL v2 license header template
GPL_HEADER="/*
 * This file is part of FMI/OS.
 *
 * FMI/OS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * FMI/OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FMI/OS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */"

EXIT_CODE=0

# Function to check if file has GPL v2 header
check_license_header() {
	local file="$1"

	# Skip if file doesn't exist
	if ! test -f "${file}"; then
		return 0
	fi

	# Check if file starts with GPL v2 header
	if ! head -n 16 "${file}" | grep -q "GNU General Public License"; then
		echo "ERROR: Missing GPL v2 license header in: ${file}"
		echo "Please add the following header to the beginning of the file:"
		echo "${GPL_HEADER}"
		echo ""
		EXIT_CODE=1
	elif ! head -n 16 "${file}" | grep -q "version 2 of the License"; then
		echo "ERROR: Incorrect GPL license version in: ${file}"
		echo "Please ensure the header specifies 'version 2 of the License'"
		echo ""
		EXIT_CODE=1
	fi
}

# Check all provided files
for file in "$@"; do
	case "${file}" in
		*.c|*.h)
			check_license_header "${file}"
			;;
	esac
done

# If no files provided, check all C/H files in the repository
if test $# -eq 0; then
	# Find all C and H files, excluding hidden directories
	find . -name "*.c" -o -name "*.h" | while read -r file; do
		case "${file}" in
			./.git/*|./.kiro/*)
				# Skip git and kiro directories
				;;
			*)
				check_license_header "${file}"
				;;
		esac
	done
fi

exit ${EXIT_CODE}
