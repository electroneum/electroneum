#!/usr/bin/env bash
#
# Local testnet migration test setup
# ===================================
# Prerequisites: rebuild electroneumd + electroneum-wallet-rpc with the toy
# hardfork table and mocked validator signatures (see MIGRATION-TEST-INSTRUCTIONS.txt).
#
# Usage:
#   ./migration-test.sh          # start daemons + auto-mine
#   ./migration-test.sh stop     # kill daemons
#   ./migration-test.sh clean    # wipe blockchain data dirs

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ── Binary location (build output) ──────────────────────────────────────────
# Points at CLion's debug build. Override via BIN env var if needed:
#   BIN=/path/to/bin ./migration-test.sh
BIN="${BIN:-$HOME/electroneum/cmake-build-debug/bin}"

# ── Burn address (fixed — HF11+ coinbase target) ─────────────────────────────
BURN_ADDR="etnkCys4uGhSi9h48ajL9vBDJTcn2s2ttXtXq3SXWPAbiMHNhHitu5fJ8QgRfFWTzmJ8QgRfFWTzmJ8QgRfFWTzm4t51HTfCtK"

# Miner addresses are derived dynamically from the seeds below (see step 5
# where the wallets are restored) so they always match whichever binary is
# used — different builds may derive different addresses from the same seed.
MINER1_ADDR=""
MINER2_ADDR=""

# ── Mnemonic seeds (for reference / recovery) ────────────────────────────────
# Miner 1: keyboard suddenly wise baby tanks logic wizard lofty
#           fully ethics spout kisses jellyfish losing balding pixels
#           eggs occur enlist pact onto hounded rays rockets onto
#
# Miner 2: fences sifting fudge urchins onboard pact incur etched
#           input building inkling abrasive different dash situated byline
#           negative aplomb phone tutor pirate cafe dubbed enraged dubbed

# ── Port layout ──────────────────────────────────────────────────────────────
#                  Daemon 1       Daemon 2
# P2P              34567          34569
# RPC              34568          34570
# ZMQ              34582          34584
# ─────────────────────────────────────────────────────────────────────────────

DATA1="$SCRIPT_DIR/testnet-data-1"
DATA2="$SCRIPT_DIR/testnet-data-2"
LOG1="$DATA1/testnet/electroneum.log"
LOG2="$DATA2/testnet/electroneum.log"
TARGET_HEIGHT=150   # mine to this height (well past HF11@60 + unlock window)
WALLETS_DIR="$SCRIPT_DIR/test-wallets"

# ── Mnemonic seeds ──────────────────────────────────────────────────────────
MINER1_SEED="keyboard suddenly wise baby tanks logic wizard lofty fully ethics spout kisses jellyfish losing balding pixels eggs occur enlist pact onto hounded rays rockets onto"
MINER2_SEED="fences sifting fudge urchins onboard pact incur etched input building inkling abrasive different dash situated byline negative aplomb phone tutor pirate cafe dubbed enraged dubbed"

# ── Helper: get daemon height ────────────────────────────────────────────────
get_height() {
    local port=$1
    curl -s --max-time 5 http://127.0.0.1:${port}/json_rpc \
        -d '{"jsonrpc":"2.0","id":"0","method":"get_info"}' \
        -H 'Content-Type: application/json' 2>/dev/null \
    | python3 -c "import sys,json; print(json.load(sys.stdin)['result']['height'])" 2>/dev/null || echo "0"
}

# ── Helper: start mining via RPC (with full response logging) ────────────────
start_mining() {
    local port=$1
    local addr=$2
    local label=$3
    echo "  [start_mining] $label on port $port"
    echo "  [start_mining] address: ${addr:0:20}...${addr: -10}"
    local resp
    resp=$(curl -s --max-time 10 http://127.0.0.1:${port}/start_mining \
        -d "{\"do_background_mining\":false,\"ignore_battery\":true,\"miner_address\":\"${addr}\",\"threads_count\":1}" \
        -H 'Content-Type: application/json' 2>&1)
    echo "  [start_mining] response: $resp"
}

