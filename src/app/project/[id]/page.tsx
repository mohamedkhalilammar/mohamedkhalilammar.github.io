import { projects } from "@/data/portfolio";
import Link from "next/link";
import { notFound } from "next/navigation";
import fs from "fs";
import path from "path";
import ReactMarkdown from "react-markdown";
import rehypeRaw from "rehype-raw";

import { ProjectGallery } from "@/components/ui/project-gallery";
import { CinematicVideo } from "@/components/ui/cinematic-video";

export const dynamicParams = false;

export async function generateStaticParams() {
  const slugs = projects.map((p) => ({
    id: p.name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, ""),
  }));
  return slugs;
}

export default async function ProjectPage({ params }: { params: Promise<{ id: string }> }) {
  const resolvedParams = await params;
  const id = resolvedParams.id;

  const project = projects.find(
    (p) => p.name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, "") === id
  );

  if (!project) {
    notFound();
  }

  const stackPreview = project.stack.slice(0, 4).join(" · ");
  const projectPulse = [
    { label: "Role", value: project.role || "Security Engineer" },
    { label: "Timeline", value: project.duration || "Continuous" },
    { label: "Stack Depth", value: `${project.stack.length} technologies` },
    { label: "Focus", value: project.context || "Applied Security Engineering" },
  ];

  let markdownContent = null;
  if (project.detailsFile) {
    try {
      const filePath = path.join(process.cwd(), "public", project.detailsFile);
      markdownContent = fs.readFileSync(filePath, "utf-8");
    } catch (e) {
      console.error(`Failed to read markdown file for ${project.name}`, e);
    }
  }

  return (
    <div className="page-shell bg-[#0d0d0d] min-h-screen">
      {/* Simplified Space */}
      <div className="fixed left-0 top-0 bottom-0 w-[1px] bg-white/[0.03] z-50" />

      <header className="fixed top-0 left-0 right-0 z-50 backdrop-blur-md border-b border-white/[0.03] px-6 py-4 flex justify-between items-center bg-black/20">
        <Link href="/#projects" className="group flex items-center gap-3 px-4 py-2 rounded-full border border-white/5 bg-white/[0.02] hover:bg-white/[0.06] hover:border-amber-500/30 transition-all duration-300">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round" className="text-amber-500 group-hover:-translate-x-1 transition-transform">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
          <span className="font-mono text-[10px] uppercase tracking-[0.2em] text-white/60 group-hover:text-white transition-colors">
            Return to Index
          </span>
        </Link>
        <div className="flex items-center gap-4">
          <div className="w-2 h-2 rounded-full bg-amber-500" />
          <span className="font-mono text-[10px] uppercase tracking-[0.3em] text-amber-500">Project Live</span>
        </div>
      </header>

      <main className="max-w-[1300px] mx-auto px-6 md:px-12 lg:px-20" style={{ paddingTop: "8.5rem", paddingBottom: "10rem" }}>

        {/* TOP LEVEL INTEL: Title & Summary */}
        <div className="relative mb-24 css-stagger-item">
          {/* Decorative Corner Brackets */}
          <div className="absolute -top-12 -left-12 w-24 h-24 border-t border-l border-white/10 rounded-tl-[3rem]" />

          <div className="flex flex-col gap-6">
            <div className="flex items-center gap-4">
              <div className="px-3 py-1 rounded bg-white/5 border border-white/10 text-[10px] font-mono font-bold text-amber-500 uppercase tracking-[0.3em]">
                Project Overview
              </div>
              <div className="h-px w-24 bg-white/10" />
              <p className="font-mono text-xs uppercase tracking-[0.4em] text-white/20 font-bold">{project.context || "Engineering Student"}</p>
            </div>

            <h1 className="font-sans text-5xl md:text-7xl lg:text-8xl font-black text-white uppercase tracking-tighter leading-[0.85] mb-8 drop-shadow-2xl">
              {project.name.split(" ").map((word, i) => (
                <span key={i} className={i % 2 !== 0 ? "text-amber-500" : ""}>
                  {word}{" "}
                </span>
              ))}
            </h1>

            <p className="text-lg md:text-2xl leading-relaxed text-zinc-400 max-w-4xl font-medium">
              {project.summary}
            </p>
          </div>
        </div>



        {/* CORE CONTENT LAYOUT */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-16">

          <div className="lg:col-span-9 space-y-32">

            {/* Visual Evidence Area */}
            {(project.mediaUrl || (project.screenshots && project.screenshots.length > 0)) && (
              <div className="css-stagger-item">
                <div className="flex items-center gap-6 mb-8">
                  <h2 className="font-sans text-2xl font-black uppercase tracking-tighter text-white">Visual Architecture</h2>
                  <div className="h-px flex-1 bg-white/[0.05]" />
                </div>

                {/* Enhanced Media Frame */}
                {project.mediaUrl && (
                  <div className="relative rounded-[2rem] overflow-hidden border border-white/10 bg-black shadow-2xl mb-16">
                    <div className="w-full">
                      {project.mediaUrl.includes("youtube.com/embed") ? (
                        <CinematicVideo
                          url={project.mediaUrl}
                          title={`${project.name} Video Demo`}
                        />
                      ) : project.mediaUrl.match(/\.(mp4|webm|ogg)$/i) ? (
                        <video src={project.mediaUrl} controls autoPlay muted loop playsInline className="w-full h-auto aspect-video object-cover" />
                      ) : (
                        <img src={project.mediaUrl} alt="Preview" className="w-full h-auto aspect-video object-cover" />
                      )}
                    </div>
                  </div>
                )}

                {/* Screenshot Gallery */}
                {project.screenshots && project.screenshots.length > 0 && (
                  <ProjectGallery name={project.name} screenshots={project.screenshots} screenshotCaptions={project.screenshotCaptions} />
                )}
              </div>
            )}

            {/* Deep Analysis (Descriptions) */}
            <div className="space-y-40 css-stagger-item">
              <div className="grid grid-cols-1 md:grid-cols-[280px_1fr] gap-x-16 gap-y-20 lg:gap-y-32 items-start py-12 border-t border-white/5">
                {project.architecture && (
                  <>
                    <h2 className="font-sans text-2xl font-black uppercase tracking-tighter text-amber-500/80">System Architecture</h2>
                    <p className="text-xl leading-relaxed text-zinc-300 font-medium max-w-4xl">{project.architecture}</p>
                  </>
                )}

                {project.challenges && (
                  <>
                    <h2 className="font-sans text-2xl font-black uppercase tracking-tighter text-amber-500/80">Implementation Strategy</h2>
                    <p className="text-xl leading-relaxed text-zinc-300 font-medium max-w-4xl">{project.challenges}</p>
                  </>
                )}
                {project.impact && (
                  <>
                    <h2 className="font-sans text-2xl font-black uppercase tracking-tighter text-amber-500/80">Technical Outcome</h2>
                    <p className="text-xl leading-relaxed text-zinc-300 font-medium max-w-4xl">{project.impact}</p>
                  </>
                )}
              </div>

              {project.features && (
                <div className="p-12 rounded-[2.5rem] bg-white/[0.01] border border-white/[0.04] relative overflow-hidden group">
                  <div className="absolute top-0 right-0 w-32 h-32 bg-amber-500/5 blur-[80px] group-hover:bg-amber-500/10 transition-all" />
                  <h2 className="font-sans text-4xl font-black uppercase tracking-tighter text-white mb-10">Key Features</h2>
                  <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-8">
                    {project.features.map((feature, i) => (
                      <div key={feature} className="flex gap-4 group/item">
                        <span className="font-mono text-sm text-amber-500/40 group-hover/item:text-amber-500 font-bold">{String(i + 1).padStart(2, '0')}</span>
                        <div className="h-px flex-1 mt-3 bg-white/[0.05]" />
                        <span className="text-base text-zinc-400 font-medium group-hover/item:text-zinc-100 transition-colors">{feature}</span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          </div>

          {/* SIDEBAR: System Specs */}
          <aside className="lg:col-span-3 lg:sticky lg:top-32 self-start space-y-10 css-stagger-item">
            <div className="p-10 rounded-3xl bg-[#0a0a0a] border border-white/5 relative overflow-hidden">
              {/* Background Grid Accent */}
              <div className="absolute inset-0 opacity-[0.03] pointer-events-none" style={{ backgroundImage: "radial-gradient(var(--accent) 1px, transparent 0)", backgroundSize: "20px 20px" }} />

              <h3 className="font-sans text-xl font-bold text-white uppercase mb-10 pb-6 border-b border-white/5 flex items-center justify-between">
                Project Specs
              </h3>

              <div className="space-y-8">

                <div className="space-y-6 pt-6 border-t border-white/5">
                  <p className="font-mono text-[9px] uppercase tracking-[0.3em] text-white/30">Technology Array</p>
                  <ul className="grid grid-cols-1 gap-3">
                    {project.stack.map(tech => (
                      <li key={tech} className="flex items-center gap-3 group/li">
                        <div className="w-1.5 h-1.5 rounded-full bg-amber-500/20 group-hover/li:bg-amber-500 group-hover/li:scale-150 transition-all" />
                        <span className="font-mono text-xs text-white/60 group-hover/li:text-white transition-colors tracking-wide">{tech}</span>
                      </li>
                    ))}
                  </ul>
                </div>
              </div>

              {project.githubUrl && (
                <div className="mt-12">
                  <a
                    href={project.githubUrl}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="btn-primary w-full flex items-center justify-center gap-3 py-4"
                  >
                    <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 24 24">
                      <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" />
                    </svg>
                    Visit GitHub
                  </a>
                </div>
              )}
            </div>

          </aside>
        </div>

        {/* FULL WIDTH MARKDOWN SECTION */}
        {markdownContent && (
          <div className="project-markdown-block mt-32 p-8 md:p-16 border border-white/5 bg-gradient-to-b from-white/[0.02] to-transparent rounded-[3rem] w-full relative overflow-hidden">
            {/* Background accents */}
            <div className="absolute top-0 right-0 w-96 h-96 bg-amber-500/5 blur-[120px] pointer-events-none" />
            <div className="absolute bottom-0 left-0 w-64 h-64 bg-amber-500/5 blur-[100px] pointer-events-none" />

            <div className="prose prose-invert prose-amber prose-lg leading-relaxed max-w-none relative z-10">
              <ReactMarkdown rehypePlugins={[rehypeRaw]}>{markdownContent}</ReactMarkdown>
            </div>
          </div>
        )}
      </main>
    </div>
  );
}
