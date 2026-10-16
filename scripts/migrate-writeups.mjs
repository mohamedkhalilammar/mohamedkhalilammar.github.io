/**
 * One-off: lift the 7 CTF writeups out of src/data/portfolio.ts into
 * content/posts/*.md.
 *
 * Their `content` fields are HTML with hardcoded Tailwind classes baked in
 * (text-primary/80, border-white/5, text-green-400) — all of which assume the
 * old dark theme and break on a light ground. Converting to plain markdown
 * drops the classes permanently and lets MarkdownBody own the styling.
 *
 * Run once:  node scripts/migrate-writeups.mjs
 */
import fs from "node:fs";
import path from "node:path";

const SRC = path.join(process.cwd(), "src", "data", "portfolio.ts");
const OUT = path.join(process.cwd(), "content", "posts");

/**
 * Pull the ctfWriteups array literal out of the TS source by bracket matching.
 * Anchor on the `= [` opener, not the first `[` — the type annotation
 * (`: Writeup[]`) contains brackets of its own.
 */
function extractArray(source, name) {
  const re = new RegExp(`export const ${name}[^=]*=\\s*\\[`);
  const m = re.exec(source);
  if (!m) throw new Error(`${name} not found`);
  const open = m.index + m[0].length - 1; // index of the array's own "["
  let i = open + 1;
  let depth = 1;
  while (depth && i < source.length) {
    if (source[i] === "[") depth++;
    else if (source[i] === "]") depth--;
    i++;
  }
  return source.slice(open, i);
}

/** The tag set is small and fixed: p, code, em, strong, h4, hr, ul, li. */
function htmlToMarkdown(html) {
  let s = html;
  s = s.replace(/<hr[^>]*\/?>/g, "\n\n---\n\n");
  s = s.replace(/<h4[^>]*>(.*?)<\/h4>/gs, (_, t) => `\n\n## ${t.trim()}\n\n`);
  s = s.replace(/<ul[^>]*>(.*?)<\/ul>/gs, (_, inner) => {
    const items = [...inner.matchAll(/<li[^>]*>(.*?)<\/li>/gs)].map(
      (m) => `- ${m[1].trim()}`
    );
    return `\n\n${items.join("\n")}\n\n`;
  });
  s = s.replace(/<p[^>]*>(.*?)<\/p>/gs, (_, t) => `\n\n${t.trim()}\n\n`);
  s = s.replace(/<code[^>]*>(.*?)<\/code>/gs, (_, t) => `\`${t.trim()}\``);
  s = s.replace(/<strong[^>]*>(.*?)<\/strong>/gs, (_, t) => `**${t.trim()}**`);
  s = s.replace(/<em[^>]*>(.*?)<\/em>/gs, (_, t) => `*${t.trim()}*`);
  s = s.replace(/<br\s*\/?>/g, "\n");
  s = s.replace(/<[^>]+>/g, ""); // anything unexpected
  // entities
  s = s
    .replace(/&nbsp;/g, " ")
    .replace(/&amp;/g, "&")
    .replace(/&lt;/g, "<")
    .replace(/&gt;/g, ">")
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'");
  return s.replace(/\n{3,}/g, "\n\n").trim() + "\n";
}

/** YAML-safe scalar. */
const q = (v) => `"${String(v).replace(/\\/g, "\\\\").replace(/"/g, '\\"')}"`;

const source = fs.readFileSync(SRC, "utf8");
const literal = extractArray(source, "ctfWriteups");

// The literal is valid JS once the TS type annotations are gone.
const writeups = new Function(`return ${literal};`)();

// CTF writeups carry no publication date in the data. Order is
// newest-first in the source, so synthesise a descending date series and
// leave a marker so real dates can be filled in later.
const BASE_YEAR = 2026;
let month = 7;

fs.mkdirSync(OUT, { recursive: true });

for (const w of writeups) {
  const date = `${BASE_YEAR}-${String(month).padStart(2, "0")}-01`;
  month = month > 1 ? month - 1 : 12;

  const tags = [...new Set([w.category, w.competition, ...(w.tools ?? [])])]
    .filter(Boolean)
    .map((t) => `  - ${q(t)}`)
    .join("\n");

  const fm = [
    "---",
    `id: ${q(w.id)}`,
    `title: ${q(w.title)}`,
    `category: "CTF Writeups"`,
    `date: ${q(date)}`,
    `dateIsPlaceholder: true`,
    `summary: ${q(w.summary)}`,
    w.mediaUrl ? `cover: ${q(w.mediaUrl)}` : null,
    w.flag ? `flag: ${q(w.flag)}` : null,
    w.githubUrl ? `githubUrl: ${q(w.githubUrl)}` : null,
    tags ? `tags:\n${tags}` : null,
    "---",
    "",
    "",
  ]
    .filter((line) => line !== null)
    .join("\n");

  const file = path.join(OUT, `${w.id}.md`);
  fs.writeFileSync(file, fm + htmlToMarkdown(w.content ?? ""));
  console.log("wrote", path.relative(process.cwd(), file));
}
console.log(`\n${writeups.length} writeups migrated.`);