# ── Helper: stop mining via RPC (with full response logging) ─────────────────
stop_mining() {
    local port=$1
    local label=$2
    echo "  [stop_mining] $label on port $port"
    local resp
    resp=$(curl -s --max-time 10 http://127.0.0.1:${port}/stop_mining \
        -H 'Content-Type: application/json' 2>&1)
    echo "  [stop_mining] response: $resp"
}

# ── Helper: test block template creation ─────────────────────────────────────
test_block_template() {
    local port=$1
    local addr=$2
    local label=$3
    echo "  [block_template] Testing template creation on $label..."
    local resp
    resp=$(curl -s --max-time 10 http://127.0.0.1:${port}/json_rpc \
        -d "{\"jsonrpc\":\"2.0\",\"id\":\"0\",\"method\":\"get_block_template\",\"params\":{\"wallet_address\":\"${addr}\",\"reserve_size\":0}}" \
        -H 'Content-Type: application/json' 2>&1)
    if echo "$resp" | grep -q '"error"'; then
        echo "  [block_template] FAILED: $resp"
    else
        local height
        height=$(echo "$resp" | python3 -c "import sys,json; print(json.load(sys.stdin)['result']['height'])" 2>/dev/null || echo "?")
        echo "  [block_template] OK — template for height $height"
    fi
}

# ── Helper: dump recent daemon log errors ────────────────────────────────────
dump_recent_errors() {
    local logfile=$1
    local label=$2
    local lines=${3:-10}
    if [[ -f "$logfile" ]]; then
        local errors
        errors=$(grep -i "error\|failed\|MERROR" "$logfile" | tail -"$lines" 2>/dev/null || true)
        if [[ -n "$errors" ]]; then
            echo "  [$label] Recent errors in log:"
            echo "$errors" | while IFS= read -r line; do echo "    $line"; done
        else
            echo "  [$label] No errors in log"
        fi
    else
        echo "  [$label] Log file not found: $logfile"
    fi
}

# ── Preflight ────────────────────────────────────────────────────────────────
if [[ ! -x "$BIN/electroneumd" ]]; then
    echo "ERROR: electroneumd not found at $BIN/electroneumd"
    echo "  Build it first, or adjust the BIN variable at the top of this script."
    exit 1
fi

# ── Stop mode ────────────────────────────────────────────────────────────────
if [[ "${1:-}" == "stop" ]]; then
    echo "Stopping daemons..."
    pkill -f "electroneumd.*testnet-data-1" 2>/dev/null && echo "Daemon 1 stopped" || echo "Daemon 1 not running"
    pkill -f "electroneumd.*testnet-data-2" 2>/dev/null && echo "Daemon 2 stopped" || echo "Daemon 2 not running"
    exit 0
fi

# ── Clean mode ───────────────────────────────────────────────────────────────
if [[ "${1:-}" == "clean" ]]; then
    echo "Removing old testnet data..."
    rm -rf "$DATA1" "$DATA2"
    echo "Removing wallet files..."
    rm -f "$WALLETS_DIR/miner1" "$WALLETS_DIR/miner1.keys" "$WALLETS_DIR/miner1.address.txt"
    rm -f "$WALLETS_DIR/miner2" "$WALLETS_DIR/miner2.keys" "$WALLETS_DIR/miner2.address.txt"
    echo "Done. Run again without arguments to start fresh."
    exit 0
fi

# ── Start ────────────────────────────────────────────────────────────────────
mkdir -p "$DATA1" "$DATA2"

