// Mirrors the frontmatter rules in src/lib/posts.ts so a bad post is caught
// without paying for a full Next build.
import fs from "node:fs";
import path from "node:path";
import matter from "gray-matter";

const ROOT = path.resolve(import.meta.dirname, "..");
const DIR = path.join(ROOT, "content", "posts");
const MEDIA = path.join(ROOT, "public");
const SLUG = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;
const ISO = /^\d{4}-\d{2}-\d{2}$/;

const problems = [];
const warnings = [];
const seen = new Map();
const files = fs
  .readdirSync(DIR)
  .filter((f) => f.endsWith(".md") && !f.startsWith("_") && f !== "README.md");

for (const file of files) {
  const slug = file.replace(/\.md$/, "");
  const fail = (m) => problems.push(`${file}: ${m}`);
  const raw = fs.readFileSync(path.join(DIR, file), "utf8");

  let data, content;
  try {
    ({ data, content } = matter(raw));
  } catch (e) {
    fail(`YAML parse error — ${e.message}`);
    continue;
  }

  if (!SLUG.test(slug)) fail(`filename is not lowercase kebab-case`);
  for (const f of ["id", "title", "date"]) {
    if (data[f] === undefined || String(data[f]).trim() === "") fail(`missing "${f}"`);
  }
  if (data.id !== slug) fail(`id "${data.id}" != slug "${slug}"`);

  const date =
    data.date instanceof Date
      ? data.date.toISOString().slice(0, 10)
      : String(data.date ?? "").trim();
  if (!ISO.test(date)) fail(`date "${data.date}" is not YYYY-MM-DD`);

  if (data.tags !== undefined && !Array.isArray(data.tags)) fail(`tags is not a YAML list`);

  // Match the build parser: it derives a summary from the first paragraph.
  const summaryBody = content.split(/^##\s+Summary\s*$/im)[1] ?? content;
  const derivedSummary = summaryBody
    .split(/\n\s*\n/)
    .map((block) => block.trim())
    .find((block) => block.length > 0 && !block.startsWith("#") && block !== "---");
  if (!String(data.summary ?? "").trim() && !derivedSummary) fail(`no summary and no usable paragraph`);

  if (data.severity) fail(`has severity — would be treated as a bug-bounty post`);
  if (data.draft === true) warnings.push(`${file}: draft retained for future publication`);

  if (seen.has(data.id)) fail(`duplicate id, also in ${seen.get(data.id)}`);
  seen.set(data.id, file);

  // Referenced local images must exist, or the article renders a broken image.
  for (const [, src] of content.matchAll(/!\[[^\]]*\]\((\/[^)\s]+)\)/g)) {
    if (!fs.existsSync(path.join(MEDIA, src))) fail(`missing image ${src}`);
  }

  const words = content.trim().split(/\s+/).filter(Boolean).length;
  if (words < 250) warnings.push(`${file}: only ${words} words — worth reviewing`);
}

console.log(`checked ${files.length} posts`);
if (warnings.length) {
  console.log(`\n${warnings.length} warning(s):`);
  for (const warning of warnings) console.log("  - " + warning);
}
if (problems.length) {
  console.log(`\n${problems.length} problem(s):`);
  for (const p of problems) console.log("  - " + p);
  process.exit(1);
}
console.log("all good");
