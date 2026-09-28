import type { Metadata } from "next";
import { projects } from "@/data/portfolio";
import Link from "next/link";
import Image from "next/image";
import { notFound } from "next/navigation";
import fs from "node:fs";
import path from "node:path";
import { ProjectGallery } from "@/components/ui/project-gallery";
import { CinematicVideo } from "@/components/ui/cinematic-video";
import { SiteHeader } from "@/components/ui/site-header";
import { EnhancedFooter } from "@/components/ui/enhanced-footer";
import { MarkdownBody } from "@/components/ui/markdown-body";

const slug = (name: string) => name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, "");
export const dynamicParams = false;
export function generateStaticParams() { return projects.map(p => ({ id: slug(p.name) })); }
export async function generateMetadata({ params }: { params: Promise<{ id: string }> }): Promise<Metadata> {
  const id = (await params).id;
  const project = projects.find(p => slug(p.name) === id);
  return { title: project ? project.name + " — Khalil Ammar" : "Project not found", description: project?.summary };
}
export default async function ProjectPage({ params }: { params: Promise<{ id: string }> }) {
  const id = (await params).id;
  const project = projects.find(p => slug(p.name) === id);
  if (!project) notFound();
  const nextProject = projects[(projects.indexOf(project) + 1) % projects.length];
  const markdown = project.detailsFile ? fs.readFileSync(path.join(process.cwd(), "public", project.detailsFile), "utf8") : null;
  // Only show files that are actually available; missing media should not produce broken images.
  const screenshots = (project.screenshots ?? []).map((src, i) => ({ src, caption: project.screenshotCaptions?.[i] }))
    .filter(item => !item.src.startsWith("/") || fs.existsSync(path.join(process.cwd(), "public", item.src)));

  return (
    <div className="editorial-page project-page">
      <SiteHeader />
      <main id="main-content" className="journal-container">
        <nav className="article-breadcrumb" aria-label="Breadcrumb"><Link href="/#projects">← All projects</Link><span aria-hidden>/</span><span>Case study</span></nav>
        <header className="project-header">
          <p className="eyebrow">{project.context ?? "Selected project"}</p>
          <h1>{project.name}</h1>
          <p>{project.summary}</p>
          <dl className="project-facts">{[["Role", project.role], ["Timeline", project.duration]].filter(([, value]) => value).map(([label, value]) => <div key={label}><dt>{label}</dt><dd>{value}</dd></div>)}</dl>
        </header>
        <div className="project-body-grid">
          <div className="project-main-column">
            {project.mediaUrl && <section className="project-demo" aria-label="Project demonstration">
              {project.mediaUrl.includes("youtube.com/embed") ? <CinematicVideo url={project.mediaUrl} title={project.name + " demonstration"} />
                : /\.(mp4|webm|ogg)$/i.test(project.mediaUrl) ? <video src={project.mediaUrl} controls playsInline preload="metadata" />
                : <Image src={project.mediaUrl} alt={project.name + " preview"} width={1000} height={600} />}
            </section>}
            {[{ title: "Architecture", text: project.architecture }, { title: "Implementation", text: project.challenges }, { title: "Outcome", text: project.impact }].filter(s => s.text).map(section => <section key={section.title} className="project-narrative"><h2>{section.title}</h2><p>{section.text}</p></section>)}
            {project.features?.length ? <section className="project-narrative"><h2>What it does</h2><ul>{project.features.map(feature => <li key={feature}>{feature}</li>)}</ul></section> : null}
          </div>
          <aside className="project-stack"><div><h2>Built with</h2><ul>{project.stack.map(tech => <li key={tech}>{tech}</li>)}</ul>{project.githubUrl ? <a className="ui-button" href={project.githubUrl} target="_blank" rel="noopener noreferrer">View source ↗</a> : <a className="journal-read" href="https://github.com/khalilammarr" target="_blank" rel="noopener noreferrer">My GitHub profile ↗</a>}</div></aside>
        </div>
        {screenshots.length > 0 && <section className="project-screenshots"><h2>In practice</h2><p>Screenshots from the project. Select an image to take a closer look.</p><ProjectGallery name={project.name} screenshots={screenshots.map(s => s.src)} screenshotCaptions={screenshots.map(s => s.caption ?? "")} /></section>}
        {markdown && <section className="project-documentation"><p className="eyebrow">Technical notes</p><MarkdownBody>{markdown}</MarkdownBody></section>}
        <nav className="next-project" aria-label="Next project"><span className="eyebrow">Next project</span><Link href={"/project/" + slug(nextProject.name)}>{nextProject.name}<span aria-hidden>↗</span></Link></nav>
      </main>
      <EnhancedFooter />
    </div>
  );
}
