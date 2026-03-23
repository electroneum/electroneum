const { execSync } = require('child_process');

const platform = process.platform;
const arch = process.arch;

let platformDir;
if (platform === 'darwin') {
  platformDir = arch === 'arm64' ? 'mac-arm64' : 'mac-x64';
} else if (platform === 'linux') {
  platformDir = 'linux-x64';
} else if (platform === 'win32') {
  platformDir = 'win32-x64';
}

/**
 * @type {import('electron-builder').Configuration}
 */
module.exports = {
  appId: 'com.electroneum.migration-tool',
  productName: 'ETN Migration Tool',
  copyright: 'Copyright © 2024 Electroneum Limited',

  directories: {
    buildResources: 'resources',
    output: 'dist-electron',
  },

  files: [
    'out/**/*',
    '!node_modules',
  ],

  extraResources: [
    {
      from: `bin/${platformDir}/`,
      to: 'bin/',
      filter: ['**/*'],
    },
  ],

  mac: {
    target: [
      { target: 'dmg', arch: ['arm64', 'x64'] },
    ],
    category: 'public.app-category.finance',
    icon: 'resources/icon.icns',
  },

  linux: {
    target: ['AppImage', 'deb'],
    category: 'Finance',
    icon: 'resources/icon.png',
  },

  win: {
    target: [
      { target: 'nsis', arch: ['x64'] },
    ],
    icon: 'resources/icon.ico',
  },

  nsis: {
    oneClick: false,
    allowToChangeInstallationDirectory: true,
    installerIcon: 'build/icon.ico',
    uninstallerIcon: 'build/icon.ico',
    installerIcon: 'resources/icon.ico',
    uninstallerIcon: 'resources/icon.ico',
  },

  dmg: {
    title: 'ETN Migration Tool',
    contents: [
      { x: 410, y: 150, type: 'link', path: '/Applications' },
      { x: 130, y: 150, type: 'file' },
    ],
  },
};
