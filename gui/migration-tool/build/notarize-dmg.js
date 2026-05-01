// Post-build hook: submit each Mac DMG to Apple's notary service and staple
// the returned ticket onto it. Runs after electron-builder finishes producing
// all artifacts, so both the .app inside and the DMG container get notarised.
//
// Auth: uses keychain profile "test-profile" stored via:
//   xcrun notarytool store-credentials "test-profile" \
//     --apple-id ... --team-id ... --password ...
//
// This replaces the earlier afterSign hook that only notarised the .app —
// stapling the DMG too means users on offline networks get a fast launch
// and Gatekeeper does not have to query Apple at first-mount time.

const { execFileSync } = require('child_process');
const path = require('path');

const KEYCHAIN_PROFILE = 'test-profile';

function run(cmd, args, opts = {}) {
  console.log(`[notarize-dmg] $ ${cmd} ${args.map(a => /\s/.test(a) ? `"${a}"` : a).join(' ')}`);
  return execFileSync(cmd, args, { stdio: 'inherit', ...opts });
}

exports.default = async function (context) {
  const { artifactPaths, platformToTargets } = context;

  // Only act when we just built for darwin
  const builtForMac = platformToTargets &&
    Array.from(platformToTargets.keys()).some(p => p.name === 'mac');
  if (!builtForMac) {
    console.log('[notarize-dmg] Skipping — not a mac build');
    return;
  }

  const dmgs = (artifactPaths || []).filter(p => p.endsWith('.dmg'));
  if (dmgs.length === 0) {
    console.log('[notarize-dmg] No DMG artifacts produced; nothing to do');
    return;
  }

  for (const dmg of dmgs) {
    console.log(`[notarize-dmg] Submitting ${path.basename(dmg)} to Apple…`);
    run('xcrun', [
      'notarytool', 'submit', dmg,
      '--keychain-profile', KEYCHAIN_PROFILE,
      '--wait',
    ]);

    console.log(`[notarize-dmg] Stapling ticket onto ${path.basename(dmg)}…`);
    run('xcrun', ['stapler', 'staple', dmg]);

    console.log(`[notarize-dmg] Validating staple on ${path.basename(dmg)}…`);
    run('xcrun', ['stapler', 'validate', dmg]);

    console.log(`[notarize-dmg] ✓ ${path.basename(dmg)} notarised + stapled`);
  }
};
