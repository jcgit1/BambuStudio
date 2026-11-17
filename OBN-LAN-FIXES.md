# OBN + Bambu Studio — LAN fixes for H2C/H2S (cachylappy, 2026-11-17)

H2-series printers send **no SSDP NOTIFY** and Bambu Cloud's `/user/print`
returns **no LAN IPs**, so Open Bamboo Networking (OBN) can never form the
`ip <-> serial` pair on its own, and Studio's device list stayed empty under
`block_cloud = 1`. Fixes (all marked `OBN fix` in the sources):

## 1. Cloud-server-ready bootstrap (OBN)
`src/abi_cloud.cpp` + `src/agent.cpp` (`notify_server_ready_when_blocked()`):
with `block_cloud = 1` the cloud MQTT never connects, so Studio's
`on_server_connected` callback (which gates the whole device-list load) never
fires. OBN now reports a one-shot fake `server_connected(OK)` when
`connect_server()` is called in blocked mode.

## 2. Machine list always refreshes (Studio)
`src/slic3r/GUI/GUI_App.cpp` (~2105): fire `EVT_UPDATE_MACHINE_LIST`
unconditionally in the server-connected handler. Upstream gates it on the
`enable_multi_machine` config key (nested under `app` in `BambuStudio.conf`).

## 3. IP seed store (OBN) — the key fix for H2
`include/obn/state.hpp` + `src/state.cpp` + `src/abi_http.cpp` + `src/agent.cpp`:
`obn.state.json` gains an optional `lan_ips` map; after every `/user/print`
fetch the plugin seeds `ip <-> serial` for each device, which completes the
credential pair (access codes already arrive from the cloud) and triggers
`autostart_lan_if_selected()` -> LAN MQTT (bblp + access code, port 8883 TLS).

Seeded on this machine (`~/.config/BambuStudio/obn.state.json`):

```json
{
  "selected_machine": "0938AC580200650",
  "lan_ips": { "0938AC580200650": "192.168.1.230",
               "31B8AK632300519": "192.168.1.163" }
}
```

## 4. bind_state passthrough (Studio, standby)
`GUI_App.cpp` (~8493) + `ReleaseNote.cpp` accept `occupied_obn` so the
"Bind with Access Code" dialog could onboard a cloud-bound printer. Not needed
since fix 3 made the printers connect directly, but harmless.

## Build
- OBN: `/tmp/obnsync` (master 77de55a + fixes). Build needs
  `CPATH=/tmp/uthash-inc ./configure --no-conf-patch --with-version=02.08.01.99
  --cmake-arg=-DOPENSSL_ROOT_DIR=/usr/lib/openssl-3 --cmake-arg=-DCMAKE_PREFIX_PATH=/usr/lib/openssl-3 && make`.
  (`uthash` headers fetched to /tmp/uthash-inc; `configure`'s header probe
  extended to look there. OpenSSL 3 ABI required; system OpenSSL is 4.1.0.)
  Also fixed for OpenSSL 4 headers: `signing.cpp` const `X509_NAME`,
  `abi_bind.cpp` `BBL::detectResult` qualification.
- Installed to `~/.config/BambuStudio/plugins/libbambu_networking.so`
  (old copies: `plugins/stock-20261117/`, `plugins/obn-v2.3.0.pre-fix.so.bak`).
- Studio: `~/bambu-studio-fixed` (see BUILD-OPENSSL3.md), target `BambuStudio`.
- Restart helper: `~/scripts/restart-bambu/launch.sh`.

## Verified
- H2C `31B8AK632300519` @ 192.168.1.163: LAN MQTT connected, telemetry
  streaming (~12.7 KB push_status every second).
- H2S `0938AC580200650` @ 192.168.1.230: LAN MQTT connected, telemetry
  streaming (~23.8 KB reports) when selected.
- Only one LanSession at a time by design (Studio single-active-printer model);
  switching printers in the sidebar re-dials LAN for the newly selected one.
- Zero split-lock traps from the new plugin since install (journal hits are
  all pre-fix, before 14:20).

## Not yet worn in
- Send an actual print to each printer over LAN.
- Camera liveview / file browser (H-series camera "not tested" upstream).
