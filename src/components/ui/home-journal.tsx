import Link from "next/link";
import type { PostMeta } from "@/lib/posts";

function PostDetails({ post }: { post: PostMeta }) {
  return (
    <div className="home-journal-meta">
      <span>{post.tags[0] ?? post.category}</span>
      <span>{post.readingMinutes} min read</span>
    </div>
  );
}

export function HomeJournal({ posts }: { posts: PostMeta[] }) {
  const [featured, ...remaining] = posts;
  if (!featured) return <p className="section-description">New articles are on the way.</p>;

  return (
    <div className="home-journal">
      <div className="home-journal-intro">
        <p>What I build, break, and learn along the way.<br />CTF write-ups, reverse engineering, and project notes.</p>
        <Link href="/blog" className="home-journal-archive">All articles <span>{posts.length}</span><span aria-hidden>↗</span></Link>
      </div>
      <div className="home-journal-grid">
        <article className="home-journal-feature">
          <Link href={`/writeups/${featured.id}`}>
            <div className="home-journal-feature-top"><span className="home-journal-kicker">Latest article</span><span aria-hidden>↗</span></div>
            <PostDetails post={featured} />
            <h3>{featured.title}</h3>
            <p>{featured.summary}</p>
            <div className="home-journal-feature-bottom">
              <span>Read article <span aria-hidden>→</span></span>
              {!featured.dateIsPlaceholder && <time dateTime={featured.date}>{new Date(`${featured.date}T00:00:00Z`).toLocaleDateString("en-GB", { day: "numeric", month: "short", year: "numeric", timeZone: "UTC" })}</time>}
            </div>
          </Link>
        </article>
        {remaining.length > 0 && <div className="home-journal-more">
          <p className="home-journal-kicker">More articles</p>
          {remaining.slice(0, 3).map((post, index) => (
            <article key={post.id}>
              <Link href={`/writeups/${post.id}`}>
                <span className="home-journal-number" aria-hidden>0{index + 2}</span>
                <div><PostDetails post={post} /><h3>{post.title}</h3><p>{post.summary}</p></div>
                <span className="home-journal-next" aria-hidden>↗</span>
              </Link>
            </article>
          ))}
        </div>}
      </div>
    </div>
  );
}