echo "============================================================"
echo "  LOCAL TESTNET MIGRATION TEST"
echo "============================================================"
echo ""
echo "Binaries:   $BIN"
echo "Test files: $SCRIPT_DIR"
echo ""
echo "Toy hardfork table (testnet):"
echo "  HF1  @ height 1   (genesis)"
echo "  HF6  @ height 10"
echo "  HF7  @ height 20"
echo "  HF8  @ height 30  (public tx / mocked validator signatures)"
echo "  HF9  @ height 40"
echo "  HF10 @ height 50"
echo "  HF11 @ height 60  (migration fork — coinbase must go to burn addr)"
echo ""
echo "Mining plan:"
echo "  Phase 1: mine to miner wallets (blocks 1-59, pre-HF11)"
echo "  Phase 2: mine to burn address  (blocks 60+, HF11 requires it)"
echo "  Target:  height $TARGET_HEIGHT (ensures coinbase from phase 1 unlocks)"
echo ""

# ── Launch Daemon 1 ──────────────────────────────────────────────────────────
echo "[1/4] Starting Daemon 1 (P2P=34567, RPC=34568)..."
"$BIN/electroneumd" \
    --testnet \
    --data-dir "$DATA1" \
    --p2p-bind-port 34567 \
    --rpc-bind-port 34568 \
    --zmq-rpc-bind-port 34582 \
    --rpc-bind-ip 127.0.0.1 \
    --confirm-external-bind \
    --add-exclusive-node 127.0.0.1:34569 \
    --fixed-difficulty 1 \
    --no-igd \
    --hide-my-port \
    --log-level 0 \
    --detach

sleep 2

# ── Launch Daemon 2 ──────────────────────────────────────────────────────────
echo "[2/4] Starting Daemon 2 (P2P=34569, RPC=34570)..."
"$BIN/electroneumd" \
    --testnet \
    --data-dir "$DATA2" \
    --p2p-bind-port 34569 \
    --rpc-bind-port 34570 \
    --zmq-rpc-bind-port 34584 \
    --rpc-bind-ip 127.0.0.1 \
    --confirm-external-bind \
    --add-exclusive-node 127.0.0.1:34567 \
    --fixed-difficulty 1 \
    --no-igd \
    --hide-my-port \
    --log-level 0 \
    --detach

sleep 2

# Verify both are up
H1=$(get_height 34568)
H2=$(get_height 34570)
echo "  Daemon 1 height: $H1"
echo "  Daemon 2 height: $H2"
if [[ "$H1" == "0" || "$H2" == "0" ]]; then
    echo "ERROR: one or both daemons failed to start."
    dump_recent_errors "$LOG1" "Daemon 1" 20
    dump_recent_errors "$LOG2" "Daemon 2" 20
    exit 1
fi
echo ""

# ══════════════════════════════════════════════════════════════════════════════
# Restore wallets now so we know which address to mine to.
# Different builds of electroneum-wallet-cli can derive different addresses
# from the same seed — so we always derive fresh from the binary in $BIN.
# ══════════════════════════════════════════════════════════════════════════════
echo "[restore] Restoring miner wallets from seed (password: 'password')..."
mkdir -p "$WALLETS_DIR"
rm -f "$WALLETS_DIR/miner1" "$WALLETS_DIR/miner1.keys" "$WALLETS_DIR/miner1.address.txt"
rm -f "$WALLETS_DIR/miner2" "$WALLETS_DIR/miner2.keys" "$WALLETS_DIR/miner2.address.txt"

echo -e "$WALLETS_DIR/miner1\npassword\npassword\n0\n" | "$BIN/electroneum-wallet-cli" \
    --testnet --restore-deterministic-wallet --electrum-seed "$MINER1_SEED" \
    --restore-height 0 --daemon-address 127.0.0.1:34568 --daemon-ssl disabled \
    --log-level 0 --command exit > /dev/null 2>&1 \
    || { echo "ERROR: miner1 restore failed"; exit 1; }

echo -e "$WALLETS_DIR/miner2\npassword\npassword\n0\n" | "$BIN/electroneum-wallet-cli" \
    --testnet --restore-deterministic-wallet --electrum-seed "$MINER2_SEED" \
    --restore-height 0 --daemon-address 127.0.0.1:34568 --daemon-ssl disabled \
    --log-level 0 --command exit > /dev/null 2>&1 \
    || { echo "ERROR: miner2 restore failed"; exit 1; }

