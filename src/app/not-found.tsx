import Link from "next/link";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";

export default function NotFound() {
  return (
    <>
      <SiteBar active="entry" symbol="??" />
      <main className="wrap sub nf" id="main-content">
        <p className="maps mono"><span className="ad">0x00000000</span><span className="gap" /><span className="kw">---p</span><span className="gap" /><span className="nm">unmapped</span></p>
        <h1 className="h2">404</h1>
        <p className="nf-sig mono">Program received signal SIGSEGV, Segmentation fault.<br /><span className="dim">0x00000000 in ?? ()</span></p>
        <p className="lede">This page isn&rsquo;t mapped. It may have moved when the site was rebuilt.</p>
        <p className="pd-go"><Link href="/">Back to the start</Link><Link href="/blog">Open the blog</Link></p>
      </main>
      <SiteFooter exit="[Inferior 1 (process 4021) exited with code 0177]" />
    </>
  );
}
