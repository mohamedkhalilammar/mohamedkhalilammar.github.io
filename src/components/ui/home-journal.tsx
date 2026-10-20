import Link from "next/link";
import type { PostMeta } from "@/lib/posts";
import { WorkshopBackground } from "@/components/ui/workshop-background";

export function HomeJournal({ posts }: { posts: PostMeta[] }) {
  return (
    <div className="home-writing">
      <header className="writing-banner">
        <WorkshopBackground photo="opening" alt="Khalil presenting a workshop on AI in cybersecurity." />
        <div className="writing-banner-copy">
          <h2>The Blog</h2>
          <p>What I build, break, and learn along the way.<br />CTF write-ups, reverse engineering, and project notes.</p>
          <Link href="/blog" className="writing-archive-link">Explore the blog <span>{posts.length} articles</span><span aria-hidden>↗</span></Link>
        </div>
      </header>
    </div>
  );
}