MINER1_ADDR=$(cat "$WALLETS_DIR/miner1.address.txt")
MINER2_ADDR=$(cat "$WALLETS_DIR/miner2.address.txt")
echo "  Miner 1 address: $MINER1_ADDR"
echo "  Miner 2 address: $MINER2_ADDR"
echo ""

# ══════════════════════════════════════════════════════════════════════════════
# PHASE 1: Mine to miner wallets (pre-HF11, height < 60)
# ══════════════════════════════════════════════════════════════════════════════
echo "[3/4] Phase 1: Mining to miner wallets (blocks up to height 55)..."
echo "  (stopping at 55 to avoid race with HF11 burn-address requirement at 60)"
echo ""
start_mining 34568 "$MINER1_ADDR" "Daemon 1 -> Miner 1"
start_mining 34570 "$MINER2_ADDR" "Daemon 2 -> Miner 2"
echo ""

STALL_COUNT=0
LAST_H=0
while true; do
    H=$(get_height 34568)
    printf "\r  Height: %s / 55  " "$H"

    # Detect stall — height hasn't changed for 10 seconds
    if [[ "$H" == "$LAST_H" ]]; then
        STALL_COUNT=$((STALL_COUNT + 1))
    else
        STALL_COUNT=0
        LAST_H="$H"
    fi

    if [[ "$STALL_COUNT" -ge 10 ]]; then
        echo ""
        echo "  WARNING: Mining appears stalled at height $H for 10s"
        dump_recent_errors "$LOG1" "Daemon 1" 5
        dump_recent_errors "$LOG2" "Daemon 2" 5
        test_block_template 34568 "$MINER1_ADDR" "Daemon 1"
        STALL_COUNT=0
    fi

    # Stop at 55 — gives margin before HF11 at height 60
    if [[ "$H" -ge 55 ]]; then
        echo ""
        break
    fi
    sleep 1
done

echo "  Phase 1 complete — miner wallets have coinbase from blocks 1-55"
echo ""

# ══════════════════════════════════════════════════════════════════════════════
# PHASE TRANSITION: Stop miners, switch to burn address
# ══════════════════════════════════════════════════════════════════════════════
echo "── Phase transition: stopping miners and switching to burn address ──"
echo ""

stop_mining 34568 "Daemon 1"
stop_mining 34570 "Daemon 2"

sleep 2

echo ""
echo "  Verifying height after stop..."
H1=$(get_height 34568)
H2=$(get_height 34570)
echo "  Daemon 1 height: $H1"
echo "  Daemon 2 height: $H2"
echo ""

echo "  Checking for errors before starting phase 2..."
dump_recent_errors "$LOG1" "Daemon 1" 5
dump_recent_errors "$LOG2" "Daemon 2" 5
echo ""

echo "  Testing block template with burn address before starting miners..."
test_block_template 34568 "$BURN_ADDR" "Daemon 1"
test_block_template 34570 "$BURN_ADDR" "Daemon 2"
echo ""

# ══════════════════════════════════════════════════════════════════════════════
# PHASE 2: Mine to burn address (HF11+, coinbase must go to burn)
# ══════════════════════════════════════════════════════════════════════════════
echo "[4/4] Phase 2: Mining to burn address (blocks 60-$TARGET_HEIGHT)..."
echo ""
start_mining 34568 "$BURN_ADDR" "Daemon 1 -> Burn"
start_mining 34570 "$BURN_ADDR" "Daemon 2 -> Burn"
echo ""

sleep 2

echo "  Quick post-start check..."
H_AFTER=$(get_height 34568)
echo "  Height 2s after start_mining: $H_AFTER"
dump_recent_errors "$LOG1" "Daemon 1" 5
echo ""

