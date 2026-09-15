# Source-specific rules

Root AGENTS.md governs this folder. Compile changed C++ with actual UE headers before claiming a pass. Explicitly include the components/types used; do not rely on guessed transitive includes. Keep generated headers last. Do not add separate game-state owners or vendor dependencies without the selected integration's evidence. The installed 5.8 headers, plugin descriptor and actual UBT error are authoritative.

Humanoid ASC must not be reset on remote-control unpossession. Input does not directly award loot/unlocks. Terrain Queued is not completed. Native diagnostic tests prove only their assertions, not the full design acceptance suite. Only authored C++ exists until BOOT-01 runs on the actual engine.
