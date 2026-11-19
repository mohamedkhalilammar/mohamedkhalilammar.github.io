// Usage: node scripts/screenshot.mjs [path=/] [width=1440] [height=900]
// Needs `npm run dev` running on :3001. Saves to .shots/ (gitignored).
import { chromium } from "playwright";
import { mkdirSync } from "node:fs";

const [path = "/", w = "1440", h = "900"] = process.argv.slice(2);
mkdirSync(".shots", { recursive: true });
const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: +w, height: +h } });
await page.goto(`http://localhost:3001${path}`, { waitUntil: "networkidle" });
const name = `.shots/${path.replace(/\W+/g, "_") || "home"}-${w}.png`;
await page.screenshot({ path: name, fullPage: true });
console.log("saved", name);
await browser.close();
