import sharp from "sharp";
import { mkdir, mkdtemp, rm } from "node:fs/promises";
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import { tmpdir } from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

// Generate responsive delivery assets without changing the original photographs.
const root = fileURLToPath(new URL("../public/media/", import.meta.url));
const output = path.join(root, "optimized");
await mkdir(output, { recursive: true });
const photos = ["cybersphere.jpeg", "finals.jpg", "photo.jpg", "security-talk-960.webp", "team.jpeg", "winners.jpeg", "dup.jpeg", "scoreboard.jpeg", "cybercampphoto.jpg", "teamm.jpeg"];
for (const photo of photos) {
  const name = path.parse(photo).name.replace(/-960$/, "");
  const widths = ["cybersphere.jpeg", "finals.jpg", "photo.jpg"].includes(photo) ? [160, 480, 960, 1600] : [160, 480, 960];
  for (const width of widths) {
    await sharp(path.join(root, photo)).rotate().resize({ width }).webp({ quality: 80 }).toFile(path.join(output, `${name}-${width}.webp`));
  }
}

// HEIC originals need the system libheif decoder (heif-convert).
// Conversion preserves framing and produces browser-compatible delivery files.
const convert = promisify(execFile);
const temporary = await mkdtemp(path.join(tmpdir(), "portfolio-workshops-"));
try {
  for (const [original, name] of [
    ["IMG_2538.HEIC", "workshop-opening"],
    ["IMG_2546.HEIC", "workshop-introduction"],
    ["IMG_2550.HEIC", "workshop-writing"],
  ]) {
    const decoded = path.join(temporary, name + ".png");
    await convert("heif-convert", [path.join(root, original), decoded]);
    for (const width of [480, 960, 1600]) {
      await sharp(decoded).rotate().resize({ width }).webp({ quality: 78 }).toFile(path.join(output, `${name}-${width}.webp`));
    }
  }
} finally {
  await rm(temporary, { recursive: true, force: true });
}
