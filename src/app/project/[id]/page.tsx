import type { Metadata } from "next";
import Link from "next/link";
import { notFound } from "next/navigation";
import fs from "node:fs";
import path from "node:path";
import { projects } from "@/data/portfolio";
import { SEC, projAddr } from "@/lib/trace";
import { SiteBar } from "@/components/site/site-bar";
import { SiteFooter } from "@/components/site/site-footer";
import { MapsLine } from "@/components/site/maps-line";
import { Glyph } from "@/components/site/glyph";
import { HoneypotDiagram } from "@/components/site/honeypot-diagram";
import { VideoEmbed } from "@/components/site/video-embed";
import { MarkdownBody } from "@/components/ui/markdown-body";
import { ProjectGallery } from "@/components/ui/project-gallery";

const ordered = () => [...projects].sort((a, b) => a.order - b.order);

export const dynamicParams = false;
export function generateStaticParams() {
  return projects.map((p) => ({ id: p.slug }));
}

export async function generateMetadata({ params }: { params: Promise<{ id: string }> }): Promise<Metadata> {
  const { id } = await params;
  const project = projects.find((p) => p.slug === id);
  return { title: project ? project.title + " — Khalil Ammar" : "Project not found", description: project?.summary };
}

const publicFile = (src: string) => path.join(process.cwd(), "public", src);

export default async function ProjectPage({ params }: { params: Promise<{ id: string }> }) {
  const list = ordered();
  const { id } = await params;
  const i = list.findIndex((p) => p.slug === id);
  if (i < 0) notFound();
  const project = list[i];
  const next = list[(i + 1) % list.length];
  const notes = project.detailsFile && fs.existsSync(publicFile(project.detailsFile)) ? fs.readFileSync(publicFile(project.detailsFile), "utf8") : null;
  // only show files that exist; missing media should not produce broken images
  const shots = (project.screenshots ?? []).map((src, n) => ({ src, caption: project.screenshotCaptions?.[n] ?? "" }))
    .filter((s) => !s.src.startsWith("/") || fs.existsSync(publicFile(s.src)));
  const media = project.mediaUrl;
  const story = [
    { title: "Architecture", text: project.architecture },
    { title: "Implementation", text: project.challenges },
    { title: "Outcome", text: project.impact },
  ].filter((s): s is { title: string; text: string } => !!s.text);
  const facts = [["Role", project.role], ["Timeline", project.duration], ["Context", project.context]].filter((f): f is [string, string] => !!f[1]);

  return (
    <>
      <SiteBar active="work" symbol={"project/" + project.slug} />
      <main className="wrap sub" id="main-content">
        <header className="pj-head rv">
          <MapsLine region="work" name={"project/" + project.slug} at={SEC.work.base + i * 0x400} />
          <nav className="crumbs" aria-label="Breadcrumb" data-r><Link href="/#work">Work</Link><span aria-hidden="true">/</span><span className="mono">{projAddr(i)}</span></nav>
          <div className="pj-title" data-r><Glyph name={project.glyph} /><h1>{project.title}</h1></div>
          <p className="ar-deck" data-r>{project.blurb}</p>
          <p className="meta" data-r>{project.meta}</p>
        </header>

        {(media || project.diagram) && (
          <section className="pj-media rv" aria-label="Demonstration"><div data-r="clip">
            {media?.includes("youtube.com/embed") ? <VideoEmbed url={media} title={project.title + " demonstration"} />
              : media && /\.(mp4|webm|ogg)$/i.test(media) ? <video src={media} controls playsInline preload="metadata" />
              : project.diagram === "honeypot" ? <HoneypotDiagram />
              // eslint-disable-next-line @next/next/no-img-element -- static export
              : media ? <img src={media} alt={project.title + " preview"} /> : null}
          </div></section>
        )}

        <ol className="flow pj-flow rv" aria-label="Pipeline">{project.flow.map((s) => <li data-r key={s}><span>{s}</span></li>)}</ol>

        <div className="pj-grid">
          <div className="pj-main">
            {story.map((s, n) => (
              <section className="pj-sec rv" key={s.title}>
                <p className="pj-n mono" data-r>{String(n + 1).padStart(2, "0")}</p>
                <div data-r><h2>{s.title}</h2><p>{s.text}</p></div>
              </section>
            ))}
            {project.features?.length ? (
              <section className="pj-sec rv">
                <p className="pj-n mono" data-r>{String(story.length + 1).padStart(2, "0")}</p>
                <div data-r><h2>What it does</h2><ul className="feats">{project.features.map((f) => <li key={f}>{f}</li>)}</ul></div>
              </section>
            ) : null}
          </div>
          <aside className="pj-side">
            <div className="pj-card">
              {facts.length > 0 && <dl className="pj-facts">{facts.map(([k, v]) => <div key={k}><dt>{k}</dt><dd>{v}</dd></div>)}</dl>}
              <p className="kick">Built with</p>
              <ul className="chips">{project.stack.map((t) => <li key={t}>{t}</li>)}</ul>
              <a className="btn" href={project.githubUrl ?? "https://github.com/khalilammarr"} target="_blank" rel="noopener noreferrer">
                {project.githubUrl ? "Source on GitHub" : "My GitHub profile"}<span aria-hidden="true">↗</span>
              </a>
            </div>
          </aside>
        </div>

        {(project.figures?.length || shots.length > 0) ? (
          <section className="pj-shots rv" aria-labelledby="shots-h">
            <h2 id="shots-h" className="pj-h2" data-r>In practice</h2>
            <div data-r><ProjectGallery name={project.title}
              screenshots={[...(project.figures ?? []).map((f) => f.src), ...shots.map((s) => s.src)]}
              screenshotCaptions={[...(project.figures ?? []).map((f) => f.alt), ...shots.map((s) => s.caption)]} /></div>
          </section>
        ) : null}

        {notes && (
          <section className="pj-notes" aria-labelledby="notes-h">
            <h2 id="notes-h" className="pj-h2">Technical notes</h2>
            <div className="ar-body"><MarkdownBody>{notes}</MarkdownBody></div>
          </section>
        )}

        <nav className="pj-next" aria-label="Next project">
          <Link href={"/project/" + next.slug}>
            <span className="mono dim">next → {projAddr(list.indexOf(next))}</span>
            <span className="pj-next-t"><Glyph name={next.glyph} />{next.title}</span>
            <span className="s">{next.short}</span>
          </Link>
        </nav>
      </main>
      <SiteFooter />
    </>
  );
}
