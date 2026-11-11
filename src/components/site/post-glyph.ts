import type { PostGroup } from "@/lib/posts";

/** Pick a line glyph for a post from its group and tags. */
export function postGlyph(group: PostGroup, tags: string[]): string {
  if (group === "Bug bounty") return "lock";
  if (group === "Engineering") return "gear";
  if (group === "Opinions") return "pen";
  const t = tags.join(" ").toLowerCase();
  if (t.includes("mobile")) return "phone";
  if (t.includes("web")) return "bag";
  if (t.includes("reverse")) return "chip";
  if (t.includes("forensic")) return "bug";
  return "flag";
}
