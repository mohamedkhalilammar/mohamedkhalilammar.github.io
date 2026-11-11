import type { Metadata } from "next";
import { POST_GROUPS, getPostCards, groupCounts, groupSlug, postGroup } from "@/lib/posts";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";
import { SectionHead } from "@/components/site/section-head";
import { BlogIndex, type BlogRow } from "@/components/site/blog-index";
import { postGlyph } from "@/components/site/post-glyph";

export const metadata: Metadata = {
  title: "Blog — Khalil Ammar",
  description: "CTF writeups, the challenges I built, engineering notes and the odd opinion, by Khalil Ammar.",
};

export default function BlogPage() {
  const posts = getPostCards();
  const counts = groupCounts(posts);
  const rows: BlogRow[] = posts.map((p) => {
    const group = postGroup(p);
    return {
      id: p.id, title: p.title, summary: p.summary, date: p.date, undated: !!p.dateIsPlaceholder, minutes: p.readingMinutes,
      group, groupSlug: groupSlug(group), tags: p.tags, glyph: postGlyph(group, p.tags),
    };
  });

  return (
    <>
      <SiteBar active="writing" symbol="blog" />
      <main className="wrap sub" id="main-content">
        <section className="region sub-head" aria-labelledby="blog-h">
          <SectionHead as="h1" region="writing" name="blog" id="blog-h" title="Blog"
            lede={<>Writeups for challenges I solved, the {counts["Challenges I built"]} I built for Friendly CTF, engineering notes and the odd opinion. {posts.length} posts so far.</>} />
          <div className="rv"><div data-r>
            <BlogIndex rows={rows} groups={POST_GROUPS.map((g) => ({ name: g, slug: groupSlug(g), count: counts[g] }))} />
          </div></div>
        </section>
      </main>
      <SiteFooter />
    </>
  );
}
