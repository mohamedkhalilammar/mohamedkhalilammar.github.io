import { featuredWriting, projects } from "@/data/portfolio";
import { POST_GROUPS, getPostCards, groupCounts } from "@/lib/posts";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";
import { Hero } from "@/components/site/hero";
import { PathGraph } from "@/components/site/path-graph";
import { RecordList } from "@/components/site/record-list";
import { SectionHead } from "@/components/site/section-head";
import { WorkList } from "@/components/site/work-list";
import { WritingDeck, type DeckCard } from "@/components/site/writing-deck";
import { DeckCover } from "@/components/site/deck-cover";
import { Glyph } from "@/components/site/glyph";
import { Contact } from "@/components/site/contact";

const NUMBER_WORDS: Record<number, string> = { 5: "Five", 6: "Six", 7: "Seven", 8: "Eight", 9: "Nine", 10: "Ten" };

export default function Home() {
  const posts = getPostCards();
  const counts = groupCounts(posts);
  const work = [...projects].sort((a, b) => a.order - b.order);
  const cards: DeckCard[] = featuredWriting.flatMap((f) => {
    const post = posts.find((p) => p.id === f.id);
    if (!post) return [];
    return [{
      id: f.id, href: "/writeups/" + f.id, cover: <DeckCover cover={f.cover} />, glyph: <Glyph name={f.glyph} />,
      kicker: f.kicker, title: f.title ?? post.title, blurb: f.blurb ?? post.summary, date: post.date,
    }];
  });

  return (
    <>
      <SiteBar isHome />
      <main className="wrap" id="main-content">
        <Hero />
        <PathGraph />
        <RecordList />
        <section className="region" id="work" aria-labelledby="work-h">
          <SectionHead region="work" id="work-h" title="Work" lede={`${NUMBER_WORDS[work.length] ?? work.length} projects. Pick one to open it.`} />
          <div className="rv"><WorkList projects={work} /></div>
        </section>
        <section className="region" id="writing" aria-labelledby="writing-h">
          <SectionHead region="writing" id="writing-h" title="Writing" lede="Writeups for challenges I solved and challenges I built, plus the odd reflection." />
          <div className="rv">
            <WritingDeck cards={cards} total={posts.length} groups={POST_GROUPS.map((g) => ({ name: g, count: counts[g] }))} />
          </div>
          <p className="note">Bug bounty disclosures will live here too. None are published yet, and when they are, the affected company stays <span className="redact" aria-label="redacted">XXXXXXXXXX</span>.</p>
        </section>
        <Contact />
      </main>
      <SiteFooter />
    </>
  );
}
