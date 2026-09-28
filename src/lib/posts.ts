import fs from "node:fs";
import path from "node:path";
import matter from "gray-matter";

/**
 * Unified post layer for the blog.
 *
 * This generalises the bug-bounty-only pipeline that used to live in
 * `src/lib/bugbounty.ts`: same gray-matter parsing, same build-failing
 * validation, same reading-time and summary derivation. What changes is that a
 * post is now any markdown file in `content/posts/`, tagged with a `category`.
 *
 * The one behavioural difference is deliberate and load-bearing: the
 * "don't publish unpatched findings" gate applies **only to bug-bounty posts**.
 * A CTF writeup has no vendor status to be resolved, so gating it on one would
 * drop every writeup on the site.
 */

const CONTENT_DIR = path.join(process.cwd(), "content", "posts");

export const BUG_BOUNTY_CATEGORY = "Bug Bounty";

export type PostSeverity = "Critical" | "High" | "Medium" | "Low" | "Informational";

export type PostMeta = {
  id: string;
  title: string;
  category: string;
  date: string;
  /** Migrated writeups carry a synthesised date until real ones are filled in. */
  dateIsPlaceholder?: boolean;
  summary: string;
  tags: string[];
  cover?: string;
  flag?: string;
  githubUrl?: string;
  draft: boolean;
  readingMinutes: number;
  words: number;
  /** Bug-bounty-only block. Absent on every other category. */
  severity?: PostSeverity;
  cvss?: number;
  class?: string;
  sector?: string;
  scale?: string;
  platform?: string;
  status?: string;
  bounty?: string;
};

export type Post = PostMeta & { content: string };

const SEVERITIES: readonly PostSeverity[] = [
  "Critical",
  "High",
  "Medium",
  "Low",
  "Informational",
];

/**
 * Statuses that mean the finding is resolved on the vendor's side and is safe
 * to publish. Anything outside this list is treated as a draft so an unpatched
 * issue can never reach the static export.
 */
const PUBLISHABLE_STATUSES = [
  "fixed",
  "fixed & paid",
  "fixed and paid",
  "resolved",
  "resolved & paid",
  "mitigated",
  "disclosed",
  "patched",
] as const;

const SLUG_PATTERN = /^[a-z0-9]+(?:-[a-z0-9]+)*$/;
const ISO_DATE_PATTERN = /^\d{4}-\d{2}-\d{2}$/;
const WORDS_PER_MINUTE = 210;

/** Thrown when a post's frontmatter is invalid — fails the build loudly. */
class PostContentError extends Error {
  constructor(file: string, problem: string) {
    super(
      `[posts] content/posts/${file}: ${problem}\n` +
        `        See content/posts/README.md for the required frontmatter.`
    );
    this.name = "PostContentError";
  }
}

export function isPublishableStatus(status: string): boolean {
  return PUBLISHABLE_STATUSES.includes(
    status.trim().toLowerCase() as (typeof PUBLISHABLE_STATUSES)[number]
  );
}

