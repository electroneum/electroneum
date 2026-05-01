// Custom notarize hook used by electron-builder via mac.afterSign.
// Uses the keychain profile "test-profile" stored via:
//   xcrun notarytool store-credentials "test-profile" --apple-id ... --team-id ... --password ...
// This avoids electron-builder's env-var pickup (which has been flaky in this
// project) and uses Apple's keychain directly.
const { notarize } = require('@electron/notarize');
const path = require('path');

exports.default = async function notarizing(context) {
  const { electronPlatformName, appOutDir } = context;
  if (electronPlatformName !== 'darwin') return;

  const appName = context.packager.appInfo.productFilename;
  const appPath = path.join(appOutDir, `${appName}.app`);

  console.log(`[notarize] Submitting ${appPath} via keychain-profile "test-profile"`);
  await notarize({
    tool: 'notarytool',
    appPath,
    keychainProfile: 'test-profile',
  });
  console.log(`[notarize] Notarisation complete for ${appPath}`);
};
