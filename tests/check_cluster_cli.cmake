# Copyright (C) 2026 EloqData Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(NOT DEFINED KEYLANE_EXECUTABLE)
  message(FATAL_ERROR "KEYLANE_EXECUTABLE is required")
endif()

execute_process(
  COMMAND "${KEYLANE_EXECUTABLE}" --help
  RESULT_VARIABLE help_result
  OUTPUT_VARIABLE help_stdout
  ERROR_VARIABLE help_stderr
)
set(help_text "${help_stdout}${help_stderr}")
if(NOT help_result EQUAL 0)
  message(FATAL_ERROR "keylane --help failed: ${help_text}")
endif()
foreach(required IN ITEMS "--cluster-node-id" "--cluster-meta-seed")
  string(FIND "${help_text}" "${required}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "keylane --help omitted ${required}")
  endif()
endforeach()