/** First real paragraph after the `## Summary` heading, else the first paragraph. */
function deriveSummary(body: string): string {
  const afterSummaryHeading = body.split(/^##\s+Summary\s*$/im)[1] ?? body;
  const paragraph = afterSummaryHeading
    .split(/\n\s*\n/)
    .map((block) => block.trim())
    .find((block) => block.length > 0 && !block.startsWith("#") && block !== "---");

  if (!paragraph) return "";

  return paragraph
    .replace(/\s+/g, " ")
    .replace(/[*_`]/g, "")
    .replace(/\[([^\]]+)\]\([^)]*\)/g, "$1")
    .trim();
}

function countWords(body: string): number {
  return body.trim().split(/\s+/).filter(Boolean).length;
}

function parsePost(file: string, raw: string): Post {
  const { data, content } = matter(raw);
  const slug = file.replace(/\.md$/, "");

  if (!SLUG_PATTERN.test(slug)) {
    throw new PostContentError(
      file,
      `filename must be a lowercase kebab-case slug (got "${slug}")`
    );
  }

  for (const field of ["id", "title", "date"] as const) {
    const value = data[field];
    if (value === undefined || value === null || String(value).trim() === "") {
      throw new PostContentError(file, `missing required frontmatter field "${field}"`);
    }
  }

  if (data.id !== slug) {
    throw new PostContentError(
      file,
      `frontmatter id "${data.id}" must match the filename slug "${slug}"`
    );
  }

  // A post carrying a severity is a finding, whether or not it says so.
  const category = String(
    data.category ?? (data.severity ? BUG_BOUNTY_CATEGORY : "Notes")
  ).trim();
  const isFinding = category === BUG_BOUNTY_CATEGORY;

  if (isFinding) {
    for (const field of ["severity", "class", "sector", "platform", "status"] as const) {
      if (data[field] === undefined || String(data[field]).trim() === "") {
        throw new PostContentError(
          file,
          `bug-bounty posts require frontmatter field "${field}"`
        );
      }
    }
    if (!SEVERITIES.includes(data.severity)) {
      throw new PostContentError(
        file,
        `severity "${data.severity}" is not one of ${SEVERITIES.join(", ")}`
      );
    }
    if (data.cvss !== undefined) {
      const cvss = Number(data.cvss);
      if (!Number.isFinite(cvss) || cvss < 0 || cvss > 10) {
        throw new PostContentError(
          file,
          `cvss must be a number between 0 and 10 (got "${data.cvss}")`
        );
      }
    }
  }

  // gray-matter turns unquoted YAML dates into Date objects — normalise both forms.
  const date =
    data.date instanceof Date ? data.date.toISOString().slice(0, 10) : String(data.date).trim();
  if (!ISO_DATE_PATTERN.test(date)) {
    throw new PostContentError(file, `date must be ISO YYYY-MM-DD (got "${data.date}")`);
  }

  if (data.tags !== undefined && !Array.isArray(data.tags)) {
    throw new PostContentError(file, `tags must be a YAML list (got "${data.tags}")`);
  }

  const summary = String(data.summary ?? "").trim() || deriveSummary(content);
  if (!summary) {
    throw new PostContentError(
      file,
      "no summary — add a `summary:` frontmatter field or a `## Summary` section"
    );
  }

  const words = countWords(content);

  return {
    id: slug,
    title: String(data.title).trim(),
    category,
    date,
    dateIsPlaceholder: data.dateIsPlaceholder === true,
    summary,
    tags: (data.tags ?? []).map((tag: unknown) => String(tag).trim()).filter(Boolean),
    cover: data.cover ? String(data.cover).trim() : undefined,
    flag: data.flag ? String(data.flag).trim() : undefined,
    githubUrl: data.githubUrl ? String(data.githubUrl).trim() : undefined,
    draft: data.draft === true,
    words,
    readingMinutes: Math.max(1, Math.round(words / WORDS_PER_MINUTE)),
    severity: isFinding ? (data.severity as PostSeverity) : undefined,
    cvss: isFinding && data.cvss !== undefined ? Number(data.cvss) : undefined,
    class: isFinding ? String(data.class).trim() : undefined,
    sector: isFinding ? String(data.sector).trim() : undefined,
    scale: isFinding && data.scale ? String(data.scale).trim() : undefined,
    platform: isFinding ? String(data.platform).trim() : undefined,
    status: isFinding ? String(data.status).trim() : undefined,
    bounty: data.bounty ? String(data.bounty).trim() : undefined,
    content,
  };
}

let cache: Post[] | null = null;

/**
 * All publishable posts, newest first. Drafts are excluded, and bug-bounty
 * posts whose status does not mean "resolved" are excluded on top of that.
 * Invalid frontmatter throws and fails the build.
 */
export function getAllPosts(): Post[] {
  if (cache) return cache;

  if (!fs.existsSync(CONTENT_DIR)) {
    cache = [];
    return cache;
  }

  // `README.md` documents the format and `_`-prefixed files are authoring
  // templates — neither is a post.
  const files = fs
    .readdirSync(CONTENT_DIR)
    .filter((file) => file.endsWith(".md") && !file.startsWith("_") && file !== "README.md");

  const posts = files
    .map((file) => parsePost(file, fs.readFileSync(path.join(CONTENT_DIR, file), "utf8")))
    .filter((post) => {
      if (post.draft) return false;
      // The gate is bug-bounty-only by design: other categories have no
      // vendor status, and applying it to them would drop every one of them.
      if (post.category === BUG_BOUNTY_CATEGORY && !isPublishableStatus(post.status ?? "")) {
        console.warn(
          `[posts] Skipping "${post.id}" — status "${post.status}" does not mean resolved. ` +
            `Unpatched findings are never published.`
        );
        return false;
      }
      return true;
    })
    .sort((a, b) => b.date.localeCompare(a.date));

  const seen = new Set<string>();
  for (const post of posts) {
    if (seen.has(post.id)) {
      throw new Error(`[posts] Duplicate post id "${post.id}".`);
    }
    seen.add(post.id);
  }

  cache = posts;
  return cache;
}

/** Metadata only, no markdown body — safe to hand to a client component. */
export function getPostCards(): PostMeta[] {
  return getAllPosts().map(({ content: _content, ...card }) => card);
}

export function getPost(id: string): Post | undefined {
  return getAllPosts().find((post) => post.id === id);
}

export type Facet = { name: string; slug: string; count: number };

/** URL-safe slug for a category or tag. */
export function toSlug(value: string): string {
  return value
    .toLowerCase()
    .trim()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "");
}

function tally(values: string[]): Facet[] {
  const counts = new Map<string, number>();
  for (const value of values) {
    counts.set(value, (counts.get(value) ?? 0) + 1);
  }
  return [...counts.entries()]
    .map(([name, count]) => ({ name, slug: toSlug(name), count }))
    .sort((a, b) => b.count - a.count || a.name.localeCompare(b.name));
}

/** Categories that actually have posts. Empty ones are never rendered. */
export function getCategories(): Facet[] {
  return tally(getAllPosts().map((post) => post.category));
}

/** Tags that actually have posts. Empty ones are never rendered. */
export function getTags(): Facet[] {
  return tally(getAllPosts().flatMap((post) => post.tags));
}

export function getPostsByCategory(slug: string): Post[] {
  return getAllPosts().filter((post) => toSlug(post.category) === slug);
}

export function getPostsByTag(slug: string): Post[] {
  return getAllPosts().filter((post) => post.tags.some((tag) => toSlug(tag) === slug));
}

export type ArchiveYear = { year: string; posts: PostMeta[] };

/** Posts grouped by year, newest year first. */
export function getArchiveByYear(): ArchiveYear[] {
  const byYear = new Map<string, PostMeta[]>();
  for (const { content: _content, ...card } of getAllPosts()) {
    const year = card.date.slice(0, 4);
    byYear.set(year, [...(byYear.get(year) ?? []), card]);
  }
  return [...byYear.entries()]
    .map(([year, posts]) => ({ year, posts }))
    .sort((a, b) => b.year.localeCompare(a.year));
}
