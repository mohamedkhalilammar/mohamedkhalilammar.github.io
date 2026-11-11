import type { DeckCover as Cover } from "@/data/portfolio";

/** Cover art for a writing card: a capture, a phone capture in a lit frame, or type pulled from the post. */
export function DeckCover({ cover }: { cover: Cover }) {
  switch (cover.kind) {
    case "shot":
      // eslint-disable-next-line @next/next/no-img-element -- static export
      return <figure className="dk-cover dk-shot"><img src={cover.src} alt={cover.alt} width={cover.width} height={cover.height} loading="lazy" draggable={false} /></figure>;
    case "phone":
      // eslint-disable-next-line @next/next/no-img-element -- static export
      return <figure className="dk-cover dk-phone"><img src={cover.src} alt="" width={cover.width} height={cover.height} loading="lazy" draggable={false} /></figure>;
    case "big":
      return <figure className="dk-cover dk-type dk-big" aria-hidden="true"><b>{cover.text}</b><span>{cover.caption}</span></figure>;
    case "term":
      // authored markup from portfolio.ts, never user input
      return <figure className="dk-cover dk-type dk-term" aria-hidden="true"><code dangerouslySetInnerHTML={{ __html: cover.html }} /></figure>;
  }
}