STALL_COUNT=0
LAST_H=0
while true; do
    H=$(get_height 34568)
    printf "\r  Height: %s / %s  " "$H" "$TARGET_HEIGHT"

    # Detect stall
    if [[ "$H" == "$LAST_H" ]]; then
        STALL_COUNT=$((STALL_COUNT + 1))
    else
        STALL_COUNT=0
        LAST_H="$H"
    fi

    if [[ "$STALL_COUNT" -ge 10 ]]; then
        echo ""
        echo "  WARNING: Mining appears stalled at height $H for 10s"
        dump_recent_errors "$LOG1" "Daemon 1" 10
        dump_recent_errors "$LOG2" "Daemon 2" 5
        test_block_template 34568 "$BURN_ADDR" "Daemon 1"
        echo ""
        echo "  Checking if miner is still running..."
        local_resp=$(curl -s --max-time 5 http://127.0.0.1:34568/mining_status -H 'Content-Type: application/json' 2>&1)
        echo "  [mining_status] Daemon 1: $local_resp"
        local_resp=$(curl -s --max-time 5 http://127.0.0.1:34570/mining_status -H 'Content-Type: application/json' 2>&1)
        echo "  [mining_status] Daemon 2: $local_resp"
        STALL_COUNT=0
    fi

    if [[ "$H" -ge "$TARGET_HEIGHT" ]]; then
        echo ""
        break
    fi
    sleep 1
done

stop_mining 34568 "Daemon 1"
stop_mining 34570 "Daemon 2"
echo ""
echo "  Phase 2 complete — chain at height $(get_height 34568), coinbase rewards unlocked"
echo ""

# ── Final status ─────────────────────────────────────────────────────────────
echo "  Final error check..."
dump_recent_errors "$LOG1" "Daemon 1" 5
dump_recent_errors "$LOG2" "Daemon 2" 5
echo ""

# ══════════════════════════════════════════════════════════════════════════════
# STEP 5: Sync wallets against the freshly mined chain
# (wallets were already restored up front so we could mine to their addresses)
# ══════════════════════════════════════════════════════════════════════════════
echo "[5/5] Syncing wallets against the mined chain..."

echo "password" | "$BIN/electroneum-wallet-cli" --testnet \
    --wallet-file "$WALLETS_DIR/miner1" \
    --daemon-address 127.0.0.1:34568 --daemon-ssl disabled \
    --log-level 0 --command refresh > /dev/null 2>&1 \
    && echo "  miner1 synced" \
    || echo "  WARNING: miner1 sync failed"

echo "password" | "$BIN/electroneum-wallet-cli" --testnet \
    --wallet-file "$WALLETS_DIR/miner2" \
    --daemon-address 127.0.0.1:34568 --daemon-ssl disabled \
    --log-level 0 --command refresh > /dev/null 2>&1 \
    && echo "  miner2 synced" \
    || echo "  WARNING: miner2 sync failed"

echo ""

# ── Done ─────────────────────────────────────────────────────────────────────
echo "============================================================"
echo "  READY FOR MIGRATION TESTING"
echo "============================================================"
echo ""
echo "Both daemons are running. Miner wallets have unlocked coinbase."
echo ""
echo "Open a wallet with:"
echo ""
echo "  $BIN/electroneum-wallet-cli --testnet \\"
echo "    --wallet-file $WALLETS_DIR/miner1 --password password \\"
echo "    --daemon-address 127.0.0.1:34568 --daemon-ssl disabled"
echo ""
echo "  Then type:  refresh"
echo "              balance"
echo "              show_transfers"
echo ""
echo "To get keys for the GUI tool:"
echo ""
echo "  spendkey        (reveals secret spend key)"
echo "  viewkey         (reveals secret view key)"
echo "  address         (shows wallet address)"
echo "  exit"
echo ""
echo "Run the GUI migration tool:"
echo ""
echo "  cd ~/electroneum/gui/migration-tool && npm run dev"
echo ""
echo "Stop daemons when done:"
echo ""
echo "  $0 stop"
echo ""
echo "Wallet files: $WALLETS_DIR/"
echo "  miner1 — seed: keyboard suddenly wise baby ..."
echo "  miner2 — seed: fences sifting fudge urchins ..."
echo ""
