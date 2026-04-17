import PDFDocument from 'pdfkit';
import QRCode from 'qrcode';
import crypto from 'crypto';
import fs from 'fs';

const ETN_BLUE = '#1a73e8';
const EXPLORER_BASE = 'https://blockexplorer.electroneum.com/address/';

/**
 * Generate a password-protected PDF paper wallet.
 * @param {Object} opts
 * @param {string} opts.address     - EVM address (0x...)
 * @param {string} opts.privateKey  - Private key hex (0x...)
 * @param {string} opts.password    - Password to encrypt the PDF
 * @param {string} opts.logoPath    - Absolute path to the Electroneum logo PNG
 * @returns {Promise<Buffer>}       - The PDF as a Buffer
 */
async function generatePaperWallet({ address, privateKey, password, logoPath }) {
  // Generate QR code PNGs
  const explorerUrl = `${EXPLORER_BASE}${address}`;
  const [addressQR, privKeyQR] = await Promise.all([
    QRCode.toBuffer(explorerUrl, { type: 'png', width: 160, margin: 1 }),
    QRCode.toBuffer(privateKey.replace(/^0x/, ''), { type: 'png', width: 160, margin: 1 }),
  ]);

  const doc = new PDFDocument({
    size: 'A4',
    margin: 50,
    userPassword: password,
    ownerPassword: crypto.randomBytes(32).toString('hex'),
    permissions: {
      printing: 'highResolution',
      modifying: false,
      copying: true,
      annotating: false,
      fillingForms: false,
      contentAccessibility: true,
      documentAssembly: false,
    },
  });

  // Collect the PDF into a buffer
  const chunks = [];
  doc.on('data', (chunk) => chunks.push(chunk));
  const finished = new Promise((resolve, reject) => {
    doc.on('end', () => resolve(Buffer.concat(chunks)));
    doc.on('error', reject);
  });

  const pageWidth = doc.page.width - doc.page.margins.left - doc.page.margins.right;
  const leftMargin = doc.page.margins.left;

  // ── Header ──────────────────────────────────────────────────────────────
  if (logoPath && fs.existsSync(logoPath)) {
    doc.image(logoPath, leftMargin, 40, { height: 40 });
  }

  doc
    .font('Helvetica-Bold')
    .fontSize(18)
    .fillColor(ETN_BLUE)
    .text('ETN SMART CHAIN PAPER WALLET', leftMargin, 50, {
      align: 'center',
      width: pageWidth,
    });

  const dateStr = new Date().toISOString().split('T')[0];
  doc
    .font('Helvetica')
    .fontSize(10)
    .fillColor('#666666')
    .text(`Generated: ${dateStr}`, leftMargin, 75, {
      align: 'center',
      width: pageWidth,
    });

  // ── Address Section ─────────────────────────────────────────────────────
  const addrBoxY = 110;
  drawSectionBox(doc, leftMargin, addrBoxY, pageWidth, 170);

  doc
    .font('Helvetica-Bold')
    .fontSize(12)
    .fillColor(ETN_BLUE)
    .text('YOUR SMART CHAIN ADDRESS', leftMargin + 15, addrBoxY + 12);

  doc.image(addressQR, leftMargin + 15, addrBoxY + 35, { width: 110, height: 110 });

  doc
    .font('Courier')
    .fontSize(8.5)
    .fillColor('#333333')
    .text(address, leftMargin + 140, addrBoxY + 40, {
      width: pageWidth - 160,
    });

  doc
    .font('Helvetica')
    .fontSize(8)
    .fillColor('#888888')
    .text(explorerUrl, leftMargin + 140, addrBoxY + 60, {
      width: pageWidth - 160,
    });

  doc
    .font('Helvetica')
    .fontSize(9)
    .fillColor('#666666')
    .text('Scan QR to view on block explorer', leftMargin + 15, addrBoxY + 150);

  // ── Private Key Section ─────────────────────────────────────────────────
  const pkBoxY = addrBoxY + 185;
  drawSectionBox(doc, leftMargin, pkBoxY, pageWidth, 170);

  doc
    .font('Helvetica-Bold')
    .fontSize(12)
    .fillColor(ETN_BLUE)
    .text('YOUR PRIVATE KEY', leftMargin + 15, pkBoxY + 12);

  doc.image(privKeyQR, leftMargin + 15, pkBoxY + 35, { width: 110, height: 110 });

  const pkDisplay = privateKey.replace(/^0x/, '');
  doc
    .font('Courier')
    .fontSize(7.5)
    .fillColor('#333333')
    .text(pkDisplay, leftMargin + 140, pkBoxY + 40, {
      width: pageWidth - 155,
    });

  doc
    .font('Helvetica')
    .fontSize(9)
    .fillColor('#666666')
    .text('Scan QR to import via "Import Account" in MetaMask (or other compatible wallet)', leftMargin + 15, pkBoxY + 150);

  // ── Security Notice ─────────────────────────────────────────────────────
  const secBoxY = pkBoxY + 185;
  drawSectionBox(doc, leftMargin, secBoxY, pageWidth, 130, '#fff8e1');

  doc
    .font('Helvetica-Bold')
    .fontSize(11)
    .fillColor('#b8860b')
    .text('SECURITY NOTICE', leftMargin + 15, secBoxY + 12);

  const notices = [
    'Store in a safe, dry location',
    'Never share this document or its password',
    'This PDF is encrypted \u2014 keep the password you used to generate it',
    'To import into MetaMask (or other compatible wallet): scan the private key QR above, or enter the key manually',
  ];

  doc
    .font('Helvetica')
    .fontSize(9)
    .fillColor('#555555');

  let noticeY = secBoxY + 32;
  for (const notice of notices) {
    doc.text(`\u2022  ${notice}`, leftMargin + 15, noticeY, {
      width: pageWidth - 30,
    });
    noticeY += 20;
  }

  // ── Footer ──────────────────────────────────────────────────────────────
  doc
    .font('Helvetica')
    .fontSize(9)
    .fillColor('#999999')
    .text(
      'Electroneum Smart Chain  \u00b7  electroneum.com',
      leftMargin,
      doc.page.height - doc.page.margins.bottom - 20,
      { align: 'center', width: pageWidth }
    );

  doc.end();
  return finished;
}

function drawSectionBox(doc, x, y, width, height, bgColor) {
  if (bgColor) {
    doc.save()
      .roundedRect(x, y, width, height, 6)
      .fill(bgColor)
      .restore();
  }

  doc.save()
    .roundedRect(x, y, width, height, 6)
    .lineWidth(1)
    .strokeColor('#e0e0e0')
    .stroke()
    .restore();
}

export { generatePaperWallet };
