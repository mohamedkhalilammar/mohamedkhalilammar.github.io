cmake_minimum_required(VERSION 3.22.1)
project(crackingtheshell)

find_package(Python3 REQUIRED)

set(MAZEGEN "${CMAKE_CURRENT_SOURCE_DIR}/generate_decoys.py")
file(GLOB MAZEGEN_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../tools/mazegen/*.py")

# Each library gets its own maze, generated from its own profile seed. The two
# graphs are unrelated on purpose: a script that unpicks one teaches you nothing
# about the other, so the cost of the attack does not transfer between targets.
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/decoys.c"
    COMMAND Python3::Interpreter "${MAZEGEN}" "${CMAKE_CURRENT_SOURCE_DIR}/decoys.c"
            --profile crackingtheshell
    DEPENDS "${MAZEGEN}" ${MAZEGEN_MODULE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/maze_vaultcrypto.c"
    COMMAND Python3::Interpreter "${MAZEGEN}" "${CMAKE_CURRENT_SOURCE_DIR}/maze_vaultcrypto.c"
            --profile vaultcrypto
    DEPENDS "${MAZEGEN}" ${MAZEGEN_MODULE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/maze_chronos.c"
    COMMAND Python3::Interpreter "${MAZEGEN}" "${CMAKE_CURRENT_SOURCE_DIR}/maze_chronos.c"
            --profile chronos
    DEPENDS "${MAZEGEN}" ${MAZEGEN_MODULE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)

add_library(crackingtheshell SHARED
    jni.c
    decoys.c
)

target_link_options(crackingtheshell PRIVATE "-s")

find_library(log-lib log)
if(log-lib)
    target_link_libraries(crackingtheshell ${log-lib})
endif()

# ---------------------------------------------------------------------------
# Target 16 — Sirr.
#
# `nm -D libvaultcrypto.so` must still show sirr_unseal on the first try:
# finding the export is not the challenge, calling it is. Everything else is
# stripped, and the unseal logic itself is split across the maze nodes rather
# than sitting in one readable function, so the static shortcut (reimplement
# the algorithm from the decompilation, skip the device) costs real work while
# the dynamic path the challenge teaches stays wide open.
#
# Kept as a SEPARATE library from crackingtheshell on purpose. Merging them
# would force one binary to be both hardened and legible, and target 15's
# intended solve is patching and re-signing its lib — no other challenge's
# material should be riding in the binary players are told to tamper with.
# ---------------------------------------------------------------------------
add_library(vaultcrypto SHARED
    vaultcrypto.c
    maze_vaultcrypto.c
)

target_link_options(vaultcrypto PRIVATE "-s")

# Designer mode, driven from gradle by -PctfDesigner=<value>. Off in every shipped
# build: it exposes sirr_designer_open and lets SX_DESIGNER_ID force the process
# identity the cascade binds to, so testing does not need a device and Frida.
if(SX_DESIGNER)
    target_compile_definitions(vaultcrypto PRIVATE SX_DESIGNER)
    target_compile_definitions(crackingtheshell PRIVATE SX_DESIGNER)
endif()

if(log-lib)
    target_link_libraries(vaultcrypto ${log-lib})
endif()

# ---------------------------------------------------------------------------
# SecureStore (renamed from FlagVault) — hardening item #2. Backs challenges
# 3, 4, 5, 6 and 8, plus decoy slots mixed in alongside the real ones.
#
# Same posture as libcrackingtheshell.so: stripped, dynamic RegisterNatives,
# no ciphertext-adjacent key. Kept as its own library rather than folded into
# vaultcrypto.c, which carries target 16's material.
# ---------------------------------------------------------------------------
add_library(securestore SHARED
    securestore.c
)

target_link_options(securestore PRIVATE "-s")

if(log-lib)
    target_link_libraries(securestore ${log-lib})
endif()

# ---------------------------------------------------------------------------
# Target 17 — Second Hand.
#
# A FOURTH library, deliberately. It carries K and the flag ciphertext for a
# challenge whose intended solve is a Frida hook on the Java clock, so none of
# its material may ride in a binary the player already has a reason to open:
# vaultcrypto carries target 16's note, and target 15's players are told to
# patch crackingtheshell and re-sign it. Its maze is generated from its own
# profile seed, so unpicking either of the other two teaches nothing here.
#
# Nothing is exported by name. JNI_OnLoad + RegisterNatives only, stripped.
# ---------------------------------------------------------------------------
add_library(chronos SHARED
    chronos.c
    maze_chronos.c
)

target_link_options(chronos PRIVATE "-s")

# Declared after the target exists, unlike the block above: the maze's identity
# node reads SX_DESIGNER_ID instead of /proc/self/cmdline under it, which is what
# lets tools/verify_chronos.py exercise the real cascade off-device.
if(SX_DESIGNER)
    target_compile_definitions(chronos PRIVATE SX_DESIGNER)
endif()

if(log-lib)
    target_link_libraries(chronos ${log-lib})
endif()
