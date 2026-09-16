<!--
Copyright (C) 2026 EloqData Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# Replace unreleased failover formats without compatibility

## Status

Accepted

The failover command, snapshot, Full Desired State, and Operation-result
layouts are not deployed to users. The simplified design therefore replaces
their schemas without a legacy decoder, dual write, or mixed-version
negotiation. A format version may advance to make an old durable or wire object
fail closed at its outer envelope; that discriminator does not create a
compatibility path. Only the resulting current binaries, state, and wire
layouts are supported. Old binaries and old persisted formats are outside the
product contract.
