# Building bambu-studio-fixed against system OpenSSL (for Open Bamboo Networking)

Required because the deps-tree static OpenSSL 1.1.1k exports its symbols and
hijacks any plugin's libssl bindings → TLS crash (OBN issue #110).

## One-time host prep (done 2026-11-17)
- Static 1.1.1k hidden: `deps/build/destdir/usr/local/lib/lib{ssl,crypto}.a`
  → renamed `.111k.disabled`; 1.1.1k headers moved to
  `/tmp/opencode/quarantine/` (destdir + /usr/local/include/openssl).
- deps curl rebuilt against system OpenSSL with SSH disabled.

## Shims (ENGINE_* removed by OpenSSL 4; deps curl 7.75 still references them)
Sources: /tmp/opencode/shims2/openssl-engine-shim.c, ssh-handlers.c
Built into build/src/shim/ (kept out of git; rebuild recipe):
    gcc -O2 -fPIC -shared -o build/src/shim/libopenssl-engine-shim.so shim.c -ldl
    gcc -O2 -fPIC -shared -o build/src/shim/libssh-handlers.so   handlers.c

## Configure
    cmake -B build -G Ninja \
      -DSLIC3R_USE_SYSTEM_OPENSSL=ON \
      -DOPENSSL_ROOT_DIR=/usr -DOPENSSL_SSL_LIBRARY=/usr/lib/openssl-3/libssl.so \
      -DOPENSSL_CRYPTO_LIBRARY=/usr/lib/openssl-3/libcrypto.so \
      -DCMAKE_EXE_LINKER_FLAGS="-L$PWD/build/src/shim -Wl,-rpath,$PWD/build/src/shim -l:libopenssl-engine-shim.so -l:libssh-handlers.so -lz"
(`-L/usr/lib/openssl-3` pins OpenSSL 3 — OpenSSL 4's symbols are versioned
`@OPENSSL_4.0.0` and curl 7.75 needs the 3.x ABI.)

## Verify
    nm -D build/src/bambu-studio | grep -cE ' (T|W) (EVP_|SSL_|OPENSSL_)'   # must be 0
    nm -D build/src/bambu-studio | grep X509_get_default_cert                # must be absent
Plugin: OBN v2.3.0 linux-x64, ABI 02.08.01.99 → ~/.config/BambuStudio/plugins/
(stock backup: plugins/stock-20261117/)
