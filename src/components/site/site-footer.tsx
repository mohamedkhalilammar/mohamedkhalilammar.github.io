import Link from "next/link";
import { profile } from "@/data/portfolio";

/** The process exits cleanly at the bottom of every page. */
export function SiteFooter({ exit = "[Inferior 1 (process 4021) exited normally]" }: { exit?: string }) {
  return (
    <footer className="foot wrap">
      <span className="mono dim">{exit}</span>
      <nav className="foot-nav" aria-label="Footer">
        <Link href="/blog">Blog</Link>
        <a href="/arcade">Arcade</a>
        <a href={profile.socials.github} target="_blank" rel="noopener noreferrer">GitHub</a>
        <a href={profile.socials.linkedin} target="_blank" rel="noopener noreferrer">LinkedIn</a>
      </nav>
      <span className="mono dim">Khalil Ammar · Tunis</span>
    </footer>
  );
}
