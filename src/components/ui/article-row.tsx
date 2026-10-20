import Link from "next/link";
import type { PostMeta } from "@/lib/posts";

export function ArticleRow({ post, showTags = false }: { post: PostMeta; showTags?: boolean }) {
  const date = post.dateIsPlaceholder ? null : new Date(post.date + "T00:00:00Z").toLocaleDateString("en-GB", { day: "numeric", month: "short", year: "numeric", timeZone: "UTC" });
  return (
    <article className="reading-row">
      <Link href={`/writeups/${post.id}`} className="reading-row-link">
        <div className="reading-row-meta" data-undated={!date}>
          <span>{showTags ? post.category : post.tags[0] ?? post.category}</span>
          {date && <time dateTime={post.date}>{date}</time>}
        </div>
        <div className="reading-row-copy">
          <h3>{post.title}</h3>
          <p>{post.summary}</p>
          {showTags && post.tags.length > 0 && <ul className="reading-row-tags" aria-label="Article tags">{post.tags.map(tag => <li key={tag}>{tag}</li>)}</ul>}
        </div>
        <span className="reading-row-action"><span>{post.readingMinutes} min read</span><span aria-hidden>↗</span></span>
      </Link>
    </article>
  );
}
